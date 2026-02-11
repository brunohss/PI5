# PrintSense - Monitor Ambiental para Impressão 3D

Sistema completo de monitoramento ambiental para farms de impressoras 3D, rodando em **ESP32-S3 WROOM1** com cartão microSD.

![Status](https://img.shields.io/badge/Status-Pronto_para_Uso-success)
![Versão](https://img.shields.io/badge/Versão-1.0-blue)
![Hardware](https://img.shields.io/badge/Hardware-ESP32--S3-orange)

## 📋 Índice

- [Características](#-características)
- [Hardware Necessário](#-hardware-necessário)
- [Esquema de Ligação](#-esquema-de-ligação)
- [Instalação](#-instalação)
- [Configuração](#-configuração)
- [Uso](#-uso)
- [API REST](#-api-rest)
- [Troubleshooting](#-troubleshooting)

---

## ✨ Características

- ✅ **Sistema autônomo** - WebServer rodando no próprio ESP32
- ✅ **Armazenamento local** - Logs salvos em microSD
- ✅ **Multi-material** - Suporte para PLA, PETG, ABS e Resina
- ✅ **Interface responsiva** - Acesso via navegador (PC/mobile)
- ✅ **Análise em tempo real** - Status instantâneo das condições
- ✅ **Histórico completo** - Logs CSV para análise de defeitos
- ✅ **Sem servidor externo** - Tudo roda localmente

---

## 🛠️ Hardware Necessário

### Componentes Principais

| Componente | Modelo Recomendado | Quantidade | Preço Aprox. |
|------------|-------------------|------------|--------------|
| Microcontrolador | ESP32-S3 WROOM1 (com SD) | 1 | R$ 40-60 |
| Sensor Temp/Umidade | DHT22 (AM2302) | 1 | R$ 15-25 |
| Sensor Luminosidade | BH1750 (I2C) | 1 | R$ 8-15 |
| Sensor de Som | KY-037 ou MAX4466 | 1 | R$ 5-12 |
| MicroSD Card | 4GB+ (Classe 10) | 1 | R$ 10-20 |
| Fonte 5V | USB ou DC 5V/1A | 1 | R$ 10-15 |
| **TOTAL** | | | **~R$ 88-147** |

### Componentes Opcionais

- **Display OLED 0.96"** (I2C) - Para visualização local
- **MPU6050** - Sensor de vibração
- **Case impresso 3D** - Proteção do circuito
- **Cabos jumper** - Conexões

---

## 🔌 Esquema de Ligação

### Pinout ESP32-S3

```
ESP32-S3 WROOM1 (GPIO)
├── GPIO 4  → DHT22 (DATA)
├── GPIO 21 → BH1750 (SDA)
├── GPIO 22 → BH1750 (SCL)
├── GPIO 34 → KY-037 (AOUT)
├── GPIO 10 → MicroSD (CS) - Ajustar conforme PCB
└── 5V/GND  → Alimentação sensores
```

### Diagrama de Conexão

```
┌──────────────────────────────────────┐
│         ESP32-S3 WROOM1              │
│  ┌────────────────────────────────┐  │
│  │                                │  │
│  │  [MicroSD Card Slot]           │  │
│  │                                │  │
│  └────────────────────────────────┘  │
│                                      │
│  GPIO4  ●────────────┐               │
│  5V     ●────────┐   │               │
│  GND    ●─────┐  │   │               │
│               │  │   │               │
│  GPIO21 ●──┐  │  │   │               │
│  GPIO22 ●──│─┐│  │   │               │
│            │ ││  │   │               │
│  GPIO34 ●──│─│┼──│───│───┐           │
│            │ ││  │   │   │           │
└────────────┼─┼┼──┼───┼───┼───────────┘
             │ ││  │   │   │
         ┌───┴─┴┴──┴───┴───┴──────┐
         │   I2C BUS   5V  GND     │
         └────┬────────┬────┬──────┘
              │        │    │
        ┌─────┴───┐  ┌─┴────┴─┐  ┌────────┐
        │ BH1750  │  │ DHT22  │  │KY-037  │
        │  (I2C)  │  │(Temp/  │  │ (Som)  │
        │         │  │ Umid)  │  │        │
        └─────────┘  └────────┘  └────────┘
```

### Conexões Detalhadas

#### DHT22 (Temperatura e Umidade)
- VCC → 5V (ESP32)
- DATA → GPIO 4
- GND → GND

#### BH1750 (Luminosidade)
- VCC → 3.3V ou 5V
- SDA → GPIO 21
- SCL → GPIO 22
- GND → GND

#### KY-037 (Som)
- VCC → 5V
- AOUT → GPIO 34 (analógico)
- GND → GND

---

## 📥 Instalação

### 1. Preparação do Ambiente Arduino

```bash
# Instalar Arduino IDE (se não tiver)
# Download em: https://www.arduino.cc/en/software

# Instalar suporte ESP32:
# File → Preferences → Additional Boards Manager URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

# Tools → Board → Boards Manager → buscar "ESP32" → Install
```

### 2. Instalar Bibliotecas

No Arduino IDE: **Sketch → Include Library → Manage Libraries**

Instale as seguintes bibliotecas:

- ✅ **DHT sensor library** (by Adafruit)
- ✅ **Adafruit Unified Sensor**
- ✅ **BH1750** (by Christopher Laws)
- ✅ **ArduinoJson** (by Benoit Blanchon) - versão 6.x

### 3. Preparar MicroSD Card

1. **Formatar** o cartão em **FAT32**
2. Criar estrutura de pastas:

```
/
├── web/
│   ├── index.html
│   ├── style.css
│   └── script.js
├── logs/
└── jobs/
```

3. Copiar arquivos `index.html`, `style.css` e `script.js` para a pasta `/web/`

### 4. Upload do Firmware

1. Abrir `PrintSense_ESP32.ino` no Arduino IDE
2. Configurar placa:
   - **Board:** "ESP32S3 Dev Module"
   - **Flash Size:** "8MB" ou conforme sua placa
   - **Partition Scheme:** "Default 4MB with spiffs"
   - **PSRAM:** "QSPI PSRAM"
   
3. Ajustar configurações no código:

```cpp
// WiFi - Escolher modo AP ou Station
const char* ssid = "PrintSense";      // Nome do WiFi
const char* password = "printsense123"; // Senha

// Pino CS do SD Card (verificar PCB)
#define SD_CS 10
```

4. Conectar ESP32 via USB
5. **Upload** (Ctrl+U)

---

## ⚙️ Configuração

### Modo Access Point (AP) - Padrão

O ESP32 cria sua própria rede WiFi:

```
SSID: PrintSense
Senha: printsense123
IP: 192.168.4.1
```

**Acesso:** http://192.168.4.1

### Modo Station - Conectar à rede existente

No código, comentar linhas do AP e descomentar:

```cpp
// Modo Station (conectar à rede)
const char* ssid = "NOME_DA_SUA_REDE";
const char* password = "SUA_SENHA_WIFI";

// No setup():
// WiFi.softAP(ssid, password);  // <-- COMENTAR
WiFi.begin(ssid, password);      // <-- DESCOMENTAR
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}
Serial.println(WiFi.localIP());
```

Após upload, verificar IP no **Serial Monitor** (115200 baud).

---

## 🚀 Uso

### Acesso Inicial

1. Ligar o ESP32
2. Conectar ao WiFi "PrintSense" (senha: printsense123)
3. Abrir navegador: http://192.168.4.1
4. Aguardar carregamento da interface

### Seleção de Material

1. No topo da página, selecionar material:
   - **PLA** (padrão)
   - **PETG**
   - **ABS**
   - **Resina**

2. Sistema ajusta automaticamente faixas ideais

### Interpretação de Status

| Status | Significado | Ação |
|--------|-------------|------|
| ✅ **IDEAL** | Todas condições perfeitas | Pode imprimir |
| ⚠️ **BOM** | Temp/Umid OK, outros fatores fora | Avaliar se crítico |
| ❌ **RUIM** | Condições inadequadas | **NÃO imprimir** |

### Análise de Sensores

Cada sensor mostra:
- **Valor atual**
- **Faixa ideal** para o material selecionado
- **Status individual** (OK/Abaixo/Acima)

### Download de Logs

1. Rolar até "Histórico e Análise"
2. Clicar **"Ver Logs Salvos"**
3. Selecionar arquivo (formato: YYYYMMDD.csv)
4. Abrir no Excel/LibreOffice para análise

#### Formato do CSV

```csv
timestamp,temperature,humidity,light,sound,status
2024-02-05 14:30:00,24.5,52.3,350,45,IDEAL
2024-02-05 14:31:00,24.7,52.1,348,47,IDEAL
```

### Atalhos de Teclado

- **R** - Atualizar dados manualmente
- **L** - Mostrar logs disponíveis
- **1** - Trocar para PLA
- **2** - Trocar para PETG
- **3** - Trocar para ABS
- **4** - Trocar para Resina

---

## 🌐 API REST

Endpoints disponíveis para integração:

### GET /api/data

Retorna dados atuais dos sensores:

```json
{
  "temperature": 24.5,
  "humidity": 52.3,
  "light": 350,
  "sound": 45,
  "timestamp": "2024-02-05 14:30:00",
  "material": "PLA",
  "status": "IDEAL",
  "statusDetails": "Todas as condições ideais!",
  "thresholds": {
    "tempMin": 18,
    "tempMax": 28,
    "humMin": 40,
    "humMax": 60
  }
}
```

### POST /api/material

Alterar material monitorado:

```bash
curl -X POST http://192.168.4.1/api/material \
  -d "material=PETG"
```

### GET /api/logs

Listar arquivos de log disponíveis:

```json
{
  "logs": [
    "20240201.csv",
    "20240202.csv",
    "20240205.csv"
  ]
}
```

### GET /api/log?file=YYYYMMDD.csv

Download de log específico em formato CSV.

---

## 🔧 Troubleshooting

### ESP32 não conecta ao WiFi

**Solução:**
- Verificar se SSID/senha estão corretos
- Aumentar potência do sinal WiFi
- Verificar se ESP32 está dentro do alcance
- Resetar ESP32 (botão RESET)

### SD Card não é detectado

**Causas comuns:**
- Cartão não formatado em FAT32
- Pino CS incorreto (verificar GPIO na PCB)
- Cartão corrompido ou incompatível

**Solução:**
```cpp
// Ajustar pino CS no código
#define SD_CS 10  // Trocar para GPIO correto
```

### Sensor DHT22 retorna NaN

**Possíveis causas:**
- Conexão solta
- Sensor defeituoso
- Falta de pull-up resistor (4.7kΩ)

**Debug:**
```cpp
Serial.println(dht.readTemperature());  // Ver valor bruto
```

### BH1750 não responde

**Solução:**
- Verificar conexões I2C (SDA/SCL)
- Testar com I2C Scanner
- Trocar endereço I2C se necessário

### Interface não carrega

**Verificar:**
1. Arquivos HTML/CSS/JS estão na pasta `/web/` do SD?
2. SD Card está inserido corretamente?
3. Abrir Serial Monitor - ver mensagens de erro

### Logs não salvam

**Solução:**
- Verificar se pasta `/logs/` existe
- Checar espaço disponível no SD
- Formatar cartão e recriar estrutura

---

## 📊 Análise de Defeitos

Correlacionar defeitos com ambiente usando logs:

| Defeito | Causa Provável | Parâmetro |
|---------|----------------|-----------|
| **Warping** | Temp. muito baixa | Temperatura < mínimo |
| **Bolhas/Zits** | Umidade alta | Umidade > 60% |
| **Delaminação** | Variação térmica | Delta temp. > 5°C |
| **Resina não cura** | Temp. baixa | Temperatura < 20°C |
| **Stringing** | Umidade excessiva | Umidade > 70% |

### Exemplo de Análise

1. Imprimir objeto
2. Anotar horário início/fim
3. Após impressão, verificar status no log
4. Se houver defeito, comparar parâmetros com faixas ideais
5. Ajustar ambiente conforme necessário

---

## 🔮 Próximas Melhorias

- [ ] Sensor de vibração (MPU6050)
- [ ] Alertas via Telegram/Discord
- [ ] Integração com OctoPrint
- [ ] Previsão ML de defeitos
- [ ] Controle automático (desumidificador/aquecedor)
- [ ] Display OLED local
- [ ] Gráficos históricos na interface

---

## 📝 Licença

Projeto open-source - Livre para uso e modificação

## 👤 Autor

PrintSense v1.0 - Monitor Ambiental para Impressão 3D

---

## 🆘 Suporte

Para dúvidas e problemas:
1. Verificar esta documentação
2. Checar Serial Monitor (115200 baud)
3. Abrir issue no repositório (se aplicável)

**Boas impressões! 🎯🖨️**
