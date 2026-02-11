/*
 * Teste SD Card - Descobrir pino CS correto
 * Este código testa vários pinos comuns de CS
 */

#include <SD.h>
#include <SPI.h>

// Pinos comuns de CS em ESP32-S3 com SD integrado
int csTestPins[] = {5, 10, 13, 15, 21, 22};
int numPins = 6;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== TESTE SD CARD ===");
  Serial.println("Procurando pino CS correto...\n");
  
  // Testar cada pino
  for (int i = 0; i < numPins; i++) {
    int testPin = csTestPins[i];
    
    Serial.print("Testando GPIO ");
    Serial.print(testPin);
    Serial.print("... ");
    
    if (SD.begin(testPin)) {
      Serial.println("✅ ENCONTRADO!");
      Serial.print("\n>>> Pino CS correto: GPIO ");
      Serial.println(testPin);
      
      // Informações do cartão
      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.print("Tamanho do cartão: ");
      Serial.print(cardSize);
      Serial.println(" MB");
      
      Serial.print("Tipo: ");
      switch (SD.cardType()) {
        case CARD_MMC:
          Serial.println("MMC");
          break;
        case CARD_SD:
          Serial.println("SDSC");
          break;
        case CARD_SDHC:
          Serial.println("SDHC");
          break;
        default:
          Serial.println("UNKNOWN");
      }
      
      // Listar arquivos
      Serial.println("\nArquivos no cartão:");
      listDir(SD, "/", 0);
      
      Serial.println("\n=== SUCESSO ===");
      Serial.print("Use este valor no código: #define SD_CS ");
      Serial.println(testPin);
      
      return;  // Encontrou, parar teste
    } else {
      Serial.println("❌ Falhou");
    }
    
    delay(500);
  }
  
  // Se chegou aqui, não encontrou
  Serial.println("\n❌ ERRO: Nenhum pino funcionou!");
  Serial.println("\nVerifique:");
  Serial.println("1. Cartão está inserido corretamente?");
  Serial.println("2. Cartão está formatado em FAT32?");
  Serial.println("3. Cartão funciona em outro dispositivo?");
  Serial.println("4. Conexões SPI estão OK?");
}

void loop() {
  // Nada aqui
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listando diretório: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Erro ao abrir diretório");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Não é um diretório");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
