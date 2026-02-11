# 🔄 PrintSense - Versão SDMMC Adaptada

## ✅ O Que Foi Mudado?

O código foi **completamente adaptado** para usar o modo **SDMMC** com os pinos específicos da sua placa ESP32-S3!

---

## 📌 Pinos SD Card (Modo SDMMC)

```cpp
// Pinos fixos do modo SDMMC (1-bit)
CLK  = GPIO 39  // Clock do SD Card
CMD  = GPIO 38  // Comando do SD Card
DATA0= GPIO 40  // Dados (modo 1-bit)
```

**Importante:** Estes pinos são **FIXOS** no modo SDMMC. Não podem ser alterados!

---

## 🔄 Principais Mudanças no Código

### 1. **Bibliotecas**

❌ **Removido (SPI):**
```cpp
#include <SD.h>
#include <SPI.h>
#define SD_CS 10
```

✅ **Adicionado (SDMMC):**
```cpp
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "FS.h"
#include "SD_MMC.h"

// Pinos SD
#define SD_CLK_PIN GPIO_NUM_39
#define SD_CMD_PIN GPIO_NUM_38
#define SD_D0_PIN  GPIO_NUM_40
```

### 2. **Inicialização do SD Card**

❌ **Antes (SPI):**
```cpp
if (!SD.begin(SD_CS)) {
    Serial.println("Falhou!");
}
```

✅ **Agora (SDMMC):**
```cpp
bool initSDCard() {
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
  
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.clk = SD_CLK_PIN;
  slot_config.cmd = SD_CMD_PIN;
  slot_config.d0 = SD_D0_PIN;
  slot_config.width = 1;  // 1-bit mode
  
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
  };
  
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, 
                    &slot_config, &mount_config, &sdCard);
  
  return (ret == ESP_OK);
}
```

### 3. **Caminhos de Arquivo**

❌ **Antes:**
```cpp
/logs/20240206.csv
/web/index.html
```

✅ **Agora:**
```cpp
/sdcard/logs/20240206.csv
/sdcard/web/index.html
```

**Todos os caminhos agora começam com `/sdcard/`!**

### 4. **Operações de Arquivo**

❌ **Antes (Arduino SD):**
```cpp
File file = SD.open("/logs/log.csv", FILE_WRITE);
file.println("dados");
file.close();
```

✅ **Agora (POSIX - padrão C):**
```cpp
FILE* file = fopen("/sdcard/logs/log.csv", "a");
fprintf(file, "dados\n");
fclose(file);
```

### 5. **Listar Arquivos**

❌ **Antes:**
```cpp
File root = SD.open("/logs");
File file = root.openNextFile();
while (file) {
  Serial.println(file.name());
  file = root.openNextFile();
}
```

✅ **Agora:**
```cpp
DIR* dir = opendir("/sdcard/logs");
struct dirent* entry;
while ((entry = readdir(dir)) != NULL) {
  Serial.println(entry->d_name);
}
closedir(dir);
```

---

## 📁 Estrutura de Pastas no SD Card

```
/sdcard/
├── logs/
│   ├── 20240206.csv
│   ├── 20240207.csv
│   └── ...
├── jobs/
│   ├── job_001.json
│   └── ...
└── web/
    ├── index.html
    ├── style.css
    └── script.js
```

**IMPORTANTE:** Criar pastas **DENTRO** de `/sdcard/`!

---

## 🚀 Como Usar

### 1. **Preparar SD Card**

```bash
# Formatar em FAT32 (Windows/Mac/Linux)

# Criar estrutura de pastas:
/sdcard/logs/
/sdcard/jobs/
/sdcard/web/
```

### 2. **Copiar Arquivos Web**

Copie para o SD Card:
- `index.html` → `/sdcard/web/index.html`
- `style.css` → `/sdcard/web/style.css`
- `script.js` → `/sdcard/web/script.js`

### 3. **Inserir SD Card**

Insira o cartão no slot até ouvir "click".

### 4. **Upload do Firmware**

```bash
1. Abrir PrintSense_ESP32.ino
2. Verificar que está usando modo SDMMC
3. Upload
4. Abrir Serial Monitor (115200)
```

### 5. **Verificar Inicialização**

Você deve ver:

```
=================================
PrintSense - Iniciando...
=================================

[1/4] Inicializando DHT22...
✅ DHT22 OK! Temp inicial: 24.5°C

[2/4] Inicializando BH1750...
✅ BH1750 OK!

[3/4] Inicializando SD Card (SDMMC)...
   Pinos SDMMC: CLK=39, CMD=38, DATA0=40
✅ SD Card OK!
   Nome: SD16G
   Tipo: SDHC/SDXC
   Capacidade: 14.83 GB
   Frequência: 40000 kHz
   Estrutura de pastas criada

[4/4] Configurando WiFi...
✅ AP criado! IP: 192.168.4.1

✅ WebServer iniciado!
=================================
Acesse: http://192.168.4.1
=================================
```

---

## ⚡ Vantagens do Modo SDMMC

### vs. Modo SPI:

| Característica | SPI | SDMMC |
|----------------|-----|-------|
| Velocidade | ~4 MB/s | ~20 MB/s |
| Pinos | Configurável | Fixo |
| CPU Overhead | Alto | Baixo |
| Compatibilidade | Universal | ESP32 específico |
| Logs por segundo | Limitado | Alto |

**Para este projeto, SDMMC é MELHOR!**

---

## 🔧 Troubleshooting

### ❌ "SDMMC falhou: 0x107"
**Causa:** Cartão não inserido ou mal inserido

**Solução:**
1. Remover cartão
2. Inserir novamente até ouvir "click"
3. Resetar ESP32

### ❌ "SDMMC falhou: 0x108"
**Causa:** Cartão não formatado ou incompatível

**Solução:**
1. Formatar em FAT32
2. Tamanho máximo: 32GB
3. Classe 10 recomendada

### ❌ "Formatação falhou"
**Causa:** Cartão defeituoso ou write-protected

**Solução:**
1. Verificar trava física do cartão (deve estar em UNLOCK)
2. Testar cartão em PC
3. Trocar por cartão novo se necessário

### ❌ SD funciona mas logs não salvam
**Causa:** Pastas não criadas

**Solução:**
```cpp
// O código cria automaticamente, mas manualmente:
mkdir("/sdcard/logs", 0777);
mkdir("/sdcard/jobs", 0777);
mkdir("/sdcard/web", 0777);
```

### ⚠️ Logs salvam mas não aparecem na API
**Causa:** Caminhos errados

**Solução:** Verificar que todos os caminhos começam com `/sdcard/`

---

## 📊 Velocidade de Logs

Com SDMMC, você pode salvar logs **muito mais rápido**:

```
SPI:  ~10 logs/segundo máximo
SDMMC: ~100 logs/segundo possível
```

Para este projeto (1 log/minuto), SDMMC é mais que suficiente!

---

## 🔄 Reverter para SPI (se necessário)

Se por algum motivo precisar voltar para SPI:

1. Trocar bibliotecas
2. Definir pino CS
3. Usar `SD.begin(CS)`
4. Mudar caminhos (`/logs/` em vez de `/sdcard/logs/`)
5. Usar API Arduino (`File` em vez de `FILE*`)

Mas **não recomendado** - SDMMC é superior para esta placa!

---

## ✅ Resumo

- ✅ Código adaptado para **modo SDMMC**
- ✅ Pinos corretos: **39, 38, 40**
- ✅ Todos caminhos com `/sdcard/`
- ✅ API POSIX para arquivos
- ✅ **4x mais rápido** que SPI
- ✅ Pronto para usar!

---

**Agora seu PrintSense está otimizado para sua placa ESP32-S3! 🚀**
