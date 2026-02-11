/*
 * PrintSense - Monitor Ambiental para Impressão 3D
 * ESP32-S3 WROOM1 com Display LCD 20x4
 * 
 * Hardware:
 * - SD Card SDMMC: CLK=39, CMD=38, D0=40
 * - DHT22: GPIO 17
 * - LDR: GPIO 4 (analógico)
 * - MAX4466 (som): GPIO 8 (analógico)
 * - LCD I2C 20x4: SDA=10, SCL=13, Endereço 0x27
 * - Encoder Rotativo: CLK=7, DT=6, SW=5
 */

#include <WiFi.h>
#include <WebServer.h>
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "FS.h"
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <time.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

// ==================== CONFIGURAÇÕES ====================
const char* ssid = "VIVOFIBRA-BAF2";
const char* password = "3dd920baf2";

// Pinos Sensores
#define DHTPIN 17
#define DHT_TYPE DHT22
#define LDR_PIN 4
#define MAX4466_PIN 8

// Pinos Encoder Rotativo
#define ENCODER_CLK 7
#define ENCODER_DT 6
#define ENCODER_SW 5

// Pinos SD Card - SDMMC (1-bit mode)
#define SD_CLK_PIN GPIO_NUM_39
#define SD_CMD_PIN GPIO_NUM_38
#define SD_D0_PIN  GPIO_NUM_40

// Pinos I2C LCD (customizados conforme hardware)
#define I2C_SDA 10
#define I2C_SCL 13

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Configuração de tempo
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

// ==================== OBJETOS ====================
DHT dht(DHTPIN, DHT_TYPE);
WebServer server(80);

bool sdCardAvailable = false;
sdmmc_card_t* sdCard = nullptr;

// ==================== ESTRUTURAS ====================
struct SensorData {
  float temperature;
  float humidity;
  uint16_t light;
  int sound;
  unsigned long timestamp;
};

SensorData currentData;

// Materiais disponíveis
const char* materialNames[] = {"PLA", "PETG", "ABS", "RESINA"};
int currentMaterialIndex = 0;
String currentMaterial = "PLA";

unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 60000;

// Controle do Encoder
volatile int encoderPos = 0;
int lastEncoderPos = 0;
bool encoderButtonPressed = false;
unsigned long lastEncoderTime = 0;
bool inSelectionMode = false;  // Flag para modo de seleção

// Controle do LCD
unsigned long lastLCDUpdate = 0;
const unsigned long LCD_UPDATE_INTERVAL = 1000;

struct MaterialThresholds {
  String name;
  float tempMin, tempMax;
  float humMin, humMax;
  int lightMax;
  int soundMax;
};

MaterialThresholds materials[] = {
  {"PLA",   18, 28, 40, 60, 3000, 2000},
  {"PETG",  20, 30, 30, 50, 3000, 2000},
  {"ABS",   22, 32, 20, 40, 3000, 2000},
  {"RESINA", 20, 25, 40, 60, 1000, 1500}
};

// ==================== DECLARAÇÃO DE FUNÇÕES ====================
bool initSDCard();
void printSDCardInfo();
void createDirectories();
void initLCD();
void updateLCD();
void readSensors();
void logToSD();
String getLogFilename();
String getFormattedTime();
String getStatus();
String getStatusDetails();
MaterialThresholds getMaterialThresholds(String material);
void setupWebServer();
String getDefaultHTML();
String getDefaultCSS();
String getDefaultJS();
void setupEncoder();
void IRAM_ATTR encoderISR();
void IRAM_ATTR encoderButtonISR();
void handleEncoder();
void displayMaterialSelection();
char getStatusIcon();

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=================================");
  Serial.println("PrintSense - Iniciando...");
  Serial.println("=================================\n");
  
  // Configurar ADC
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  // LCD
  Serial.println("[1/6] Inicializando LCD...");
  initLCD();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  PrintSense v1.0");
  lcd.setCursor(0, 1);
  lcd.print("  Inicializando...");
  delay(2000);
  
  // Encoder
  Serial.println("\n[2/6] Configurando Encoder...");
  setupEncoder();
  lcd.setCursor(0, 2);
  lcd.print("Encoder: OK");
  delay(500);
  
  // DHT22
  Serial.println("\n[3/6] Inicializando DHT22...");
  pinMode(DHTPIN, INPUT_PULLUP);
  delay(100);
  dht.begin();
  delay(2000);
  
  float testTemp = dht.readTemperature();
  if (isnan(testTemp)) {
    Serial.println("⚠️  DHT22: Sensor não responde!");
    lcd.setCursor(0, 3);
    lcd.print("DHT22: ERRO");
  } else {
    Serial.print("✅ DHT22 OK! Temp: ");
    Serial.print(testTemp);
    Serial.println("°C");
    lcd.setCursor(0, 3);
    lcd.print("DHT22: OK ");
    lcd.print(testTemp, 1);
    lcd.print((char)223);  // Símbolo de grau
    lcd.print("C");
  }
  delay(1000);
  
  // SD Card
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando SD Card...");
  
  Serial.println("\n[4/6] Inicializando SD Card (SDMMC)...");
  sdCardAvailable = initSDCard();
  
  if (sdCardAvailable) {
    Serial.println("✅ SD Card OK!");
    printSDCardInfo();
    createDirectories();
    lcd.setCursor(0, 1);
    lcd.print("SD: OK");
  } else {
    Serial.println("❌ SD Card falhou!");
    lcd.setCursor(0, 1);
    lcd.print("SD: FALHOU");
  }
  delay(1000);
  
  // WiFi
  lcd.setCursor(0, 2);
  lcd.print("Conectando WiFi...");
  
  Serial.println("\n[5/6] Configurando WiFi...");
  WiFi.begin(ssid, password);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    lcd.setCursor(0, 2);
    lcd.print("WiFi: OK            ");
    lcd.setCursor(0, 3);
    lcd.print(WiFi.localIP());
  } else {
    Serial.println("\n⚠️  WiFi falhou, criando AP...");
    WiFi.softAP("PrintSense", "printsense123");
    lcd.setCursor(0, 2);
    lcd.print("WiFi: AP Mode       ");
    lcd.setCursor(0, 3);
    lcd.print(WiFi.softAPIP());
  }
  
  delay(2000);
  
  // WebServer
  Serial.println("\n[6/6] Iniciando WebServer...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  setupWebServer();
  server.begin();
  
  Serial.println("\n✅ Sistema pronto!");
  Serial.println("=================================\n");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Pronto!");
  delay(1500);
  
  // Primeira atualização
  updateLCD();
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  handleEncoder();
  readSensors();
  
  // Atualizar LCD apenas se NÃO estiver em modo de seleção
  if (!inSelectionMode && millis() - lastLCDUpdate > LCD_UPDATE_INTERVAL) {
    updateLCD();
    lastLCDUpdate = millis();
  }
  
  // Log periódico
  if (millis() - lastLogTime > LOG_INTERVAL) {
    logToSD();
    lastLogTime = millis();
  }
  
  delay(100);
}

// ==================== LCD ====================
void initLCD() {
  Wire.begin(I2C_SDA, I2C_SCL);  // SDA=10, SCL=13
  lcd.init();
  lcd.backlight();
  Serial.println("✅ LCD inicializado (I2C: SDA=10, SCL=13)");
}

void updateLCD() {
  lcd.clear();
  
  // Linha 0: Material e Status
  lcd.setCursor(0, 0);
  lcd.print(currentMaterial);
  lcd.print(" ");
  lcd.print(getStatusIcon());
  lcd.print(" ");
  lcd.print(getStatus());
  
  // Linha 1: Temperatura
  lcd.setCursor(0, 1);
  lcd.print("T:");
  if (!isnan(currentData.temperature)) {
    lcd.print(currentData.temperature, 1);
    lcd.print((char)223);  // Símbolo de grau
    lcd.print("C");
  } else {
    lcd.print("--");
  }
  
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  lcd.setCursor(11, 1);
  lcd.print(mat.tempMin, 0);
  lcd.print("-");
  lcd.print(mat.tempMax, 0);
  lcd.print((char)223);
  lcd.print("C");
  
  // Linha 2: Umidade
  lcd.setCursor(0, 2);
  lcd.print("H:");
  if (!isnan(currentData.humidity)) {
    lcd.print(currentData.humidity, 1);
    lcd.print("%");
  } else {
    lcd.print("--");
  }
  
  lcd.setCursor(11, 2);
  lcd.print(mat.humMin, 0);
  lcd.print("-");
  lcd.print(mat.humMax, 0);
  lcd.print("%");
  
  // Linha 3: Luz e Som
  lcd.setCursor(0, 3);
  lcd.print("Luz:");
  lcd.print(currentData.light);
  
  lcd.setCursor(10, 3);
  lcd.print("Som:");
  lcd.print(currentData.sound);
}

void displayMaterialSelection() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  SELECIONAR MATERIAL");
  lcd.setCursor(0, 1);
  lcd.print("--------------------");
  
  // Mostrar material atual centralizado
  lcd.setCursor(0, 2);
  String mat = String(materialNames[currentMaterialIndex]);
  int spaces = (20 - mat.length() - 6) / 2;
  for (int i = 0; i < spaces; i++) lcd.print(" ");
  lcd.print(">> ");
  lcd.print(mat);
  lcd.print(" <<");
  
  lcd.setCursor(0, 3);
  lcd.print("Gire p/ mudar | OK");
}

char getStatusIcon() {
  String status = getStatus();
  if (status == "IDEAL") return '*';
  if (status == "BOM") return '!';
  return 'X';
}

// ==================== ENCODER ====================
void setupEncoder() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_SW), encoderButtonISR, FALLING);
  
  Serial.println("✅ Encoder configurado");
}

void IRAM_ATTR encoderISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderTime < 5) return;
  
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
  
  if (clkState != dtState) {
    encoderPos++;
  } else {
    encoderPos--;
  }
  
  lastEncoderTime = currentTime;
}

void IRAM_ATTR encoderButtonISR() {
  encoderButtonPressed = true;
}

void handleEncoder() {
  static unsigned long selectionStartTime = 0;
  
  // Botão pressionado
  if (encoderButtonPressed) {
    encoderButtonPressed = false;
    
    if (inSelectionMode) {
      // Confirmar seleção
      currentMaterial = String(materialNames[currentMaterialIndex]);
      inSelectionMode = false;
      
      Serial.print("Material alterado para: ");
      Serial.println(currentMaterial);
      
      // Atualizar LCD imediatamente com novo material
      updateLCD();
      lastLCDUpdate = millis();
    } else {
      // Entrar modo seleção
      inSelectionMode = true;
      selectionStartTime = millis();
      displayMaterialSelection();
    }
    
    delay(200);
  }
  
  // Modo seleção ativo
  if (inSelectionMode) {
    // Timeout de 10 segundos
    if (millis() - selectionStartTime > 10000) {
      inSelectionMode = false;
      updateLCD();
      lastLCDUpdate = millis();
      return;
    }
    
    // Detectar rotação
    if (encoderPos != lastEncoderPos) {
      if (encoderPos > lastEncoderPos) {
        currentMaterialIndex++;
        if (currentMaterialIndex > 3) currentMaterialIndex = 0;
      } else {
        currentMaterialIndex--;
        if (currentMaterialIndex < 0) currentMaterialIndex = 3;
      }
      
      lastEncoderPos = encoderPos;
      displayMaterialSelection();
    }
  }
}

// ==================== SD CARD ====================
bool initSDCard() {
  Serial.printf("   Pinos: CLK=%d, CMD=%d, D0=%d\n", 
                SD_CLK_PIN, SD_CMD_PIN, SD_D0_PIN);

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
  
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.clk = SD_CLK_PIN;
  slot_config.cmd = SD_CMD_PIN;
  slot_config.d0 = SD_D0_PIN;
  slot_config.width = 1;
  
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
  };
  
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &sdCard);
  
  if (ret != ESP_OK) {
    Serial.printf("   Falha: 0x%x - %s\n", ret, esp_err_to_name(ret));
    return false;
  }
  
  return true;
}

void printSDCardInfo() {
  if (!sdCardAvailable || !sdCard) return;
  
  Serial.print("   Nome: ");
  Serial.println(sdCard->cid.name);
  
  uint64_t cardSize = (uint64_t)sdCard->csd.capacity * sdCard->csd.sector_size;
  Serial.printf("   Capacidade: %.2f GB\n", (float)cardSize / (1024 * 1024 * 1024));
}

void createDirectories() {
  mkdir("/sdcard/logs", 0777);
  mkdir("/sdcard/jobs", 0777);
  mkdir("/sdcard/web", 0777);
}

// ==================== SENSORES ====================
void readSensors() {
  // DHT22
  currentData.temperature = dht.readTemperature();
  currentData.humidity = dht.readHumidity();
  
  if (isnan(currentData.temperature)) currentData.temperature = 0.0;
  if (isnan(currentData.humidity)) currentData.humidity = 0.0;
  
  // LDR (0-4095: mais luz = maior valor ADC)
  currentData.light = analogRead(LDR_PIN);
  
  // MAX4466 (microfone)
  currentData.sound = analogRead(MAX4466_PIN);
  
  currentData.timestamp = millis();
}

// ==================== LOGS ====================
void logToSD() {
  if (!sdCardAvailable) {
    Serial.println("[LOG] SD indisponível");
    return;
  }
  
  String filename = getLogFilename();
  FILE* file = fopen(filename.c_str(), "a");
  
  if (file) {
    String logLine = String(getFormattedTime()) + "," +
                     String(currentData.temperature, 1) + "," +
                     String(currentData.humidity, 1) + "," +
                     String(currentData.light) + "," +
                     String(currentData.sound) + "," +
                     getStatus();
    
    fprintf(file, "%s\n", logLine.c_str());
    fclose(file);
    Serial.println("[LOG] ✓ " + logLine);
  }
}

String getLogFilename() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "/sdcard/logs/log.csv";
  }
  
  char filename[64];
  snprintf(filename, sizeof(filename), "/sdcard/logs/%04d%02d%02d.csv", 
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  
  FILE* testFile = fopen(filename, "r");
  if (!testFile) {
    FILE* newFile = fopen(filename, "w");
    if (newFile) {
      fprintf(newFile, "timestamp,temperature,humidity,light,sound,status\n");
      fclose(newFile);
    }
  } else {
    fclose(testFile);
  }
  
  return String(filename);
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return String(millis());
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// ==================== STATUS ====================
String getStatus() {
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  
  bool tempOK = (currentData.temperature >= mat.tempMin && currentData.temperature <= mat.tempMax);
  bool humOK = (currentData.humidity >= mat.humMin && currentData.humidity <= mat.humMax);
  
  if (tempOK && humOK) {
    return "IDEAL";
  } else if (tempOK || humOK) {
    return "BOM";
  } else {
    return "RUIM";
  }
}

String getStatusDetails() {
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  String details = "";
  
  if (currentData.temperature < mat.tempMin) {
    details += "Temperatura baixa. ";
  } else if (currentData.temperature > mat.tempMax) {
    details += "Temperatura alta. ";
  }
  
  if (currentData.humidity < mat.humMin) {
    details += "Umidade baixa. ";
  } else if (currentData.humidity > mat.humMax) {
    details += "Umidade alta. ";
  }
  
  if (details == "") {
    details = "Condições ideais!";
  }
  
  return details;
}

MaterialThresholds getMaterialThresholds(String material) {
  for (int i = 0; i < 4; i++) {
    if (materials[i].name == material) {
      return materials[i];
    }
  }
  return materials[0];
}

// ==================== WEB SERVER ====================
void setupWebServer() {
  // Página principal - tenta do SD, senão usa padrão
  server.on("/", HTTP_GET, []() {
    if (sdCardAvailable) {
      FILE* file = fopen("/sdcard/web/index.html", "r");
      if (file) {
        String content = "";
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), file)) {
          content += buffer;
        }
        fclose(file);
        server.send(200, "text/html", content);
        Serial.println("Servindo index.html do SD Card");
        return;
      }
    }
    server.send(200, "text/html", getDefaultHTML());
    Serial.println("Servindo index.html padrão (SD não disponível)");
  });
  
  // API - Dados dos sensores
  server.on("/api/data", HTTP_GET, []() {
    StaticJsonDocument<512> doc;
    
    doc["temperature"] = currentData.temperature;
    doc["humidity"] = currentData.humidity;
    doc["light"] = currentData.light;
    doc["sound"] = currentData.sound;
    doc["timestamp"] = getFormattedTime();
    doc["material"] = currentMaterial;
    doc["status"] = getStatus();
    doc["statusDetails"] = getStatusDetails();
    
    MaterialThresholds mat = getMaterialThresholds(currentMaterial);
    doc["thresholds"]["tempMin"] = mat.tempMin;
    doc["thresholds"]["tempMax"] = mat.tempMax;
    doc["thresholds"]["humMin"] = mat.humMin;
    doc["thresholds"]["humMax"] = mat.humMax;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  // API - Mudar material
  server.on("/api/material", HTTP_POST, []() {
    if (server.hasArg("material")) {
      currentMaterial = server.arg("material");
      
      // Atualizar índice do encoder
      for (int i = 0; i < 4; i++) {
        if (currentMaterial == materialNames[i]) {
          currentMaterialIndex = i;
          break;
        }
      }
      
      Serial.print("Material alterado via API: ");
      Serial.println(currentMaterial);
      
      server.send(200, "application/json", "{\"success\":true}");
    } else {
      server.send(400, "application/json", "{\"success\":false}");
    }
  });
  
  // API - Listar logs disponíveis
  server.on("/api/logs", HTTP_GET, []() {
    StaticJsonDocument<2048> doc;
    JsonArray logs = doc.createNestedArray("logs");
    
    if (sdCardAvailable) {
      DIR* dir = opendir("/sdcard/logs");
      if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
          if (entry->d_type == DT_REG) {  // Arquivo regular
            String filename = String(entry->d_name);
            if (filename.endsWith(".csv")) {
              logs.add(filename);
            }
          }
        }
        closedir(dir);
      }
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  // API - Download de log específico
  server.on("/api/log", HTTP_GET, []() {
    if (server.hasArg("file")) {
      String filename = "/sdcard/logs/" + server.arg("file");
      
      if (sdCardAvailable) {
        FILE* file = fopen(filename.c_str(), "r");
        if (file) {
          String content = "";
          char buffer[256];
          while (fgets(buffer, sizeof(buffer), file)) {
            content += buffer;
          }
          fclose(file);
          
          Serial.print("Download log: ");
          Serial.println(server.arg("file"));
          
          server.send(200, "text/csv", content);
        } else {
          server.send(404, "text/plain", "Arquivo não encontrado");
        }
      } else {
        server.send(503, "text/plain", "SD Card indisponível");
      }
    } else {
      server.send(400, "text/plain", "Nome do arquivo não especificado");
    }
  });
  
  // Servir CSS do SD Card
  server.on("/style.css", HTTP_GET, []() {
    if (sdCardAvailable) {
      FILE* file = fopen("/sdcard/web/style.css", "r");
      if (file) {
        String content = "";
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), file)) {
          content += buffer;
        }
        fclose(file);
        server.send(200, "text/css", content);
        Serial.println("Servindo style.css do SD Card");
        return;
      }
    }
    server.send(200, "text/css", getDefaultCSS());
    Serial.println("Servindo style.css padrão");
  });
  
  // Servir JavaScript do SD Card
  server.on("/script.js", HTTP_GET, []() {
    if (sdCardAvailable) {
      FILE* file = fopen("/sdcard/web/script.js", "r");
      if (file) {
        String content = "";
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), file)) {
          content += buffer;
        }
        fclose(file);
        server.send(200, "application/javascript", content);
        Serial.println("Servindo script.js do SD Card");
        return;
      }
    }
    server.send(200, "application/javascript", getDefaultJS());
    Serial.println("Servindo script.js padrão");
  });
}

String getDefaultHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PrintSense</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <h1>PrintSense</h1>
        <div class="material-selector">
            <label>Material:</label>
            <select id="materialSelect" onchange="changeMaterial()">
                <option value="PLA">PLA</option>
                <option value="PETG">PETG</option>
                <option value="ABS">ABS>
                <option value="RESINA">Resina</option>
            </select>
        </div>
        <div class="status-card" id="statusCard">
            <h2 id="statusTitle">Carregando...</h2>
            <p id="statusDetails"></p>
        </div>
        <div class="sensors">
            <div class="sensor">
                <h3>Temperatura</h3>
                <div id="temperature">--</div>
                <div id="tempRange">--</div>
            </div>
            <div class="sensor">
                <h3>Umidade</h3>
                <div id="humidity">--</div>
                <div id="humRange">--</div>
            </div>
            <div class="sensor">
                <h3>Luz</h3>
                <div id="light">--</div>
            </div>
            <div class="sensor">
                <h3>Som</h3>
                <div id="sound">--</div>
            </div>
        </div>
        <p>Última: <span id="lastUpdate">--</span></p>
    </div>
    <script src="/script.js"></script>
</body>
</html>
)rawliteral";
}

String getDefaultCSS() {
  return R"rawliteral(
body{font-family:Arial;background:#667eea;color:#fff;margin:0;padding:20px}
.container{max-width:800px;margin:0 auto}
h1{text-align:center;font-size:3em}
.material-selector{background:#fff;color:#000;padding:15px;border-radius:10px;margin:20px 0;text-align:center}
select{padding:10px;font-size:1.1em;border:2px solid #667eea;border-radius:5px}
.status-card{background:#fff;color:#000;padding:30px;border-radius:15px;margin:20px 0;text-align:center}
.sensors{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:15px;margin:20px 0}
.sensor{background:#fff;color:#000;padding:20px;border-radius:10px;text-align:center}
.sensor h3{margin:0 0 10px 0;font-size:1em}
.sensor div{font-size:2em;font-weight:bold;color:#667eea}
)rawliteral";
}

String getDefaultJS() {
  return R"rawliteral(
let currentMaterial='PLA';
function updateData(){
fetch('/api/data').then(r=>r.json()).then(d=>{
document.getElementById('temperature').textContent=d.temperature.toFixed(1)+'°C';
document.getElementById('humidity').textContent=d.humidity.toFixed(1)+'%';
document.getElementById('light').textContent=d.light;
document.getElementById('sound').textContent=d.sound;
document.getElementById('tempRange').textContent='Ideal: '+d.thresholds.tempMin+'-'+d.thresholds.tempMax+'°C';
document.getElementById('humRange').textContent='Ideal: '+d.thresholds.humMin+'-'+d.thresholds.humMax+'%';
document.getElementById('statusTitle').textContent=d.status+' para '+d.material;
document.getElementById('statusDetails').textContent=d.statusDetails;
document.getElementById('lastUpdate').textContent=d.timestamp;
document.getElementById('materialSelect').value=d.material;
currentMaterial=d.material;
}).catch(e=>console.error(e));}
function changeMaterial(){
const m=document.getElementById('materialSelect').value;
fetch('/api/material',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'material='+m})
.then(r=>r.json()).then(d=>{if(d.success){currentMaterial=m;updateData();}});}
setInterval(updateData,2000);updateData();
)rawliteral";
}
