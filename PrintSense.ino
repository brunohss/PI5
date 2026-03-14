/*
 * ═══════════════════════════════════════════════════════════════
 *                    PRINTSENSE v4.2 FreeRTOS
 * ═══════════════════════════════════════════════════════════════
 * 
 * Monitor Ambiental Profissional para Impressão 3D
 * ESP32-S3 N16R8 (2 Cores) com Arquitetura FreeRTOS
 * 
 * ARQUITETURA DUAL-CORE:
 * ┌─────────────────────────────────────────────────────────────┐
 * │ CORE 0 (Protocol CPU):                                      │
 * │  ✅ Task WebServer (prioridade 2) - 4KB stack               │
 * │  ✅ Task Logging (prioridade 1) - 3KB stack                 │
 * │                                                             │
 * │ CORE 1 (Application CPU):                                   │
 * │  ✅ Task Sensores (prioridade 3) - 4KB stack - CRÍTICA      │
 * │  ✅ Task LCD (prioridade 2) - 3KB stack                     │
 * │  ✅ Task LEDs (prioridade 2) - 2KB stack                    │
 * │  ✅ Task Encoder (prioridade 3) - 2KB stack - CRÍTICA       │
 * │                                                             │
 * │ TOTAL: 6 Tasks paralelas + Sincronização otimizada          │
 * └─────────────────────────────────────────────────────────────┘
 * 
 * VANTAGENS FREERTOS:
 *  ✅ Encoder NUNCA trava (task dedicada 10ms)
 *  ✅ LCD não afeta sensores (tasks separadas)
 *  ✅ WebServer em core separado (zero interferência)
 *  ✅ Logging assíncrono (não bloqueia nada)
 *  ✅ Uso inteligente de 2 cores
 *  ✅ Proteção de recursos compartilhados (mutexes)
 *  ✅ Comunicação eficiente (queues)
 *  ✅ Sistema nunca trava!
 * 
 * SOLUÇÃO DO PROBLEMA DO ENCODER:
 *  ❌ ANTES: Encoder no loop() principal
 *     → Compartilhava tempo com LCD, sensores, web
 *     → Atualizações lentas
 *     → Travamentos ao atualizar LCD
 * 
 *  ✅ AGORA: Encoder em task dedicada (CORE 1)
 *     → Roda a cada 10ms (100 Hz!)
 *     → Nunca é interrompido
 *     → LCD em task separada
 *     → Resposta INSTANTÂNEA!
 * 
 * AUTOR: Equipe PrintSense
 * DATA: 2026
 * VERSÃO: 4.2 FreeRTOS FINAL
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

// ==================== CONFIG WiFi ====================
const char* ssid     = "ssid";
const char* password = "password";

// ==================== PINOS ====================
#define DHTPIN 17
#define DHT_TYPE DHT22
#define LDR_PIN 4
#define MAX4466_PIN 8
#define ENCODER_CLK 7
#define ENCODER_DT 6
#define ENCODER_SW 5
#define LED_GREEN 2
#define LED_YELLOW 15
#define LED_RED 16
#define SD_CLK_PIN GPIO_NUM_39
#define SD_CMD_PIN GPIO_NUM_38
#define SD_D0_PIN  GPIO_NUM_40
#define I2C_SDA 10
#define I2C_SCL 13

// ==================== OBJETOS ====================
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHTPIN, DHT_TYPE);
WebServer server(80);
bool sdCardAvailable = false;
sdmmc_card_t* sdCard = nullptr;

// ==================== NTP ====================
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

// ==================== ESTRUTURAS ====================
struct SensorData {
  float temperature;
  float humidity;
  uint16_t light;
  int soundADC;
  float soundDB;
  unsigned long timestamp;
  String status;
  String ledStatus;
};

struct MaterialThresholds {
  const char* name;
  float tempMin, tempMax;
  float humMin, humMax;
  int lightMax;
  int soundMaxADC;
  float soundMaxDB;
};

// ==================== DADOS ====================
SensorData currentData;
String currentMaterial = "PLA";
int currentMaterialIndex = 0;

const char* materialNames[] = {"PLA", "PETG", "ABS", "RESINA"};
const int numMaterials = 4;

MaterialThresholds materials[] = {
  {"PLA",    18.0, 28.0, 40.0, 60.0, 3000, 800, 70.0},
  {"PETG",   20.0, 30.0, 30.0, 50.0, 3000, 800, 70.0},
  {"ABS",    22.0, 32.0, 20.0, 40.0, 3000, 800, 70.0},
  {"RESINA", 20.0, 25.0, 40.0, 60.0, 1000, 600, 65.0}
};

// ==================== FREERTOS ====================
TaskHandle_t taskSensorsHandle = NULL;
TaskHandle_t taskLCDHandle = NULL;
TaskHandle_t taskLEDsHandle = NULL;
TaskHandle_t taskEncoderHandle = NULL;
TaskHandle_t taskWebServerHandle = NULL;
TaskHandle_t taskLoggingHandle = NULL;

QueueHandle_t sensorDataQueue = NULL;
QueueHandle_t lcdUpdateQueue = NULL;
QueueHandle_t logQueue = NULL;

SemaphoreHandle_t i2cMutex = NULL;
SemaphoreHandle_t dataMutex = NULL;
SemaphoreHandle_t sdMutex = NULL;

EventGroupHandle_t systemEvents = NULL;
#define SENSOR_READY_BIT (1 << 0)
#define MATERIAL_CHANGED_BIT (1 << 4)

// ==================== ENCODER ====================
enum MenuState { MENU_CLOSED, MENU_OPEN };
volatile MenuState menuState = MENU_CLOSED;
volatile int selectedMaterialIndex = 0;
volatile int lastCLK = HIGH;
volatile int lastButtonState = HIGH;
volatile unsigned long lastButtonPress = 0;
volatile unsigned long lastEncoderTurn = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// ═══════════════════════════════════════════════════════════════
//                    TASK: SENSORES (CORE 1)
// ═══════════════════════════════════════════════════════════════
void taskSensors(void *parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(1000);
  
  Serial.println("[TASK] Sensores iniciada (CORE 1)");
  
  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    
    SensorData newData;
    newData.temperature = dht.readTemperature();
    newData.humidity = dht.readHumidity();
    if (isnan(newData.temperature)) newData.temperature = 0.0;
    if (isnan(newData.humidity)) newData.humidity = 0.0;
    
    newData.light = analogRead(LDR_PIN);
    newData.soundADC = readSoundPeakToPeak();
    newData.soundDB = convertToDBSPL(newData.soundADC);
    newData.timestamp = millis();
    newData.status = calculateStatus(newData);
    newData.ledStatus = statusToLED(newData.status);
    
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      currentData = newData;
      xSemaphoreGive(dataMutex);
      xEventGroupSetBits(systemEvents, SENSOR_READY_BIT);
      xQueueSend(logQueue, &newData, 0);
      xQueueSend(lcdUpdateQueue, &newData, 0);
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//                    TASK: LCD (CORE 1)
// ═══════════════════════════════════════════════════════════════
void taskLCD(void *parameter) {
  SensorData data;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(500);
  
  Serial.println("[TASK] LCD iniciada (CORE 1)");
  
  for(;;) {
    if (xQueueReceive(lcdUpdateQueue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (menuState == MENU_CLOSED) {
          updateLCDNormal(data);
        } else {
          updateLCDMenu();
        }
        xSemaphoreGive(i2cMutex);
      }
    }
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// ═══════════════════════════════════════════════════════════════
//                    TASK: LEDs (CORE 1)
// ═══════════════════════════════════════════════════════════════
void taskLEDs(void *parameter) {
  String lastStatus = "";
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(100);
  
  Serial.println("[TASK] LEDs iniciada (CORE 1)");
  
  for(;;) {
    String currentStatus;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      currentStatus = currentData.status;
      xSemaphoreGive(dataMutex);
    }
    
    if (currentStatus != lastStatus) {
      updateLEDsHardware(currentStatus);
      lastStatus = currentStatus;
    }
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// ═══════════════════════════════════════════════════════════════
//              TASK: ENCODER (CORE 1 - PRIORIDADE ALTA!)
// ═══════════════════════════════════════════════════════════════
void taskEncoder(void *parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 10ms = 100 Hz !!!
  
  Serial.println("[TASK] Encoder iniciada (CORE 1 - PRIORIDADE ALTA)");
  
  for(;;) {
    handleEncoderStateMachine();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// ═══════════════════════════════════════════════════════════════
//                    TASK: WEB SERVER (CORE 0)
// ═══════════════════════════════════════════════════════════════
void taskWebServer(void *parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(1);
  
  Serial.println("[TASK] WebServer iniciada (CORE 0)");
  
  for(;;) {
    server.handleClient();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// ═══════════════════════════════════════════════════════════════
//                    TASK: LOGGING (CORE 0)
// ═══════════════════════════════════════════════════════════════
void taskLogging(void *parameter) {
  SensorData data;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(60000);
  
  Serial.println("[TASK] Logging iniciada (CORE 0)");
  
  for(;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    if (xQueuePeek(logQueue, &data, 0) == pdTRUE) {
      if (sdCardAvailable) {
        if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
          writeLogToSD(data);
          xSemaphoreGive(sdMutex);
        }
      }
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//                    FUNÇÕES DE LEITURA
// ═══════════════════════════════════════════════════════════════

int readSoundPeakToPeak() {
  int signalMax = 0, signalMin = 4095;
  unsigned long startTime = micros();
  while (micros() - startTime < 10000) {
    int sample = analogRead(MAX4466_PIN);
    if (sample > signalMax) signalMax = sample;
    if (sample < signalMin) signalMin = sample;
    delayMicroseconds(500);
  }
  return signalMax - signalMin;
}

float convertToDBSPL(int adc) {
  const int pts[][2] = {{50,30},{150,40},{300,50},{600,60},{1000,70},{1500,80},{2500,90}};
  for(int i=0; i<6; i++) {
    if(adc >= pts[i][0] && adc <= pts[i+1][0]) {
      return pts[i][1] + ((adc-pts[i][0])*(pts[i+1][1]-pts[i][1]))/(pts[i+1][0]-pts[i][0]);
    }
  }
  return adc < 50 ? 20.0 : (adc > 2500 ? 100.0 : 60.0);
}

// ═══════════════════════════════════════════════════════════════
//                    FUNÇÕES DO ENCODER
// ═══════════════════════════════════════════════════════════════

bool readEncoderButton() {
  int buttonState = digitalRead(ENCODER_SW);
  unsigned long now = millis();
  if (now - lastButtonPress < DEBOUNCE_DELAY) return false;
  if (buttonState == LOW && lastButtonState == HIGH) {
    lastButtonPress = now;
    lastButtonState = buttonState;
    return true;
  }
  lastButtonState = buttonState;
  return false;
}

int readEncoderRotation() {
  int currentCLK = digitalRead(ENCODER_CLK);
  unsigned long now = millis();
  if (now - lastEncoderTurn < 50) return 0;
  if (currentCLK != lastCLK) {
    lastEncoderTurn = now;
    if (currentCLK == LOW) {
      int dt = digitalRead(ENCODER_DT);
      lastCLK = currentCLK;
      return (dt == HIGH) ? 1 : -1;
    }
  }
  lastCLK = currentCLK;
  return 0;
}

void handleEncoderStateMachine() {
  bool buttonPressed = readEncoderButton();
  int rotation = readEncoderRotation();
  
  switch (menuState) {
    case MENU_CLOSED:
      if (buttonPressed) {
        menuState = MENU_OPEN;
        selectedMaterialIndex = currentMaterialIndex;
        Serial.println("[ENCODER] Menu ABERTO");
        SensorData dummy;
        xQueueSend(lcdUpdateQueue, &dummy, 0);
      }
      break;
    
    case MENU_OPEN:
      if (rotation != 0) {
        selectedMaterialIndex += rotation;
        if (selectedMaterialIndex < 0) selectedMaterialIndex = numMaterials - 1;
        if (selectedMaterialIndex >= numMaterials) selectedMaterialIndex = 0;
        Serial.printf("[ENCODER] → %s\n", materialNames[selectedMaterialIndex]);
        SensorData dummy;
        xQueueSend(lcdUpdateQueue, &dummy, 0);
      }
      if (buttonPressed) {
        currentMaterial = String(materialNames[selectedMaterialIndex]);
        currentMaterialIndex = selectedMaterialIndex;
        menuState = MENU_CLOSED;
        Serial.printf("[ENCODER] Material confirmado: %s\n", currentMaterial.c_str());
        xEventGroupSetBits(systemEvents, MATERIAL_CHANGED_BIT);
        SensorData dummy;
        xQueueSend(lcdUpdateQueue, &dummy, 0);
      }
      break;
  }
}

// ═══════════════════════════════════════════════════════════════
//                    FUNÇÕES DO LCD
// ═══════════════════════════════════════════════════════════════

void updateLCDNormal(SensorData data) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(currentMaterial);
  lcd.setCursor(5,0); 
  if(data.status=="IDEAL") lcd.print("* IDEAL");
  else if(data.status=="BOM") lcd.print("! BOM");
  else lcd.print("X RUIM");
  
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  lcd.setCursor(0,1); lcd.print("T:"); lcd.print(data.temperature,1); lcd.print((char)223); lcd.print("C");
  lcd.setCursor(11,1); lcd.print(mat.tempMin,0); lcd.print("-"); lcd.print(mat.tempMax,0); lcd.print((char)223); lcd.print("C");
  lcd.setCursor(0,2); lcd.print("H:"); lcd.print(data.humidity,1); lcd.print("%");
  lcd.setCursor(11,2); lcd.print(mat.humMin,0); lcd.print("-"); lcd.print(mat.humMax,0); lcd.print("%");
  lcd.setCursor(0,3); lcd.print("Luz:"); lcd.print(data.light);
  lcd.setCursor(10,3); lcd.print("Som:"); lcd.print(data.soundDB,0); lcd.print("dB");
}

void updateLCDMenu() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("SELECIONAR MATERIAL:");
  int prevIdx = (selectedMaterialIndex-1+numMaterials)%numMaterials;
  lcd.setCursor(0,1); lcd.print("  "); lcd.print(materialNames[prevIdx]);
  lcd.setCursor(0,2); lcd.print("> "); lcd.print(materialNames[selectedMaterialIndex]); lcd.print(" <");
  int nextIdx = (selectedMaterialIndex+1)%numMaterials;
  lcd.setCursor(0,3); lcd.print("  "); lcd.print(materialNames[nextIdx]);
}

// ═══════════════════════════════════════════════════════════════
//                    FUNÇÕES DOS LEDs
// ═══════════════════════════════════════════════════════════════

void updateLEDsHardware(String status) {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  delayMicroseconds(10);
  if(status=="IDEAL") digitalWrite(LED_GREEN, HIGH);
  else if(status=="BOM") digitalWrite(LED_YELLOW, HIGH);
  else digitalWrite(LED_RED, HIGH);
}

String statusToLED(String status) {
  if(status=="IDEAL") return "GREEN";
  if(status=="BOM") return "YELLOW";
  return "RED";
}

// ═══════════════════════════════════════════════════════════════
//                    CÁLCULO DE STATUS
// ═══════════════════════════════════════════════════════════════

String calculateStatus(SensorData data) {
  MaterialThresholds mat = getMaterialThresholds(currentMaterial);
  int outOfRange = 0;
  if(data.temperature < (mat.tempMin-2.0) || data.temperature > (mat.tempMax+2.0)) outOfRange++;
  if(data.humidity < (mat.humMin-5.0) || data.humidity > (mat.humMax+5.0)) outOfRange++;
  if(data.light > (mat.lightMax+200)) outOfRange++;
  if(data.soundDB > (mat.soundMaxDB+5.0)) outOfRange++;
  return (outOfRange==0) ? "IDEAL" : ((outOfRange==1) ? "BOM" : "RUIM");
}

MaterialThresholds getMaterialThresholds(String material) {
  for(int i=0; i<numMaterials; i++) {
    if(material == materials[i].name) return materials[i];
  }
  return materials[0];
}

// ═══════════════════════════════════════════════════════════════
//                    LOGGING
// ═══════════════════════════════════════════════════════════════

void writeLogToSD(SensorData data) {
  if(!sdCardAvailable) return;
  String filename = getLogFilename();
  FILE* file = fopen(filename.c_str(), "a");
  if(file) {
    String logLine = String(getFormattedTime()) + "," + String(data.temperature,1) + "," +
                     String(data.humidity,1) + "," + String(data.light) + "," +
                     String(data.soundADC) + "," + String(data.soundDB,1) + "," + data.status;
    fprintf(file, "%s\n", logLine.c_str());
    fclose(file);
    Serial.println("[LOG] ✓ " + logLine);
  }
}

String getLogFilename() {
  time_t now; struct tm timeinfo;
  time(&now); localtime_r(&now, &timeinfo);
  char filename[40];
  strftime(filename, sizeof(filename), "/sdcard/logs/%Y%m%d.csv", &timeinfo);
  return String(filename);
}

String getFormattedTime() {
  time_t now; struct tm timeinfo;
  time(&now); localtime_r(&now, &timeinfo);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// ═══════════════════════════════════════════════════════════════
//                    INICIALIZAÇÕES
// ═══════════════════════════════════════════════════════════════

void initSDCard() {
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.clk = SD_CLK_PIN; slot_config.cmd = SD_CMD_PIN; slot_config.d0 = SD_D0_PIN; slot_config.width = 1;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed=false, .max_files=5};
  if(esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &sdCard)==ESP_OK) {
    sdCardAvailable = true;
    mkdir("/sdcard/logs", 0777); mkdir("/sdcard/web", 0777);
    Serial.println("✅ SD Card OK!");
    uint64_t cardSize = ((uint64_t) sdCard->csd.capacity) * sdCard->csd.sector_size;
    Serial.printf("   Capacidade: %.2f GB\n", (float)cardSize / (1024 * 1024 * 1024));
  } else {
    Serial.println("❌ SD Card falhou");
  }
}

void setupWebServer() {
  // Rota principal - index.html
  server.on("/", HTTP_GET, []() {
    if(sdCardAvailable) {
      FILE* file = fopen("/sdcard/web/index.html", "r");
      if(file) {
        String content=""; char buffer[512];
        while(fgets(buffer, sizeof(buffer), file)) content += buffer;
        fclose(file);
        server.send(200, "text/html", content);
        Serial.println("[WEB] Servindo index.html");
        return;
      }
    }
    server.send(200, "text/html", "<h1>PrintSense v4.0 FreeRTOS</h1><p>Coloque arquivos em /sdcard/web/</p>");
  });
  
  // API - Dados dos sensores
  server.on("/api/data", HTTP_GET, []() {
    StaticJsonDocument<1024> doc;
    if(xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100))==pdTRUE) {
      doc["temperature"]=currentData.temperature; 
      doc["humidity"]=currentData.humidity;
      doc["light"]=currentData.light; 
      doc["soundADC"]=currentData.soundADC;
      doc["soundDB"]=currentData.soundDB; 
      doc["timestamp"]=getFormattedTime();
      doc["status"]=currentData.status;
      doc["ledStatus"]=currentData.ledStatus; 
      doc["material"]=currentMaterial;
      doc["statusDetails"]="Condições " + String(currentData.status) + " para " + currentMaterial;
      
      MaterialThresholds mat = getMaterialThresholds(currentMaterial);
      doc["thresholds"]["tempMin"]=mat.tempMin; 
      doc["thresholds"]["tempMax"]=mat.tempMax;
      doc["thresholds"]["humMin"]=mat.humMin; 
      doc["thresholds"]["humMax"]=mat.humMax;
      doc["thresholds"]["lightMax"]=mat.lightMax;
      doc["thresholds"]["soundMaxDB"]=mat.soundMaxDB;
      
      xSemaphoreGive(dataMutex);
    }
    String response; 
    serializeJson(doc, response);
    server.send(200, "application/json", response);
    Serial.println("[API] /api/data respondido");
  });
  
  // API - Mudar material
  server.on("/api/material", HTTP_POST, []() {
    if(server.hasArg("plain")) {
      StaticJsonDocument<256> doc;
      if(deserializeJson(doc, server.arg("plain"))==DeserializationError::Ok && doc.containsKey("material")) {
        currentMaterial = doc["material"].as<String>();
        for(int i=0; i<numMaterials; i++) {
          if(currentMaterial == materialNames[i]) { 
            currentMaterialIndex=i; 
            break; 
          }
        }
        Serial.printf("[API] Material alterado: %s\n", currentMaterial.c_str());
        
        StaticJsonDocument<256> responseDoc;
        responseDoc["success"] = true;
        responseDoc["newMaterial"] = currentMaterial;
        responseDoc["status"] = currentData.status;
        responseDoc["ledStatus"] = currentData.ledStatus;
        
        String response;
        serializeJson(responseDoc, response);
        server.send(200, "application/json", response);
        return;
      }
    }
    server.send(400, "application/json", "{\"success\":false}");
  });
  
  // API - Listar logs
  server.on("/api/logs", HTTP_GET, []() {
    StaticJsonDocument<2048> doc;
    JsonArray logs = doc.createNestedArray("logs");
    
    if(sdCardAvailable) {
      DIR* dir = opendir("/sdcard/logs");
      if(dir) {
        struct dirent* entry;
        while((entry = readdir(dir)) != NULL) {
          if(entry->d_type == DT_REG) {
            String filename = String(entry->d_name);
            if(filename.endsWith(".csv")) {
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
    Serial.printf("[API] /api/logs respondido (%d arquivos)\n", logs.size());
  });
  
  // API - Download de log específico
  server.on("/api/log", HTTP_GET, []() {
    if(server.hasArg("file")) {
      String filename = "/sdcard/logs/" + server.arg("file");
      
      if(sdCardAvailable) {
        FILE* file = fopen(filename.c_str(), "r");
        if(file) {
          String content = "";
          char buffer[256];
          while(fgets(buffer, sizeof(buffer), file)) {
            content += buffer;
          }
          fclose(file);
          
          Serial.printf("[API] Download log: %s\n", server.arg("file").c_str());
          server.send(200, "text/csv", content);
          return;
        } else {
          server.send(404, "text/plain", "Arquivo não encontrado");
          return;
        }
      } else {
        server.send(503, "text/plain", "SD Card indisponível");
        return;
      }
    } else {
      server.send(400, "text/plain", "Parâmetro 'file' não especificado");
    }
  });
  
  // Servir script.js
  server.on("/script.js", HTTP_GET, []() {
    if(sdCardAvailable) {
      FILE* file = fopen("/sdcard/web/script.js", "r");
      if(file) {
        String content = "";
        char buffer[512];
        while(fgets(buffer, sizeof(buffer), file)) {
          content += buffer;
        }
        fclose(file);
        server.send(200, "application/javascript", content);
        Serial.println("[WEB] Servindo script.js");
        return;
      }
    }
    server.send(404, "text/plain", "script.js não encontrado");
  });
  
  // Servir style.css
  server.on("/style.css", HTTP_GET, []() {
    if(sdCardAvailable) {
      FILE* file = fopen("/sdcard/web/style.css", "r");
      if(file) {
        String content = "";
        char buffer[512];
        while(fgets(buffer, sizeof(buffer), file)) {
          content += buffer;
        }
        fclose(file);
        server.send(200, "text/css", content);
        Serial.println("[WEB] Servindo style.css");
        return;
      }
    }
    server.send(404, "text/plain", "style.css não encontrado");
  });
  
  // Handler para arquivos não encontrados (fallback para outros arquivos do SD)
  server.onNotFound([]() {
    String path = "/sdcard/web" + server.uri();
    
    if(sdCardAvailable) {
      FILE* file = fopen(path.c_str(), "r");
      if(file) {
        String contentType = "text/plain";
        if(server.uri().endsWith(".html")) contentType = "text/html";
        else if(server.uri().endsWith(".css")) contentType = "text/css";
        else if(server.uri().endsWith(".js")) contentType = "application/javascript";
        else if(server.uri().endsWith(".ico")) contentType = "image/x-icon";
        else if(server.uri().endsWith(".png")) contentType = "image/png";
        else if(server.uri().endsWith(".jpg")) contentType = "image/jpeg";
        
        String content = "";
        char buffer[512];
        while(fgets(buffer, sizeof(buffer), file)) {
          content += buffer;
        }
        fclose(file);
        
        server.send(200, contentType, content);
        Serial.printf("[WEB] Servindo %s\n", server.uri().c_str());
        return;
      }
    }
    
    server.send(404, "text/plain", "Arquivo não encontrado: " + server.uri());
    Serial.printf("[WEB] 404: %s\n", server.uri().c_str());
  });
  
  server.begin();
  Serial.println("✅ WebServer iniciado com todas as rotas!");
}

// ═══════════════════════════════════════════════════════════════
//                           SETUP
// ═══════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("╔═══════════════════════════════════════════════════════╗");
  Serial.println("║                                                       ║");
  Serial.println("║         PRINTSENSE v4.0 FreeRTOS DUAL-CORE            ║");
  Serial.println("║      Sistema de Monitoramento para Impressão 3D      ║");
  Serial.println("║                                                       ║");
  Serial.println("╚═══════════════════════════════════════════════════════╝");
  Serial.println();
  
  // [0/7] LEDs
  Serial.println("[0/7] Configurando LEDs...");
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  
  // Testar LEDs
  Serial.print("   Testando LEDs: ");
  digitalWrite(LED_GREEN, HIGH);
  Serial.print("🟢");
  delay(300);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  Serial.print("→🟡");
  delay(300);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  Serial.print("→🔴");
  delay(300);
  digitalWrite(LED_RED, LOW);
  Serial.println(" OK!");
  
  Serial.println("✅ LEDs configurados");
  Serial.println("   Verde (IDEAL): GPIO 2 + 220Ω");
  Serial.println("   Amarelo (BOM): GPIO 15 + 220Ω");
  Serial.println("   Vermelho (RUIM): GPIO 16 + 220Ω\n");
  
  // [1/7] LCD
  Serial.println("[1/7] Inicializando LCD...");
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("PrintSense v4.0");
  lcd.setCursor(0, 1);
  lcd.print("FreeRTOS Dual-Core");
  lcd.setCursor(0, 2);
  lcd.print("Inicializando...");
  Serial.println("✅ LCD 20x4 inicializado (I2C: SDA=10, SCL=13)\n");
  
  // [2/7] Encoder
  Serial.println("[2/7] Configurando Encoder...");
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  lcd.setCursor(0, 3);
  lcd.print("Encoder: OK");
  Serial.println("✅ Encoder configurado (CLK=7, DT=6, SW=5)\n");
  delay(1000);
  
  // [3/7] DHT22
  Serial.println("[3/7] Inicializando DHT22...");
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();
  delay(2000);
  
  float testTemp = dht.readTemperature();
  float testHum = dht.readHumidity();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Testando DHT22...");
  
  if (!isnan(testTemp) && !isnan(testHum)) {
    Serial.printf("✅ DHT22 OK! Temp: %.1f°C, Umid: %.1f%%\n\n", testTemp, testHum);
    lcd.setCursor(0, 1);
    lcd.print("DHT22: OK");
    lcd.setCursor(0, 2);
    lcd.print("T:");
    lcd.print(testTemp, 1);
    lcd.print("C H:");
    lcd.print(testHum, 0);
    lcd.print("%");
  } else {
    Serial.println("⚠️  DHT22: Aguarde estabilização\n");
    lcd.setCursor(0, 1);
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
    lcd.setCursor(0, 1);
    lcd.print("SD: OK");
    Serial.println("✅ Diretórios criados (/web, /logs)\n");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SD: Erro (opcional)");
  }
  delay(1500);
  
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
    Serial.print("   SSID: ");
    Serial.println(ssid);
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("   RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm\n");
    
    lcd.setCursor(0, 2);
    lcd.print("WiFi: OK");
    lcd.setCursor(0, 3);
    lcd.print(WiFi.localIP());
  } else {
    Serial.println("⚠️  WiFi falhou - iniciando modo AP");
    WiFi.softAP("PrintSense", "printsense123");
    Serial.println("   SSID AP: PrintSense");
    Serial.println("   Senha: printsense123");
    Serial.println("   IP AP: 192.168.4.1\n");
    
    lcd.setCursor(0, 2);
    lcd.print("WiFi: Modo AP");
    lcd.setCursor(0, 3);
    lcd.print("192.168.4.1");
  }
  delay(2000);
  
  // [6/7] NTP
  Serial.println("[6/7] Configurando NTP...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("✅ NTP configurado (pool.ntp.org, GMT-3)\n");
  
  // [7/7] WebServer & FreeRTOS
  Serial.println("[7/7] Iniciando WebServer e FreeRTOS...");
  setupWebServer();
  
  // Criar Mutexes
  i2cMutex = xSemaphoreCreateMutex();
  dataMutex = xSemaphoreCreateMutex();
  sdMutex = xSemaphoreCreateMutex();
  Serial.println("   ✓ Mutexes criados (i2c, data, sd)");
  
  // Criar Queues
  sensorDataQueue = xQueueCreate(5, sizeof(SensorData));
  lcdUpdateQueue = xQueueCreate(5, sizeof(SensorData));
  logQueue = xQueueCreate(10, sizeof(SensorData));
  Serial.println("   ✓ Queues criadas (sensor, lcd, log)");
  
  // Criar Event Groups
  systemEvents = xEventGroupCreate();
  Serial.println("   ✓ Event Groups criados");
  
  Serial.println("\n   Criando Tasks:");
  
  // CORE 1 - Application
  xTaskCreatePinnedToCore(taskSensors, "Sensors", 4096, NULL, 3, &taskSensorsHandle, 1);
  Serial.println("   ✓ Task Sensores (CORE 1, P3, 4KB)");
  
  xTaskCreatePinnedToCore(taskLCD, "LCD", 3072, NULL, 2, &taskLCDHandle, 1);
  Serial.println("   ✓ Task LCD (CORE 1, P2, 3KB)");
  
  xTaskCreatePinnedToCore(taskLEDs, "LEDs", 2048, NULL, 2, &taskLEDsHandle, 1);
  Serial.println("   ✓ Task LEDs (CORE 1, P2, 2KB)");
  
  xTaskCreatePinnedToCore(taskEncoder, "Encoder", 2048, NULL, 3, &taskEncoderHandle, 1);
  Serial.println("   ✓ Task Encoder (CORE 1, P3, 2KB) ★");
  
  // CORE 0 - Protocol
  xTaskCreatePinnedToCore(taskWebServer, "WebServer", 4096, NULL, 2, &taskWebServerHandle, 0);
  Serial.println("   ✓ Task WebServer (CORE 0, P2, 4KB)");
  
  xTaskCreatePinnedToCore(taskLogging, "Logging", 3072, NULL, 1, &taskLoggingHandle, 0);
  Serial.println("   ✓ Task Logging (CORE 0, P1, 3KB)");
  
  Serial.println("\n✅ TODAS AS TASKS CRIADAS!\n");
  
  Serial.println("╔═══════════════════════════════════════════════════════╗");
  Serial.println("║                                                       ║");
  Serial.println("║            SISTEMA FREERTOS INICIALIZADO!             ║");
  Serial.println("║                                                       ║");
  Serial.println("╠═══════════════════════════════════════════════════════╣");
  Serial.println("║                                                       ║");
  Serial.println("║  CORE 0 (Protocol CPU):                               ║");
  Serial.println("║    → Task WebServer (P2, 1ms cycle)                   ║");
  Serial.println("║    → Task Logging (P1, 60s cycle)                     ║");
  Serial.println("║                                                       ║");
  Serial.println("║  CORE 1 (Application CPU):                            ║");
  Serial.println("║    → Task Sensores (P3, 1000ms cycle) ★               ║");
  Serial.println("║    → Task LCD (P2, 500ms cycle)                       ║");
  Serial.println("║    → Task LEDs (P2, 100ms cycle)                      ║");
  Serial.println("║    → Task Encoder (P3, 10ms cycle) ★★                 ║");
  Serial.println("║                                                       ║");
  Serial.println("║  6 Tasks Paralelas | 2 Cores Ativos                   ║");
  Serial.println("║                                                       ║");
  Serial.println("╚═══════════════════════════════════════════════════════╝");
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("🌐 ACESSE A INTERFACE WEB:");
    Serial.print("   http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("🌐 MODO AP - ACESSE:");
    Serial.println("   http://192.168.4.1");
  }
  
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SISTEMA PRONTO!");
  lcd.setCursor(0, 1);
  lcd.print("FreeRTOS 6 Tasks");
  lcd.setCursor(0, 2);
  lcd.print("Material: ");
  lcd.print(currentMaterial);
  lcd.setCursor(0, 3);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print(WiFi.localIP());
  } else {
    lcd.print("AP: 192.168.4.1");
  }
  
  delay(3000);
}

// ═══════════════════════════════════════════════════════════════
//                           LOOP
// ═══════════════════════════════════════════════════════════════

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
  static unsigned long lastMemCheck = 0;
  if(millis() - lastMemCheck > 30000) {
    Serial.printf("[MEM] Free: %d bytes | PSRAM: %d bytes\n", ESP.getFreeHeap(), ESP.getFreePsram());
    lastMemCheck = millis();
  }
}
