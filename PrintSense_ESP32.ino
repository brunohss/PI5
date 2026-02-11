/*
 * PrintSense - Monitor Ambiental para Impressão 3D
 * ESP32-S3 WROOM1 com MicroSD Card (Modo SDMMC)
 * 
 * Pinos SD Card (SDMMC - 1-bit mode):
 * - CLK  = GPIO 39
 * - CMD  = GPIO 38
 * - DATA0= GPIO 40
 * 
 * Sensores:
 * - DHT22: Temperatura e Umidade (GPIO 4)
 * - BH1750: Luminosidade (I2C)
 * - KY-037: Nível de Ruído (GPIO 34)
 * 
 * Funcionalidades:
 * - WebServer com dashboard
 * - Log em SD Card
 * - Status para cada material
 */

#include <WiFi.h>
#include <WebServer.h>
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "FS.h"
#include "SD_MMC.h"
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <ArduinoJson.h>
#include <time.h>

// ==================== CONFIGURAÇÕES ====================
// WiFi - Modo AP (Access Point)
const char* ssid = "PrintSense";
const char* password = "printsense123";

// WiFi - Modo Station (conectar à rede existente)
// const char* ssid = "SUA_REDE";
// const char* password = "SUA_SENHA";

// Pinos
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define SOUND_PIN 34

// Pinos SD Card - SDMMC (1-bit mode)
#define SD_CLK_PIN GPIO_NUM_39
#define SD_CMD_PIN GPIO_NUM_38
#define SD_D0_PIN  GPIO_NUM_40

// Configuração de tempo
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;  // GMT-3 (Brasil)
const int daylightOffset_sec = 0;

// ==================== OBJETOS ====================
DHT dht(DHT_PIN, DHT_TYPE);
BH1750 lightMeter;
WebServer server(80);

// Variável global SD Card
bool sdCardAvailable = false;
sdmmc_card_t* sdCard = nullptr;

// ==================== VARIÁVEIS GLOBAIS ====================
struct SensorData {
  float temperature;
  float humidity;
  uint16_t light;
  int sound;
  unsigned long timestamp;
};

SensorData currentData;
String currentMaterial = "PLA";  // Material selecionado
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 60000;  // Log a cada 1 minuto

// Thresholds para cada material
struct MaterialThresholds {
  String name;
  float tempMin, tempMax;
  float humMin, humMax;
  int lightMax;
  int soundMax;
};

MaterialThresholds materials[] = {
  {"PLA",   18, 28, 40, 60, 500, 70},
  {"PETG",  20, 30, 30, 50, 500, 70},
  {"ABS",   22, 32, 20, 40, 500, 70},
  {"RESINA", 20, 25, 40, 60, 100, 60}
};

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(2000);  // Aguardar estabilização
  
  Serial.println("\n=================================");
  Serial.println("PrintSense - Iniciando...");
  Serial.println("=================================\n");
  
  // ===== INICIALIZAR DHT22 =====
  Serial.println("[1/4] Inicializando DHT22...");
  pinMode(DHT_PIN, INPUT_PULLUP);  // Ativar pull-up interno
  delay(100);
  dht.begin();
  delay(2000);  // DHT22 precisa de tempo para estabilizar
  
  // Testar leitura
  float testTemp = dht.readTemperature();
  if (isnan(testTemp)) {
    Serial.println("⚠️  DHT22: Sensor não responde!");
    Serial.println("   Verifique:");
    Serial.println("   - Conexões (VCC, GND, DATA em GPIO 4)");
    Serial.println("   - Resistor pull-up 4.7kΩ entre VCC e DATA");
    Serial.println("   - Sensor não está defeituoso");
  } else {
    Serial.print("✅ DHT22 OK! Temp inicial: ");
    Serial.print(testTemp);
    Serial.println("°C");
  }
  
  // ===== INICIALIZAR I2C e BH1750 =====
  Serial.println("\n[2/4] Inicializando BH1750...");
  Wire.begin();
  delay(100);
  
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("⚠️  BH1750: Sensor não encontrado!");
    Serial.println("   Verifique conexões I2C (SDA: GPIO 21, SCL: GPIO 22)");
  } else {
    Serial.println("✅ BH1750 OK!");
  }
  
  // ===== INICIALIZAR SD CARD (SDMMC) =====
  Serial.println("\n[3/4] Inicializando SD Card (SDMMC)...");
  sdCardAvailable = initSDCard();
  
  if (sdCardAvailable) {
    Serial.println("✅ SD Card OK!");
    printSDCardInfo();
    createDirectories();
    Serial.println("   Estrutura de pastas criada");
  } else {
    Serial.println("❌ SD Card falhou!");
    Serial.println("   ⚠️  Sistema continuará SEM logs!");
  }
  
  // ===== CONFIGURAR WiFi =====
  Serial.println("\n[4/4] Configurando WiFi...");
  
  // Modo AP
  WiFi.softAP(ssid, password);
  Serial.print("✅ AP criado! IP: ");
  Serial.println(WiFi.softAPIP());
  
  // OU Modo Station (descomentar para conectar à rede)
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("\n✅ WiFi conectado!");
  // Serial.print("IP: ");
  // Serial.println(WiFi.localIP());
  
  // Configurar NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Configurar rotas do servidor
  setupWebServer();
  
  server.begin();
  Serial.println("\n✅ WebServer iniciado!");
  Serial.println("=================================");
  Serial.print("Acesse: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("=================================\n");
}
  
  // Configurar WiFi - Modo AP
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
  // OU Modo Station (descomentar para conectar à rede)
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("\nWiFi conectado!");
  // Serial.print("IP: ");
  // Serial.println(WiFi.localIP());
  
  // Configurar NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Configurar rotas do servidor
  setupWebServer();
  
  server.begin();
  Serial.println("WebServer iniciado!");
  Serial.println("Acesse: http://192.168.4.1");
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  
  // Ler sensores
  readSensors();
  
  // Log periódico
  if (millis() - lastLogTime > LOG_INTERVAL) {
    logToSD();
    lastLogTime = millis();
  }
  
  delay(1000);
}

// ==================== FUNÇÕES DE LEITURA ====================
void readSensors() {
  currentData.temperature = dht.readTemperature();
  currentData.humidity = dht.readHumidity();
  currentData.light = lightMeter.readLightLevel();
  currentData.sound = analogRead(SOUND_PIN);
  currentData.timestamp = millis();
  
  // Verificar DHT22
  if (isnan(currentData.temperature) || isnan(currentData.humidity)) {
    Serial.println("⚠️  DHT22 retornando NaN - verifique sensor");
    // Manter valores anteriores se disponíveis
    // ou usar valores padrão para não quebrar o sistema
    if (isnan(currentData.temperature)) {
      currentData.temperature = 0.0;
    }
    if (isnan(currentData.humidity)) {
      currentData.humidity = 0.0;
    }
  } else {
    // Debug apenas quando valores são válidos
    Serial.printf("✓ Temp: %.1f°C | Umid: %.1f%% | Luz: %d lux | Som: %d\n", 
                  currentData.temperature, currentData.humidity, 
                  currentData.light, currentData.sound);
  }
}

// ==================== SD CARD (SDMMC) ====================
bool initSDCard() {
  Serial.printf("   Pinos SDMMC: CLK=%d, CMD=%d, DATA0=%d\n", 
                SD_CLK_PIN, SD_CMD_PIN, SD_D0_PIN);

  // Configurar SDMMC host
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
  
  // Configurar SDMMC slot
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.clk = SD_CLK_PIN;
  slot_config.cmd = SD_CMD_PIN;
  slot_config.d0 = SD_D0_PIN;
  slot_config.width = 1;  // 1-bit mode
  
  // Configuração de montagem
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
  };
  
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &sdCard);
  
  if (ret != ESP_OK) {
    Serial.printf("   SDMMC falhou: 0x%x - %s\n", ret, esp_err_to_name(ret));
    
    // Tentar formatar
    Serial.println("   Tentando formatar SD card...");
    mount_config.format_if_mount_failed = true;
    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &sdCard);
    
    if (ret != ESP_OK) {
      Serial.printf("   Formatação falhou: 0x%x - %s\n", ret, esp_err_to_name(ret));
      return false;
    }
  }
  
  return true;
}

void printSDCardInfo() {
  if (!sdCardAvailable || !sdCard) return;
  
  Serial.print("   Nome: ");
  Serial.println(sdCard->cid.name);
  
  Serial.print("   Tipo: ");
  if (sdCard->ocr & 0x40000000) {
    Serial.println("SDHC/SDXC");
  } else {
    Serial.println("SDSC");
  }
  
  uint64_t cardSize = (uint64_t)sdCard->csd.capacity * sdCard->csd.sector_size;
  Serial.printf("   Capacidade: %.2f GB\n", (float)cardSize / (1024 * 1024 * 1024));
  Serial.printf("   Frequência: %d kHz\n", sdCard->max_freq_khz);
}

void createDirectories() {
  // Criar estrutura de pastas
  mkdir("/sdcard/logs", 0777);
  mkdir("/sdcard/jobs", 0777);
  mkdir("/sdcard/web", 0777);
}

void logToSD() {
  // Verificar se SD está disponível
  if (!sdCardAvailable) {
    Serial.println("[LOG] SD Card não disponível - pulando log");
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
    Serial.println("[LOG] ✓ Salvo: " + logLine);
  } else {
    Serial.println("[LOG] ❌ Erro ao abrir arquivo: " + filename);
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
  
  // Criar cabeçalho se arquivo não existir
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

// ==================== ANÁLISE DE STATUS ====================
String getStatus() {
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  
  bool tempOK = (currentData.temperature >= mat.tempMin && currentData.temperature <= mat.tempMax);
  bool humOK = (currentData.humidity >= mat.humMin && currentData.humidity <= mat.humMax);
  bool lightOK = (currentData.light <= mat.lightMax);
  bool soundOK = (currentData.sound <= mat.soundMax);
  
  if (tempOK && humOK && lightOK && soundOK) {
    return "IDEAL";
  } else if (tempOK && humOK) {
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
  
  if (currentData.light > mat.lightMax) {
    details += "Muita luz (crítico para resina). ";
  }
  
  if (currentData.sound > mat.soundMax) {
    details += "Ambiente ruidoso. ";
  }
  
  if (details == "") {
    details = "Todas as condições ideais!";
  }
  
  return details;
}

MaterialThresholds getMaterialThresholds(String material) {
  for (int i = 0; i < 4; i++) {
    if (materials[i].name == material) {
      return materials[i];
    }
  }
  return materials[0];  // Default PLA
}

// ==================== WEB SERVER ====================
void setupWebServer() {
  // Rota principal - servir index.html do SD
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
        return;
      }
    }
    server.send(200, "text/html", getDefaultHTML());
  });
  
  // API - Dados atuais
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
  
  // API - Alterar material
  server.on("/api/material", HTTP_POST, []() {
    if (server.hasArg("material")) {
      currentMaterial = server.arg("material");
      server.send(200, "application/json", "{\"success\":true,\"material\":\"" + currentMaterial + "\"}");
    } else {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Material não especificado\"}");
    }
  });
  
  // API - Listar logs
  server.on("/api/logs", HTTP_GET, []() {
    StaticJsonDocument<2048> doc;
    JsonArray logs = doc.createNestedArray("logs");
    
    if (sdCardAvailable) {
      DIR* dir = opendir("/sdcard/logs");
      if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
          if (entry->d_type == DT_REG) {  // Arquivo regular
            logs.add(entry->d_name);
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
          // Ler conteúdo do arquivo
          String content = "";
          char buffer[256];
          while (fgets(buffer, sizeof(buffer), file)) {
            content += buffer;
          }
          fclose(file);
          
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
  
  // Servir CSS
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
        return;
      }
    }
    server.send(200, "text/css", getDefaultCSS());
  });
  
  // Servir JS
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
        return;
      }
    }
    server.send(200, "application/javascript", getDefaultJS());
  });
}

// ==================== HTML/CSS/JS PADRÃO ====================
String getDefaultHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PrintSense - Monitor Ambiental</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>🖨️ PrintSense</h1>
            <p>Monitor Ambiental para Impressão 3D</p>
        </header>

        <div class="material-selector">
            <label>Material:</label>
            <select id="materialSelect" onchange="changeMaterial()">
                <option value="PLA">PLA</option>
                <option value="PETG">PETG</option>
                <option value="ABS">ABS</option>
                <option value="RESINA">Resina</option>
            </select>
        </div>

        <div class="status-card" id="statusCard">
            <h2 id="statusTitle">Carregando...</h2>
            <p id="statusDetails"></p>
        </div>

        <div class="sensors-grid">
            <div class="sensor-card">
                <div class="sensor-icon">🌡️</div>
                <h3>Temperatura</h3>
                <div class="sensor-value" id="temperature">--</div>
                <div class="sensor-range" id="tempRange">--</div>
            </div>

            <div class="sensor-card">
                <div class="sensor-icon">💧</div>
                <h3>Umidade</h3>
                <div class="sensor-value" id="humidity">--</div>
                <div class="sensor-range" id="humRange">--</div>
            </div>

            <div class="sensor-card">
                <div class="sensor-icon">💡</div>
                <h3>Luminosidade</h3>
                <div class="sensor-value" id="light">--</div>
                <div class="sensor-unit">lux</div>
            </div>

            <div class="sensor-card">
                <div class="sensor-icon">🔊</div>
                <h3>Ruído</h3>
                <div class="sensor-value" id="sound">--</div>
                <div class="sensor-unit">dB aprox.</div>
            </div>
        </div>

        <div class="info-section">
            <h3>📊 Logs e Histórico</h3>
            <button onclick="window.location.href='/api/logs'">Ver Logs Disponíveis</button>
        </div>

        <footer>
            <p>Última atualização: <span id="lastUpdate">--</span></p>
        </footer>
    </div>
    <script src="/script.js"></script>
</body>
</html>
)rawliteral";
}

String getDefaultCSS() {
  return R"rawliteral(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    padding: 20px;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
}

header {
    text-align: center;
    color: white;
    margin-bottom: 30px;
}

header h1 {
    font-size: 3em;
    margin-bottom: 10px;
}

.material-selector {
    background: white;
    padding: 20px;
    border-radius: 10px;
    margin-bottom: 20px;
    text-align: center;
}

.material-selector select {
    padding: 10px 20px;
    font-size: 1.1em;
    border: 2px solid #667eea;
    border-radius: 5px;
    margin-left: 10px;
}

.status-card {
    background: white;
    padding: 30px;
    border-radius: 15px;
    margin-bottom: 30px;
    text-align: center;
    box-shadow: 0 10px 30px rgba(0,0,0,0.2);
}

.status-card h2 {
    font-size: 2.5em;
    margin-bottom: 10px;
}

.status-card.ideal { border-left: 10px solid #10b981; }
.status-card.bom { border-left: 10px solid #f59e0b; }
.status-card.ruim { border-left: 10px solid #ef4444; }

.sensors-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 20px;
    margin-bottom: 30px;
}

.sensor-card {
    background: white;
    padding: 25px;
    border-radius: 15px;
    text-align: center;
    box-shadow: 0 5px 15px rgba(0,0,0,0.1);
}

.sensor-icon {
    font-size: 3em;
    margin-bottom: 10px;
}

.sensor-value {
    font-size: 2.5em;
    font-weight: bold;
    color: #667eea;
    margin: 10px 0;
}

.sensor-range {
    color: #666;
    font-size: 0.9em;
}

.sensor-unit {
    color: #999;
    font-size: 0.9em;
}

.info-section {
    background: white;
    padding: 20px;
    border-radius: 10px;
    margin-bottom: 20px;
}

button {
    background: #667eea;
    color: white;
    border: none;
    padding: 12px 30px;
    border-radius: 5px;
    font-size: 1em;
    cursor: pointer;
    margin-top: 10px;
}

button:hover {
    background: #5568d3;
}

footer {
    text-align: center;
    color: white;
    margin-top: 20px;
}
)rawliteral";
}

String getDefaultJS() {
  return R"rawliteral(
let currentMaterial = 'PLA';

function updateData() {
    fetch('/api/data')
        .then(response => response.json())
        .then(data => {
            document.getElementById('temperature').textContent = data.temperature.toFixed(1) + '°C';
            document.getElementById('humidity').textContent = data.humidity.toFixed(1) + '%';
            document.getElementById('light').textContent = data.light;
            document.getElementById('sound').textContent = data.sound;
            
            document.getElementById('tempRange').textContent = 
                `Ideal: ${data.thresholds.tempMin}-${data.thresholds.tempMax}°C`;
            document.getElementById('humRange').textContent = 
                `Ideal: ${data.thresholds.humMin}-${data.thresholds.humMax}%`;
            
            const statusCard = document.getElementById('statusCard');
            statusCard.className = 'status-card ' + data.status.toLowerCase();
            
            let statusEmoji = data.status === 'IDEAL' ? '✅' : data.status === 'BOM' ? '⚠️' : '❌';
            document.getElementById('statusTitle').textContent = statusEmoji + ' ' + data.status + ' para ' + data.material;
            document.getElementById('statusDetails').textContent = data.statusDetails;
            
            document.getElementById('lastUpdate').textContent = data.timestamp;
        })
        .catch(error => console.error('Erro:', error));
}

function changeMaterial() {
    const material = document.getElementById('materialSelect').value;
    
    fetch('/api/material', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'material=' + material
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            currentMaterial = material;
            updateData();
        }
    })
    .catch(error => console.error('Erro:', error));
}

// Atualizar a cada 2 segundos
setInterval(updateData, 2000);
updateData();
)rawliteral";
}
