/*
 * PrintSense - Monitor Ambiental para Impressão 3D
 * ESP32-S3 WROOM1 N16R8 com MicroSD Card Embutido
 * 
 * VERSÃO CORRIGIDA:
 * - LDR ao invés de BH1750
 * - SD Card com pinout correto para ESP32-S3
 * 
 * Sensores:
 * - DHT22: Temperatura e Umidade (GPIO 4)
 * - LDR: Luminosidade (GPIO 34 - ADC)
 * - KY-037: Nível de Ruído (GPIO 35 - ADC)
 * 
 * SD Card (SDMMC - pinos fixos do ESP32-S3):
 * - CLK: GPIO 39
 * - CMD: GPIO 38  
 * - D0:  GPIO 40
 * - D1:  GPIO 41
 * - D2:  GPIO 42
 * - D3:  GPIO 43
 */

#include <WiFi.h>
#include <WebServer.h>
#include <SD_MMC.h>  // Usar SD_MMC ao invés de SD (mais rápido e usa pinos fixos)
#include <DHT.h>
#include <ArduinoJson.h>
#include <time.h>

// ==================== CONFIGURAÇÕES ====================
// WiFi - Modo Station (conectar à rede existente)
const char* ssid = "SUA_REDE";
const char* password = "SUA_SENHA";

// WiFi - Modo AP (Access Point) - Descomente se preferir
// const char* ssid = "PrintSense";
// const char* password = "printsense123";

// Pinos dos Sensores
#define DHT_PIN 4         // DHT22 - Temperatura e Umidade
#define DHT_TYPE DHT22
#define LDR_PIN 34        // LDR - Luminosidade (ADC1)
#define SOUND_PIN 35      // KY-037 - Som (ADC1)

// Configuração LDR
#define LDR_RESISTOR 10000  // Resistor em série com LDR (10kΩ)
const int LDR_MAX = 4095;   // Valor ADC máximo (12-bit)
const int LDR_MIN = 0;      // Valor ADC mínimo

// Configuração de tempo (NTP)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;  // GMT-3 (Brasil)
const int daylightOffset_sec = 0;

// ==================== OBJETOS ====================
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

// ==================== VARIÁVEIS GLOBAIS ====================
struct SensorData {
  float temperature;
  float humidity;
  int lightRaw;      // Valor bruto do LDR (0-4095)
  int lightLux;      // Valor aproximado em lux
  int sound;
  unsigned long timestamp;
};

SensorData currentData;
String currentMaterial = "PLA";
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 60000;  // Log a cada 1 minuto

// Thresholds para cada material
struct MaterialThresholds {
  String name;
  float tempMin, tempMax;
  float humMin, humMax;
  int lightMax;      // Valor máximo de luz (em lux aproximado)
  int soundMax;
};

MaterialThresholds materials[] = {
  {"PLA",    18, 28, 40, 60, 500, 70},
  {"PETG",   20, 30, 30, 50, 500, 70},
  {"ABS",    22, 32, 20, 40, 500, 70},
  {"RESINA", 20, 25, 40, 60, 100, 60}
};

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("PrintSense - Iniciando...");
  Serial.println("ESP32-S3 WROOM1 N16R8");
  Serial.println("=================================\n");
  
  // Configurar ADC para leitura dos sensores analógicos
  analogReadResolution(12);  // 12-bit ADC (0-4095)
  analogSetAttenuation(ADC_11db);  // Range completo 0-3.3V
  
  // Inicializar DHT22
  Serial.println("[DHT22] Inicializando sensor de temperatura e umidade...");
  dht.begin();
  delay(2000);  // DHT22 precisa de tempo para estabilizar
  
  // Testar DHT22
  float testTemp = dht.readTemperature();
  if (isnan(testTemp)) {
    Serial.println("[DHT22] ERRO: Sensor não responde!");
    Serial.println("Verifique:");
    Serial.println("  - Conexão no GPIO 4");
    Serial.println("  - Alimentação 5V");
    Serial.println("  - Resistor pull-up (ou use módulo com pull-up)");
  } else {
    Serial.printf("[DHT22] OK! Temperatura inicial: %.1f°C\n", testTemp);
  }
  
  // Inicializar LDR (apenas teste)
  Serial.println("\n[LDR] Inicializando sensor de luminosidade...");
  int ldrTest = analogRead(LDR_PIN);
  Serial.printf("[LDR] OK! Valor inicial: %d (0-4095)\n", ldrTest);
  Serial.println("Circuito LDR:");
  Serial.println("  3.3V ─── LDR ─── GPIO34 ─── Resistor 10kΩ ─── GND");
  
  // Inicializar SD Card usando SDMMC (1-bit mode)
  Serial.println("\n[SD CARD] Inicializando...");
  Serial.println("Pinos SDMMC do ESP32-S3:");
  Serial.println("  CLK:  GPIO 39");
  Serial.println("  CMD:  GPIO 38");
  Serial.println("  D0:   GPIO 40");
  
  // Tentar montar SD Card em modo 1-bit (apenas D0)
  if (!SD_MMC.begin("/sdcard", true)) {  // true = 1-bit mode
    Serial.println("[SD CARD] ERRO: Falha ao montar!");
    Serial.println("\nVerifique:");
    Serial.println("  1. Cartão inserido corretamente");
    Serial.println("  2. Cartão formatado em FAT32");
    Serial.println("  3. Cartão funcionando (teste em PC)");
    Serial.println("  4. Pinos corretos conectados");
    Serial.println("\n⚠️  Sistema continuará SEM logs em SD Card");
  } else {
    uint8_t cardType = SD_MMC.cardType();
    
    if (cardType == CARD_NONE) {
      Serial.println("[SD CARD] ERRO: Nenhum cartão detectado!");
    } else {
      Serial.println("[SD CARD] ✓ Montado com sucesso!");
      
      // Informações do cartão
      Serial.print("Tipo: ");
      if (cardType == CARD_MMC) Serial.println("MMC");
      else if (cardType == CARD_SD) Serial.println("SDSC");
      else if (cardType == CARD_SDHC) Serial.println("SDHC");
      else Serial.println("UNKNOWN");
      
      uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
      Serial.printf("Tamanho: %lluMB\n", cardSize);
      
      // Criar estrutura de pastas
      createDirectories();
    }
  }
  
  // Configurar WiFi
  Serial.println("\n[WiFi] Conectando...");
  
  // Modo Station (conectar à rede)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] ✓ Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("\nAcesse o dashboard em:");
    Serial.print("http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] ERRO: Não conectou!");
    Serial.println("Iniciando modo AP...");
    
    // Fallback para modo AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP("PrintSense", "printsense123");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Acesse: http://192.168.4.1");
  }
  
  // Configurar NTP
  Serial.println("\n[NTP] Sincronizando horário...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Configurar rotas do servidor
  setupWebServer();
  
  server.begin();
  Serial.println("\n[WebServer] ✓ Iniciado!");
  
  Serial.println("\n=================================");
  Serial.println("Sistema pronto!");
  Serial.println("=================================\n");
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
  // Ler DHT22
  currentData.temperature = dht.readTemperature();
  currentData.humidity = dht.readHumidity();
  
  // Ler LDR
  currentData.lightRaw = analogRead(LDR_PIN);
  currentData.lightLux = convertLDRtoLux(currentData.lightRaw);
  
  // Ler sensor de som
  currentData.sound = analogRead(SOUND_PIN);
  
  currentData.timestamp = millis();
  
  // Debug a cada 5 segundos
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) {
    if (!isnan(currentData.temperature)) {
      Serial.printf("Temp: %.1f°C | Umid: %.1f%% | Luz: %d lux (raw:%d) | Som: %d\n", 
                    currentData.temperature, 
                    currentData.humidity, 
                    currentData.lightLux,
                    currentData.lightRaw,
                    currentData.sound);
    } else {
      Serial.println("⚠️  DHT22 retornando NaN - verifique sensor");
    }
    lastDebug = millis();
  }
}

// Converter leitura do LDR para lux aproximado
int convertLDRtoLux(int rawValue) {
  // Fórmula aproximada (calibre conforme seu LDR)
  // Quanto menor o valor ADC, mais luz (LDR com resistência menor)
  // Quanto maior o valor ADC, menos luz (LDR com resistência maior)
  
  // Inversão: 4095 (escuro) → 0 lux, 0 (claro) → 1000 lux
  int lux = map(rawValue, 0, 4095, 1000, 0);
  
  // Garantir valores positivos
  if (lux < 0) lux = 0;
  
  return lux;
}

// ==================== SD CARD ====================
void createDirectories() {
  if (SD_MMC.cardType() == CARD_NONE) return;
  
  if (!SD_MMC.exists("/logs")) {
    SD_MMC.mkdir("/logs");
    Serial.println("[SD] Pasta /logs criada");
  }
  if (!SD_MMC.exists("/jobs")) {
    SD_MMC.mkdir("/jobs");
    Serial.println("[SD] Pasta /jobs criada");
  }
  if (!SD_MMC.exists("/web")) {
    SD_MMC.mkdir("/web");
    Serial.println("[SD] Pasta /web criada");
  }
}

void logToSD() {
  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println("[LOG] SD Card não disponível - pulando log");
    return;
  }
  
  String filename = getLogFilename();
  File file = SD_MMC.open(filename, FILE_APPEND);
  
  if (file) {
    String logLine = String(getFormattedTime()) + "," +
                     String(currentData.temperature, 1) + "," +
                     String(currentData.humidity, 1) + "," +
                     String(currentData.lightLux) + "," +
                     String(currentData.sound) + "," +
                     getStatus();
    
    file.println(logLine);
    file.close();
    Serial.println("[LOG] Salvo: " + logLine);
  } else {
    Serial.println("[LOG] Erro ao abrir arquivo");
  }
}

String getLogFilename() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "/logs/log.csv";
  }
  
  char filename[32];
  strftime(filename, sizeof(filename), "/logs/%Y%m%d.csv", &timeinfo);
  
  // Criar cabeçalho se arquivo não existir
  if (!SD_MMC.exists(filename)) {
    File file = SD_MMC.open(filename, FILE_WRITE);
    if (file) {
      file.println("timestamp,temperature,humidity,light,sound,status");
      file.close();
    }
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
  bool lightOK = (currentData.lightLux <= mat.lightMax);
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
  
  if (isnan(currentData.temperature) || isnan(currentData.humidity)) {
    return "Aguardando estabilização dos sensores...";
  }
  
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
  
  if (currentData.lightLux > mat.lightMax) {
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
  return materials[0];
}

// ==================== WEB SERVER ====================
void setupWebServer() {
  // Rota principal
  server.on("/", HTTP_GET, []() {
    if (SD_MMC.cardType() != CARD_NONE && SD_MMC.exists("/web/index.html")) {
      File file = SD_MMC.open("/web/index.html", FILE_READ);
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(200, "text/html", getDefaultHTML());
    }
  });
  
  // API - Dados atuais
  server.on("/api/data", HTTP_GET, []() {
    StaticJsonDocument<512> doc;
    
    doc["temperature"] = currentData.temperature;
    doc["humidity"] = currentData.humidity;
    doc["light"] = currentData.lightLux;
    doc["lightRaw"] = currentData.lightRaw;
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
    
    if (SD_MMC.cardType() != CARD_NONE) {
      File root = SD_MMC.open("/logs");
      File file = root.openNextFile();
      while (file) {
        if (!file.isDirectory()) {
          logs.add(file.name());
        }
        file = root.openNextFile();
      }
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  // API - Download de log
  server.on("/api/log", HTTP_GET, []() {
    if (server.hasArg("file")) {
      String filename = "/logs/" + server.arg("file");
      if (SD_MMC.cardType() != CARD_NONE && SD_MMC.exists(filename)) {
        File file = SD_MMC.open(filename, FILE_READ);
        server.streamFile(file, "text/csv");
        file.close();
      } else {
        server.send(404, "text/plain", "Arquivo não encontrado");
      }
    } else {
      server.send(400, "text/plain", "Nome do arquivo não especificado");
    }
  });
  
  // Servir CSS
  server.on("/style.css", HTTP_GET, []() {
    if (SD_MMC.cardType() != CARD_NONE && SD_MMC.exists("/web/style.css")) {
      File file = SD_MMC.open("/web/style.css", FILE_READ);
      server.streamFile(file, "text/css");
      file.close();
    } else {
      server.send(200, "text/css", getDefaultCSS());
    }
  });
  
  // Servir JS
  server.on("/script.js", HTTP_GET, []() {
    if (SD_MMC.cardType() != CARD_NONE && SD_MMC.exists("/web/script.js")) {
      File file = SD_MMC.open("/web/script.js", FILE_READ);
      server.streamFile(file, "application/javascript");
      file.close();
    } else {
      server.send(200, "application/javascript", getDefaultJS());
    }
  });
}

// ==================== HTML/CSS/JS EMBUTIDO ====================
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
                <div class="sensor-unit">lux (LDR)</div>
            </div>

            <div class="sensor-card">
                <div class="sensor-icon">🔊</div>
                <h3>Ruído</h3>
                <div class="sensor-value" id="sound">--</div>
                <div class="sensor-unit">valor</div>
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
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    padding: 20px;
}
.container { max-width: 1200px; margin: 0 auto; }
header { text-align: center; color: white; margin-bottom: 30px; }
header h1 { font-size: 3em; margin-bottom: 10px; }
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
.status-card h2 { font-size: 2.5em; margin-bottom: 10px; }
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
.sensor-icon { font-size: 3em; margin-bottom: 10px; }
.sensor-value {
    font-size: 2.5em;
    font-weight: bold;
    color: #667eea;
    margin: 10px 0;
}
.sensor-range { color: #666; font-size: 0.9em; }
.sensor-unit { color: #999; font-size: 0.9em; }
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
button:hover { background: #5568d3; }
footer { text-align: center; color: white; margin-top: 20px; }
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
setInterval(updateData, 2000);
updateData();
)rawliteral";
}
