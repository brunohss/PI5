/*
 * ═══════════════════════════════════════════════════════════════
 *                    PRINTSENSE v3.1 FINAL
 * ═══════════════════════════════════════════════════════════════
 * 
 * Monitor Ambiental Profissional para Impressão 3D
 * ESP32-S3 WROOM1 com Display LCD 20x4 + LEDs + Conversão dB
 * 
 * HARDWARE:
 * ┌─────────────────────────────────────────────────────────────┐
 * │ Sensores:                                                   │
 * │  - DHT22 (Temp/Umid)      → GPIO 17                        │
 * │  - LDR (Luz)              → GPIO 4 (ADC)                   │
 * │  - MAX4466 (Som)          → GPIO 8 (ADC)                   │
 * │                                                             │
 * │ Display & Controle:                                         │
 * │  - LCD I2C 20x4           → SDA=10, SCL=13 (0x27)          │
 * │  - Encoder EC11           → CLK=7, DT=6, SW=5              │
 * │                                                             │
 * │ LEDs Indicadores (cada um com resistor próprio):            │
 * │  - Verde (IDEAL)          → GPIO 2 + 220Ω                  │
 * │  - Amarelo (BOM)          → GPIO 15 + 220Ω                 │
 * │  - Vermelho (RUIM)        → GPIO 16 + 220Ω                 │
 * │                                                             │
 * │ Armazenamento:                                              │
 * │  - SD Card SDMMC (1-bit)  → CLK=39, CMD=38, D0=40          │
 * │                                                             │
 * │ Conectividade:                                              │
 * │  - WiFi 802.11 b/g/n      → Dual mode (AP/Station)        │
 * │  - WebServer HTTP         → Porta 80                       │
 * └─────────────────────────────────────────────────────────────┘
 * 
 * FUNCIONALIDADES:
 *  ✅ Monitoramento de 4 sensores em tempo real
 *  ✅ Conversão de som para dB SPL (intuitivo para usuário)
 *  ✅ Display LCD com informações completas
 *  ✅ LEDs indicadores de status (Verde/Amarelo/Vermelho)
 *  ✅ Encoder rotativo para seleção de material
 *  ✅ Interface web profissional com gráficos Chart.js
 *  ✅ Logging em SD Card (formato CSV)
 *  ✅ API REST completa
 *  ✅ 4 perfis de material (PLA/PETG/ABS/RESINA)
 *  ✅ Histerese para evitar oscilação de status
 *  ✅ Leitura peak-to-peak do som (mais estável)
 * 
 * AUTOR: Equipe PrintSense
 * DATA: 2026
 * VERSÃO: 3.1 FINAL
 * ═══════════════════════════════════════════════════════════════
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

// ==================== CONFIGURAÇÕES WiFi ====================

const char* ssid     = "ssid";
const char* password = "password";

// ==================== PINOS - SENSORES ====================
#define DHTPIN 17
#define DHT_TYPE DHT22
#define LDR_PIN 4
#define MAX4466_PIN 8

// ==================== PINOS - ENCODER ====================
#define ENCODER_CLK 7
#define ENCODER_DT 6
#define ENCODER_SW 5

// ==================== PINOS - LEDs (cada um com resistor 220Ω próprio) ====================
#define LED_GREEN 2    // Verde - IDEAL
#define LED_YELLOW 15  // Amarelo - BOM
#define LED_RED 16     // Vermelho - RUIM

// ==================== PINOS - SD CARD SDMMC ====================
#define SD_CLK_PIN GPIO_NUM_39
#define SD_CMD_PIN GPIO_NUM_38
#define SD_D0_PIN  GPIO_NUM_40

// ==================== PINOS - LCD I2C ====================
#define I2C_SDA 10
#define I2C_SCL 13

// ==================== OBJETOS ====================
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHTPIN, DHT_TYPE);
WebServer server(80);

bool sdCardAvailable = false;
sdmmc_card_t* sdCard = nullptr;

// ==================== CONFIGURAÇÃO NTP ====================
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;  // UTC-3 (Brasília)
const int daylightOffset_sec = 0;

// ==================== ESTRUTURA DE DADOS ====================
struct SensorData {
  float temperature;
  float humidity;
  uint16_t light;
  int soundADC;        // Valor bruto ADC (0-4095)
  float soundDB;       // Valor convertido em dB SPL
  unsigned long timestamp;
};

SensorData currentData;

// ==================== MATERIAIS ====================
const char* materialNames[] = {"PLA", "PETG", "ABS", "RESINA"};
int currentMaterialIndex = 0;
String currentMaterial = "PLA";

// ==================== INTERVALOS ====================
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 60000;  // Log a cada 60 segundos

unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 1000;  // Atualizar LCD/LEDs a cada 1 segundo

// ==================== ENCODER ====================
volatile int encoderPos = 0;
int lastEncoderPos = 0;
bool encoderButtonPressed = false;
unsigned long lastEncoderTime = 0;
bool inSelectionMode = false;
unsigned long selectionModeStartTime = 0;
const unsigned long SELECTION_TIMEOUT = 10000;  // 10 segundos

// ==================== THRESHOLDS POR MATERIAL ====================
struct MaterialThresholds {
  const char* name;
  float tempMin;
  float tempMax;
  float humMin;
  float humMax;
  int lightMax;
  int soundMaxADC;    // Limite em ADC (backup)
  float soundMaxDB;   // Limite em dB SPL
};

MaterialThresholds materials[] = {
  // Nome      TempMin TempMax HumMin HumMax LightMax SoundADC SoundDB
  {"PLA",      18.0,   28.0,   40.0,  60.0,  3000,    800,     70.0},
  {"PETG",     20.0,   30.0,   30.0,  50.0,  3000,    800,     70.0},
  {"ABS",      22.0,   32.0,   20.0,  40.0,  3000,    800,     70.0},
  {"RESINA",   20.0,   25.0,   40.0,  60.0,  1000,    600,     65.0}
};

// ==================== PROTÓTIPOS ====================
void initLCD();
void initEncoder();
void initDHT();
void initSDCard();
void initLEDs();
void testLEDs();
void updateLEDs();
String getLEDStatus();
void createDirectories();
void readSensors();
int readSoundPeakToPeak();
float convertToDBSPL(int adcPeakToPeak);
void updateLCD();
void displaySelectionMode();
void logToSD();
String getLogFilename();
String getFormattedTime();
String getStatus();
String getStatusDetails();
MaterialThresholds getMaterialThresholds(String material);
void setupWebServer();
void IRAM_ATTR encoderISR();
void IRAM_ATTR encoderButtonISR();

// ═══════════════════════════════════════════════════════════════
//                         FUNÇÕES DOS LEDs
// ═══════════════════════════════════════════════════════════════

void initLEDs() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  // Todos apagados inicialmente
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

void testLEDs() {
  Serial.print("   Testando LEDs: ");
  
  // Verde
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  Serial.print("🟢");
  delay(300);
  
  // Amarelo
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, LOW);
  Serial.print("→🟡");
  delay(300);
  
  // Vermelho
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  Serial.println("→🔴");
  delay(300);
  
  // Todos apagados
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

void updateLEDs() {
  String status = getStatus();
  
  // SEMPRE desliga todos primeiro (segurança contra oscilação)
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  
  // Pequeno delay para garantir
  delayMicroseconds(10);
  
  // Liga apenas o LED correspondente
  if (status == "IDEAL") {
    digitalWrite(LED_GREEN, HIGH);
  } else if (status == "BOM") {
    digitalWrite(LED_YELLOW, HIGH);
  } else {  // RUIM
    digitalWrite(LED_RED, HIGH);
  }
}

String getLEDStatus() {
  if (digitalRead(LED_GREEN) == HIGH) {
    return "GREEN";
  } else if (digitalRead(LED_YELLOW) == HIGH) {
    return "YELLOW";
  } else if (digitalRead(LED_RED) == HIGH) {
    return "RED";
  }
  return "NONE";
}

// ═══════════════════════════════════════════════════════════════
//                    LEITURA DO SOM (MAX4466)
// ═══════════════════════════════════════════════════════════════

int readSoundPeakToPeak() {
  /*
   * Lê o nível de som usando método peak-to-peak em 50ms
   * Retorna: Diferença entre pico máximo e mínimo (mais estável que instantâneo)
   */
  int signalMax = 0;
  int signalMin = 4095;
  unsigned long startTime = micros();
  
  // Coletar amostras por 50ms (tempo suficiente para capturar variações)
  while (micros() - startTime < 50000) {
    int sample = analogRead(MAX4466_PIN);
    
    if (sample > signalMax) {
      signalMax = sample;
    }
    if (sample < signalMin) {
      signalMin = sample;
    }
    
    delayMicroseconds(500);  // 500µs entre amostras
  }
  
  return signalMax - signalMin;
}

float convertToDBSPL(int adcPeakToPeak) {
  /*
   * Converte valor ADC peak-to-peak para dB SPL (aproximado)
   * Baseado em calibração empírica
   * 
   * AJUSTE ESTES VALORES conforme seu ambiente e ganho do potenciômetro!
   * Use o código de calibração para encontrar os valores ideais.
   */
  
  // Pontos de referência (CALIBRAR no seu ambiente!)
  const int SILENCE_ADC = 50;      // ~30 dB (muito silencioso)
  const int WHISPER_ADC = 150;     // ~40 dB (sussurro/biblioteca)
  const int QUIET_ADC = 300;       // ~50 dB (escritório calmo)
  const int NORMAL_ADC = 600;      // ~60 dB (conversa normal)
  const int MODERATE_ADC = 1000;   // ~70 dB (impressora 3D)
  const int LOUD_ADC = 1500;       // ~80 dB (aspirador)
  const int VERY_LOUD_ADC = 2500;  // ~90 dB (tráfego pesado)
  
  float dbSPL;
  
  // Interpolação linear entre pontos conhecidos
  if (adcPeakToPeak < WHISPER_ADC) {
    // 30-40 dB
    dbSPL = 30.0 + ((adcPeakToPeak - SILENCE_ADC) * 10.0) / (WHISPER_ADC - SILENCE_ADC);
  } 
  else if (adcPeakToPeak < QUIET_ADC) {
    // 40-50 dB
    dbSPL = 40.0 + ((adcPeakToPeak - WHISPER_ADC) * 10.0) / (QUIET_ADC - WHISPER_ADC);
  }
  else if (adcPeakToPeak < NORMAL_ADC) {
    // 50-60 dB
    dbSPL = 50.0 + ((adcPeakToPeak - QUIET_ADC) * 10.0) / (NORMAL_ADC - QUIET_ADC);
  }
  else if (adcPeakToPeak < MODERATE_ADC) {
    // 60-70 dB
    dbSPL = 60.0 + ((adcPeakToPeak - NORMAL_ADC) * 10.0) / (MODERATE_ADC - NORMAL_ADC);
  }
  else if (adcPeakToPeak < LOUD_ADC) {
    // 70-80 dB
    dbSPL = 70.0 + ((adcPeakToPeak - MODERATE_ADC) * 10.0) / (LOUD_ADC - MODERATE_ADC);
  }
  else if (adcPeakToPeak < VERY_LOUD_ADC) {
    // 80-90 dB
    dbSPL = 80.0 + ((adcPeakToPeak - LOUD_ADC) * 10.0) / (VERY_LOUD_ADC - LOUD_ADC);
  }
  else {
    // >90 dB
    dbSPL = 90.0 + ((adcPeakToPeak - VERY_LOUD_ADC) * 10.0) / 1000.0;
    if (dbSPL > 110.0) dbSPL = 110.0;  // Limitar máximo
  }
  
  // Limitar mínimo
  if (dbSPL < 20.0) dbSPL = 20.0;
  
  return dbSPL;
}

// ═══════════════════════════════════════════════════════════════
//                            SETUP
// ═══════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║    PRINTSENSE v1.0 FINAL             ║");
  Serial.println("║    Monitor Ambiental Profissional    ║");
  Serial.println("╚═══════════════════════════════════════╝\n");
  
  // [0/7] LEDs
  Serial.println("[0/7] Configurando LEDs...");
  initLEDs();
  testLEDs();
  Serial.println("✅ LEDs configurados");
  Serial.println("   Verde (IDEAL): GPIO 2 + 220Ω");
  Serial.println("   Amarelo (BOM): GPIO 15 + 220Ω");
  Serial.println("   Vermelho (RUIM): GPIO 16 + 220Ω\n");
  
  // [1/7] LCD
  Serial.println("[1/7] Inicializando LCD...");
  initLCD();
  lcd.setCursor(0, 0);
  lcd.print("PrintSense v1.0");
  lcd.setCursor(0, 1);
  lcd.print("Inicializando...");
  
  // [2/7] Encoder
  Serial.println("[2/7] Configurando Encoder...");
  initEncoder();
  lcd.setCursor(0, 2);
  lcd.print("Encoder: OK");
  
  // [3/7] DHT22
  Serial.println("[3/7] Inicializando DHT22...");
  initDHT();
  delay(2000);
  float testTemp = dht.readTemperature();
  if (!isnan(testTemp)) {
    Serial.printf("✅ DHT22 OK! Temp: %.1f°C\n\n", testTemp);
    lcd.setCursor(0, 3);
    lcd.print("DHT22: OK ");
    lcd.print(testTemp, 1);
    lcd.print("C");
  } else {
    Serial.println("⚠️  DHT22: Aguarde estabilização\n");
    lcd.setCursor(0, 3);
    lcd.print("DHT22: Aguarde...");
  }
  
  delay(2000);
  
  // [4/7] SD Card
  Serial.println("[4/7] Inicializando SD Card (SDMMC)...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Montando SD Card...");
  initSDCard();
  
  
  if (sdCardAvailable) {
    createDirectories();
    lcd.setCursor(0, 1);
    lcd.print("SD: OK");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SD: Erro (opcional)");
  }
  
  delay(1000);
  
  // [5/7] WiFi
  Serial.println("[5/7] Configurando WiFi...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi...");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi conectado!");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    
    lcd.setCursor(0, 2);
    lcd.print("WiFi: OK");
    lcd.setCursor(0, 3);
    lcd.print(WiFi.localIP());
  } else {
    Serial.println("⚠️  WiFi falhou - modo AP");
    WiFi.softAP("PrintSense", "printsense123");
    lcd.setCursor(0, 2);
    lcd.print("WiFi: Modo AP");
    lcd.setCursor(0, 3);
    lcd.print("192.168.4.1");
  }
  
  // [6/7] NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  delay(2000);
  
  // [7/7] WebServer
  Serial.println("\n[7/7] Iniciando WebServer...");
  setupWebServer();
  server.begin();
  Serial.println("✅ WebServer iniciado!\n");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PrintSense  PRONTO!");
  lcd.setCursor(0, 1);
  lcd.print("Modo: ");
  lcd.print(currentMaterial);
  lcd.setCursor(0, 2);
  lcd.print("Acesse via web:");
  lcd.setCursor(0, 3);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print(WiFi.localIP());
  } else {
    lcd.print("192.168.4.1");
  }
  
  delay(3000);
  
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║        SISTEMA PRONTO!               ║");
  Serial.println("╚═══════════════════════════════════════╝\n");
  
  // Primeira leitura
  readSensors();
  updateLCD();
  updateLEDs();
}

// ═══════════════════════════════════════════════════════════════
//                            LOOP
// ═══════════════════════════════════════════════════════════════

void loop() {
  server.handleClient();
  
  // Atualizar sensores, display e LEDs
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    readSensors();
    
    if (!inSelectionMode) {
      updateLCD();
      updateLEDs();
    } else {
      displaySelectionMode();
      
      // Timeout do modo seleção
      if (millis() - selectionModeStartTime > SELECTION_TIMEOUT) {
        inSelectionMode = false;
        Serial.println("Modo seleção: timeout");
      }
    }
    
    lastUpdate = millis();
  }
  
  // Log periódico no SD Card
  if (millis() - lastLogTime >= LOG_INTERVAL) {
    logToSD();
    lastLogTime = millis();
  }
  
  // Detectar mudança no encoder
  if (encoderPos != lastEncoderPos) {
    if (inSelectionMode) {
      int diff = encoderPos - lastEncoderPos;
      currentMaterialIndex += diff;
      
      if (currentMaterialIndex >= 4) currentMaterialIndex = 0;
      if (currentMaterialIndex < 0) currentMaterialIndex = 3;
      
      Serial.print("Navegando: ");
      Serial.println(materialNames[currentMaterialIndex]);
    }
    lastEncoderPos = encoderPos;
  }
  
  // Detectar botão do encoder
  if (encoderButtonPressed) {
    encoderButtonPressed = false;
    
    if (!inSelectionMode) {
      // Entrar em modo seleção
      inSelectionMode = true;
      selectionModeStartTime = millis();
      Serial.println("Modo seleção: ATIVADO");
    } else {
      // Confirmar seleção
      currentMaterial = String(materialNames[currentMaterialIndex]);
      inSelectionMode = false;
      Serial.print("Material confirmado: ");
      Serial.println(currentMaterial);
      
      // Atualizar LEDs imediatamente
      updateLEDs();
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//                        FUNÇÕES DO LCD
// ═══════════════════════════════════════════════════════════════

void initLCD() {
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
  Serial.println("✅ LCD inicializado (I2C: SDA=10, SCL=13)\n");
}

void updateLCD() {
  lcd.clear();
  
  // Linha 0: Material e Status
  lcd.setCursor(0, 0);
  lcd.print(currentMaterial);
  
  String status = getStatus();
  lcd.setCursor(5, 0);
  if (status == "IDEAL") {
    lcd.print("* IDEAL");
  } else if (status == "BOM") {
    lcd.print("! BOM");
  } else {
    lcd.print("X RUIM");
  }
  
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
  
  // Linha 3: Luz e Som (em dB!)
  lcd.setCursor(0, 3);
  lcd.print("Luz:");
  lcd.print(currentData.light);
  
  lcd.setCursor(10, 3);
  lcd.print("Som:");
  lcd.print(currentData.soundDB, 0);  // Mostrar em dB
  lcd.print("dB");
}

void displaySelectionMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SELECIONAR MATERIAL");
  
  lcd.setCursor(0, 1);
  lcd.print("--------------------");
  
  lcd.setCursor(0, 2);
  lcd.print("     >> ");
  lcd.print(materialNames[currentMaterialIndex]);
  lcd.print(" <<");
  
  lcd.setCursor(0, 3);
  lcd.print("Gire p/ mudar | OK");
}

// ═══════════════════════════════════════════════════════════════
//                       FUNÇÕES DO ENCODER
// ═══════════════════════════════════════════════════════════════

void initEncoder() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_SW), encoderButtonISR, FALLING);
  
  Serial.println("✅ Encoder configurado (GPIO 5,6,7)\n");
}

void IRAM_ATTR encoderISR() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  
  if (interruptTime - lastInterruptTime > 5) {
    if (digitalRead(ENCODER_DT) == LOW) {
      encoderPos++;
    } else {
      encoderPos--;
    }
  }
  lastInterruptTime = interruptTime;
}

void IRAM_ATTR encoderButtonISR() {
  static unsigned long lastButtonTime = 0;
  unsigned long buttonTime = millis();
  
  if (buttonTime - lastButtonTime > 200) {
    encoderButtonPressed = true;
  }
  lastButtonTime = buttonTime;
}

// ═══════════════════════════════════════════════════════════════
//                       FUNÇÕES DO DHT22
// ═══════════════════════════════════════════════════════════════

void initDHT() {
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();
  Serial.println("✅ DHT22 configurado (GPIO 17)\n");
}

// ═══════════════════════════════════════════════════════════════
//                       FUNÇÕES DO SD CARD
// ═══════════════════════════════════════════════════════════════

void initSDCard() {
  Serial.println("   Configurando SDMMC 1-bit...");
  Serial.println("   Pinos: CLK=39, CMD=38, D0=40");
  
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_DEFAULT;
  
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
  
  if (ret == ESP_OK) {
    sdCardAvailable = true;
    Serial.println("✅ SD Card OK!");
    
    uint64_t cardSize = ((uint64_t) sdCard->csd.capacity) * sdCard->csd.sector_size;
    Serial.printf("   Nome: %s\n", sdCard->cid.name);
    Serial.printf("   Capacidade: %.2f GB\n", (float)cardSize / (1024 * 1024 * 1024));
  } else {
    Serial.printf("❌ Falha ao montar SD Card: %s\n", esp_err_to_name(ret));
    sdCardAvailable = false;
  }
  Serial.println();
}

void createDirectories() {
  mkdir("/sdcard/logs", 0777);
  mkdir("/sdcard/jobs", 0777);
  mkdir("/sdcard/web", 0777);
}

// ═══════════════════════════════════════════════════════════════
//                      LEITURA DOS SENSORES
// ═══════════════════════════════════════════════════════════════

void readSensors() {
  // DHT22 - Temperatura e Umidade
  currentData.temperature = dht.readTemperature();
  currentData.humidity = dht.readHumidity();
  
  if (isnan(currentData.temperature)) currentData.temperature = 0.0;
  if (isnan(currentData.humidity)) currentData.humidity = 0.0;
  
  // LDR - Luminosidade (leitura direta, sem inversão)
  currentData.light = analogRead(LDR_PIN);
  
  // MAX4466 - Som (peak-to-peak + conversão para dB)
  currentData.soundADC = readSoundPeakToPeak();
  currentData.soundDB = convertToDBSPL(currentData.soundADC);
  
  currentData.timestamp = millis();
}

// ═══════════════════════════════════════════════════════════════
//                         FUNÇÕES DE LOG
// ═══════════════════════════════════════════════════════════════

void logToSD() {
  if (!sdCardAvailable) {
    Serial.println("[LOG] SD indisponível");
    return;
  }
  
  String filename = getLogFilename();
  FILE* file = fopen(filename.c_str(), "a");
  
  if (file) {
    // Formato CSV: timestamp,temp,hum,light,soundADC,soundDB,status
    String logLine = String(getFormattedTime()) + "," +
                     String(currentData.temperature, 1) + "," +
                     String(currentData.humidity, 1) + "," +
                     String(currentData.light) + "," +
                     String(currentData.soundADC) + "," +
                     String(currentData.soundDB, 1) + "," +
                     getStatus();
    
    fprintf(file, "%s\n", logLine.c_str());
    fclose(file);
    Serial.println("[LOG] ✓ " + logLine);
  }
}

String getLogFilename() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  char filename[40];
  strftime(filename, sizeof(filename), "/sdcard/logs/%Y%m%d.csv", &timeinfo);
  return String(filename);
}

String getFormattedTime() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// ═══════════════════════════════════════════════════════════════
//                    ANÁLISE DE STATUS (COM HISTERESE)
// ═══════════════════════════════════════════════════════════════

String getStatus() {
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  
  int outOfRange = 0;
  
  // Margens de tolerância (histerese) para evitar oscilação
  const float TEMP_TOLERANCE = 2.0;      // ±2°C
  const float HUM_TOLERANCE = 5.0;       // ±5%
  const int LIGHT_TOLERANCE = 200;       // ±200 lux
  const float SOUND_TOLERANCE_DB = 5.0;  // ±5 dB
  
  // Temperatura - com tolerância
  if (currentData.temperature < (mat.tempMin - TEMP_TOLERANCE) || 
      currentData.temperature > (mat.tempMax + TEMP_TOLERANCE)) {
    outOfRange++;
  }
  
  // Umidade - com tolerância
  if (currentData.humidity < (mat.humMin - HUM_TOLERANCE) || 
      currentData.humidity > (mat.humMax + HUM_TOLERANCE)) {
    outOfRange++;
  }
  
  // Luz - com tolerância
  if (currentData.light > (mat.lightMax + LIGHT_TOLERANCE)) {
    outOfRange++;
  }
  
  // Som - Usar dB com tolerância
  if (currentData.soundDB > (mat.soundMaxDB + SOUND_TOLERANCE_DB)) {
    outOfRange++;
  }
  
  // Determinar status baseado em quantas condições estão fora
  if (outOfRange == 0) {
    return "IDEAL";
  } else if (outOfRange == 1) {
    return "BOM";
  } else {
    return "RUIM";
  }
}

String getStatusDetails() {
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  String details = "";
  
  // Mesmas tolerâncias do getStatus()
  const float TEMP_TOLERANCE = 2.0;
  const float HUM_TOLERANCE = 5.0;
  const int LIGHT_TOLERANCE = 200;
  const float SOUND_TOLERANCE_DB = 5.0;
  
  if (currentData.temperature < (mat.tempMin - TEMP_TOLERANCE)) {
    details += "Temperatura muito baixa. ";
  } else if (currentData.temperature > (mat.tempMax + TEMP_TOLERANCE)) {
    details += "Temperatura muito alta. ";
  }
  
  if (currentData.humidity < (mat.humMin - HUM_TOLERANCE)) {
    details += "Umidade muito baixa. ";
  } else if (currentData.humidity > (mat.humMax + HUM_TOLERANCE)) {
    details += "Umidade muito alta. ";
  }
  
  if (currentData.light > (mat.lightMax + LIGHT_TOLERANCE)) {
    details += "Muita luz. ";
  }
  
  if (currentData.soundDB > (mat.soundMaxDB + SOUND_TOLERANCE_DB)) {
    details += "Muito ruído. ";
  }
  
  if (details == "") {
    details = "Condições ideais para " + currentMaterial + "!";
  }
  
  return details;
}

MaterialThresholds getMaterialThresholds(String material) {
  for (int i = 0; i < 4; i++) {
    if (material == materials[i].name) {
      return materials[i];
    }
  }
  return materials[0];  // Default: PLA
}

// ═══════════════════════════════════════════════════════════════
//                         WEB SERVER
// ═══════════════════════════════════════════════════════════════

void setupWebServer() {
  server.onNotFound([]() {

  String path = "/sdcard/web" + server.uri();

  FILE* file = fopen(path.c_str(), "r");

  if (file) {

    String contentType = "text/plain";

    if (server.uri().endsWith(".html")) contentType = "text/html";
    else if (server.uri().endsWith(".css")) contentType = "text/css";
    else if (server.uri().endsWith(".js")) contentType = "application/javascript";
    else if (server.uri().endsWith(".ico")) contentType = "image/x-icon";

    String content = "";
    char buffer[512];

    while (fgets(buffer, sizeof(buffer), file)) {
      content += buffer;
    }

    fclose(file);
    server.send(200, contentType, content);
    return;
  }

  server.send(404, "Arquivo não encontrado");
});

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
        Serial.println("[WEB] Servindo index.html do SD Card");
        return;
      }
    }
    server.send(200, "text/html", "<html><body><h1>PrintSense v1.0</h1><p>Interface web em desenvolvimento. Coloque os arquivos em /sdcard/web/</p></body></html>");
    Serial.println("[WEB] Servindo index.html padrão");
  });
  
  // API - Dados dos sensores
  server.on("/api/data", HTTP_GET, []() {
    StaticJsonDocument<1024> doc;
    
    doc["temperature"] = currentData.temperature;
    doc["humidity"] = currentData.humidity;
    doc["light"] = currentData.light;
    doc["soundADC"] = currentData.soundADC;      // Valor bruto ADC
    doc["soundDB"] = currentData.soundDB;        // Valor em dB SPL
    doc["timestamp"] = getFormattedTime();
    doc["material"] = currentMaterial;
    doc["status"] = getStatus();
    doc["ledStatus"] = getLEDStatus();
    doc["statusDetails"] = getStatusDetails();
    
    MaterialThresholds mat = getMaterialThresholds(currentMaterial);
    doc["thresholds"]["tempMin"] = mat.tempMin;
    doc["thresholds"]["tempMax"] = mat.tempMax;
    doc["thresholds"]["humMin"] = mat.humMin;
    doc["thresholds"]["humMax"] = mat.humMax;
    doc["thresholds"]["lightMax"] = mat.lightMax;
    doc["thresholds"]["soundMaxDB"] = mat.soundMaxDB;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  // API - Mudar material
  server.on("/api/material", HTTP_POST, []() {

    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"success\":false,\"error\":\"Empty body\"}");
        return;
    }

    String body = server.arg("plain");

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
        return;
    }

    if (!doc.containsKey("material")) {
        server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing material\"}");
        return;
    }

    currentMaterial = doc["material"].as<String>();

    // Atualizar índice do encoder
    for (int i = 0; i < 4; i++) {
        if (currentMaterial == materialNames[i]) {
            currentMaterialIndex = i;
            break;
        }
    }

    Serial.print("[API] Material alterado (JSON): ");
    Serial.println(currentMaterial);

    updateLEDs();

    StaticJsonDocument<256> responseDoc;
    responseDoc["success"] = true;
    responseDoc["newMaterial"] = currentMaterial;
    responseDoc["status"] = getStatus();
    responseDoc["ledStatus"] = getLEDStatus();

    String response;
    serializeJson(responseDoc, response);

    server.send(200, "application/json", response);
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
          if (entry->d_type == DT_REG) {
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
  
  // API - Download de log
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
          
          Serial.print("[API] Download log: ");
          Serial.println(server.arg("file"));
          
          server.send(200, "text/csv", content);
        } else {
          server.send(404, "text/plain", "Arquivo não encontrado");
        }
      } else {
        server.send(503, "text/plain", "SD Card indisponível");
      }
    } else {
      server.send(400, "text/plain", "Parâmetro 'file' não especificado");
    }
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
        Serial.println("[WEB] Servindo script.js do SD Card");
        return;
      }
    }
    server.send(404, "text/plain", "script.js não encontrado");
  });
}

// ═══════════════════════════════════════════════════════════════
//                          FIM DO CÓDIGO
// ═══════════════════════════════════════════════════════════════
