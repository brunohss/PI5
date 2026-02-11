# 🖨️ PrintSense - Monitor Ambiental Profissional para Impressão 3D

<div align="center">

![PrintSense Logo](https://img.shields.io/badge/PrintSense-v1.0-00d9ff?style=for-the-badge&logo=3d-printing)
[![License](https://img.shields.io/badge/License-MIT-00ff88?style=for-the-badge)](LICENSE)
[![ESP32](https://img.shields.io/badge/ESP32--S3-WROOM1-ff3366?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![Status](https://img.shields.io/badge/Status-Active-00ff88?style=for-the-badge)](https://github.com)

**Sistema completo de monitoramento em tempo real para ambientes de impressão 3D**

[Características](#-características) • [Hardware](#-hardware-necessário) • [Instalação](#-instalação) • [Uso](#-uso) • [API](#-api-rest) • [Contribuir](#-contribuindo)

</div>

---

## 📋 Índice

- [Sobre o Projeto](#-sobre-o-projeto)
- [Características](#-características)
- [Hardware Necessário](#-hardware-necessário)
- [Pinout](#-pinout)
- [Instalação](#-instalação)
- [Configuração](#-configuração)
- [Interface Web](#-interface-web)
- [Display LCD](#-display-lcd)
- [LEDs Indicadores](#-leds-indicadores)
- [API REST](#-api-rest)
- [Estrutura de Arquivos](#-estrutura-de-arquivos)
- [Troubleshooting](#-troubleshooting)
- [Roadmap](#-roadmap)
- [Contribuindo](#-contribuindo)
- [Licença](#-licença)

---

## 🎯 Sobre o Projeto

**PrintSense** é um sistema de monitoramento ambiental profissional desenvolvido especificamente para ambientes de impressão 3D. Monitora temperatura, umidade, luminosidade e ruído em tempo real, com interface web moderna, display LCD local e **indicadores LED visuais**.

### Por Que PrintSense?

- ✅ Diferentes materiais (PLA, PETG, ABS, Resina) requerem condições específicas
- ✅ Umidade afeta a qualidade de impressão
- ✅ Temperatura influencia a adesão e deformação
- ✅ Resina fotopolimérica é sensível à luz
- ✅ Controle ambiental = Impressões de qualidade

### Demonstração Visual

```
┌────────────────────────────────────────┐
│  🖨️ PRINTSENSE       ● ONLINE   🟢    │
│  Material: PLA                         │
│  Status: IDEAL                         │
├────────────────────────────────────────┤
│  🌡️ 25.3°C   💧 76.0%                 │
│  💡 1770 lux  🔊 1974                 │
│                                        │
│  📊 [Gráfico em tempo real]           │
│                                        │
│  LEDs: 🟢 ON | 🟡 OFF | 🔴 OFF       │
└────────────────────────────────────────┘
```

---

## ✨ Características

### 🌡️ Monitoramento Completo
- **DHT22** - Temperatura e umidade com precisão ±0.5°C / ±2%
- **LDR** - Sensor de luminosidade ambiente (0-4095 ADC)
- **MAX4466** - Medidor de nível de ruído
- Leituras a cada 1 segundo

### 📺 Interface Quádrupla
1. **LCD 20x4 Local** - Visualização em tempo real sem internet
2. **Dashboard Web PRO** - Tema cyberpunk com gráficos Chart.js
3. **LEDs Indicadores** - Status visual (🟢 Verde / 🟡 Amarelo / 🔴 Vermelho)
4. **API REST** - Integração com outros sistemas

### 🎛️ Controle Físico
- **Encoder Rotativo EC11** - Navegação entre materiais
- Girar para selecionar | Pressionar para confirmar
- Feedback visual imediato no LCD e LEDs

### 📊 Gráficos em Tempo Real
- **Chart.js** - 4 linhas simultâneas (Temp/Umid/Luz/Som)
- Histórico de 30 pontos (~1 minuto)
- Interativo: hover, zoom, legendas clicáveis

### 💾 Logging Robusto
- **SD Card SDMMC** - 4x mais rápido que SPI (~20 MB/s)
- Logs CSV organizados por data
- Download via interface web
- Histórico ilimitado

### 🌐 Conectividade
- **WiFi Dual Mode** - AP (Access Point) ou Station
- Fallback automático
- API REST completa

### 🎯 4 Perfis de Material
| Material | Temperatura | Umidade | Luz | Som | Características |
|----------|-------------|---------|-----|-----|-----------------|
| **PLA** | 18-28°C | 40-60% | < 3000 | < 2000 | Fácil, versátil |
| **PETG** | 20-30°C | 30-50% | < 3000 | < 2000 | Resistente, flexível |
| **ABS** | 22-32°C | 20-40% | < 3000 | < 2000 | Industrial, durável |
| **RESINA** | 20-25°C | 40-60% | < 1000 | < 1500 | Precisão, detalhes |

---

## 🔧 Hardware Necessário

### Componentes Principais

| Componente | Especificação | Preço | Onde Comprar |
|------------|---------------|-------|--------------|
| ESP32-S3 WROOM1 | Com SD Card integrado | R$ 45 | [AliExpress](https://aliexpress.com) |
| DHT22 | Sensor temp/umidade | R$ 15 | [Mercado Livre](https://mercadolivre.com.br) |
| LDR 5mm | Fotoresistor | R$ 2 | Loja local |
| MAX4466 | Módulo microfone | R$ 8 | [AliExpress](https://aliexpress.com) |
| LCD I2C 20x4 | Endereço 0x27 | R$ 25 | [Mercado Livre](https://mercadolivre.com.br) |
| Encoder EC11 | 5 terminais com botão | R$ 5 | [AliExpress](https://aliexpress.com) |
| **LED Verde** 🟢 | 5mm alto brilho | R$ 0,50 | Loja local |
| **LED Amarelo** 🟡 | 5mm alto brilho | R$ 0,50 | Loja local |
| **LED Vermelho** 🔴 | 5mm alto brilho | R$ 0,50 | Loja local |
| **Resistores LED** | 3x 220Ω | R$ 0,30 | Loja local |
| Resistor 10kΩ | Para LDR | R$ 0,10 | Loja local |
| SD Card | 16GB Classe 10 | R$ 20 | Qualquer |
| Protoboard | 830 pontos | R$ 10 | Loja local |
| Jumpers | 40 unidades M-M/M-F | R$ 8 | Loja local |
| Fonte USB | 5V 1A mínimo | R$ 10 | Qualquer |

**💰 Custo Total: ~R$ 150**

---

## 📌 Pinout

### Diagrama Completo

```
┌─────────────────────────────────────────────────┐
│              ESP32-S3 WROOM1                    │
├─────────────────────────────────────────────────┤
│                                                 │
│  SENSORES:                                      │
│  ├─ DHT22 (DATA)      → GPIO 17                 │
│  ├─ LDR (Analog)      → GPIO 4                  │
│  └─ MAX4466 (Analog)  → GPIO 8                  │
│                                                 │
│  DISPLAY LCD I2C (0x27):                        │
│  ├─ SDA               → GPIO 10 (customizado)   │
│  └─ SCL               → GPIO 13 (customizado)   │
│                                                 │
│  ENCODER ROTATIVO EC11:                         │
│  ├─ CLK (Canal A)     → GPIO 7                  │
│  ├─ DT  (Canal B)     → GPIO 6                  │
│  └─ SW  (Botão)       → GPIO 5                  │
│                                                 │
│  SD CARD SDMMC (1-bit mode):                    │
│  ├─ CLK               → GPIO 39                 │
│  ├─ CMD               → GPIO 38                 │
│  └─ D0                → GPIO 40                 │
│                                                 │
│  LEDs INDICADORES (Status Visual):              │
│  ├─ LED VERDE 🟢      → GPIO 2  (IDEAL)        │
│  ├─ LED AMARELO 🟡    → GPIO 15 (BOM)          │
│  └─ LED VERMELHO 🔴   → GPIO 16 (RUIM)         │
│                                                 │
│  ALIMENTAÇÃO:                                   │
│  ├─ 5V                → USB-C                   │
│  ├─ 3.3V              → Sensores                │
│  └─ GND               → Comum                   │
│                                                 │
└─────────────────────────────────────────────────┘
```

### Conexões dos LEDs

```
     ESP32-S3
        │
   ┌────┴────┬─────┐
   │         │     │
  GPIO2   GPIO15  GPIO16
   │         │     │
  [R]       [R]   [R]  ← Resistores 220Ω
  220Ω     220Ω  220Ω
   │         │     │
  LED      LED   LED
  🟢       🟡    🔴
  Verde   Amar  Verm
   │         │     │
   └─────────┴─────┴──→ GND

Comportamento:
- Apenas 1 LED aceso por vez
- Verde = IDEAL (todas condições OK)
- Amarelo = BOM (1 condição fora)
- Vermelho = RUIM (2+ condições fora)
```

### Circuito do LDR

```
     3.3V
      │
     [R] 10kΩ
      │
      ├────→ GPIO 4 (ADC)
      │
     [LDR] Fotoresistor
      │
     GND

Funcionamento:
☀️ Muita luz  → LDR baixa resistência → ADC lê BAIXO
🌙 Pouca luz → LDR alta resistência  → ADC lê ALTO
```

---

## 🚀 Instalação

### 1️⃣ Requisitos

- **Arduino IDE** 2.0+ ou **PlatformIO**
- **Placa ESP32** instalada no board manager
- **Bibliotecas necessárias:**
  ```
  ✓ DHT sensor library (Adafruit)
  ✓ Adafruit Unified Sensor
  ✓ LiquidCrystal I2C (Frank de Brabander)
  ✓ ArduinoJson (v6.21+)
  ```

### 2️⃣ Clonar Repositório

```bash
git clone https://github.com/seu-usuario/printsense.git
cd printsense
```

### 3️⃣ Instalar Bibliotecas

#### Arduino IDE:
```
Sketch → Include Library → Manage Libraries

Buscar e instalar:
✓ DHT sensor library (Adafruit)
✓ Adafruit Unified Sensor
✓ LiquidCrystal I2C
✓ ArduinoJson (v6.21+)
```

#### PlatformIO:
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

lib_deps =
    adafruit/DHT sensor library@^1.4.4
    adafruit/Adafruit Unified Sensor@^1.1.9
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
    bblanchon/ArduinoJson@^6.21.3
```

### 4️⃣ Montar Hardware

#### a) Conectar Sensores
```
DHT22:
├─ VCC → 3.3V
├─ DATA → GPIO 17 (+ resistor 4.7kΩ pull-up)
└─ GND → GND

LDR:
├─ Terminal 1 → 3.3V (via resistor 10kΩ)
├─ Terminal 2 → GPIO 4 + GND
```

#### b) Conectar Display LCD
```
LCD I2C:
├─ VCC → 5V
├─ GND → GND
├─ SDA → GPIO 10
└─ SCL → GPIO 13
```

#### c) Conectar LEDs 🟢🟡🔴
```
LED Verde (IDEAL):
GPIO 2 → [220Ω] → LED (+) → LED (-) → GND

LED Amarelo (BOM):
GPIO 15 → [220Ω] → LED (+) → LED (-) → GND

LED Vermelho (RUIM):
GPIO 16 → [220Ω] → LED (+) → LED (-) → GND

IMPORTANTE: Perna LONGA do LED = Anodo (+)
            Perna CURTA do LED = Catodo (-)
```

#### d) Conectar Encoder
```
Encoder EC11:
├─ Pino 1 (GND) → GND
├─ Pino 2 (CLK) → GPIO 7
├─ Pino 3 (SW)  → GPIO 5
├─ Pino 4 (DT)  → GPIO 6
└─ Pino 5 (GND) → GND
```

### 5️⃣ Configurar WiFi

Edite `PrintSense_LCD_I2C_FIXED.ino`:

```cpp
// WiFi - Modo Station (conectar à rede)
const char* ssid = "SUA_REDE";         // ← Altere aqui
const char* password = "SUA_SENHA";    // ← Altere aqui
```

### 6️⃣ Preparar SD Card

1. **Formatar** em FAT32
2. **Criar estrutura:**
   ```
   /logs/
   /jobs/
   /web/
       ├── index.html
       └── script.js
   ```

3. **Copiar arquivos web:**
   ```bash
   cp web/index_pro.html [SD_CARD]/web/index.html
   cp web/script_pro.js [SD_CARD]/web/script.js
   ```

### 7️⃣ Fazer Upload

1. Conectar ESP32 via USB
2. Selecionar placa: **ESP32S3 Dev Module**
3. Selecionar porta COM
4. Clicar **Upload** (Ctrl+U)

### 8️⃣ Verificar Funcionamento

Abrir **Serial Monitor** (115200 baud):

```
=================================
PrintSense - Iniciando...
=================================

[1/6] Inicializando LCD...
✅ LCD inicializado (I2C: SDA=10, SCL=13)

[2/6] Configurando Encoder...
✅ Encoder configurado (GPIO 5,6,7)

[3/6] Inicializando DHT22...
✅ DHT22 OK! Temp: 24.5°C

[4/6] Configurando LEDs...
✅ LEDs configurados
   Verde (IDEAL): GPIO 2
   Amarelo (BOM): GPIO 15
   Vermelho (RUIM): GPIO 16
   [Teste visual: 🟢→🟡→🔴]

[5/6] Inicializando SD Card (SDMMC)...
✅ SD Card OK!
   Nome: SD16G
   Capacidade: 14.83 GB

[6/6] Configurando WiFi...
✅ WiFi conectado!
   IP: 192.168.1.100

✅ WebServer iniciado!
Acesse: http://192.168.1.100

Status atual: IDEAL
LED Verde: ON 🟢
```

---

## 🔴🟡🟢 LEDs Indicadores

### Sistema de Status Visual

Os LEDs fornecem **feedback imediato** do status ambiental, visível de longe:

| LED | Cor | Status | Condição | Ação |
|-----|-----|--------|----------|------|
| 🟢 | **Verde** | **IDEAL** | Todas as condições dentro da faixa | Continue imprimindo |
| 🟡 | **Amarelo** | **BOM** | Uma condição ligeiramente fora | Monitorar |
| 🔴 | **Vermelho** | **RUIM** | Múltiplas condições ruins | Verificar ambiente |

### Como Funciona

```cpp
// Lógica de Status
void updateLEDs() {
  // Contar condições fora da faixa
  int outOfRange = 0;
  
  if (temp fora da faixa) outOfRange++;
  if (umidade fora da faixa) outOfRange++;
  if (luz fora da faixa) outOfRange++;
  if (som fora da faixa) outOfRange++;
  
  // Determinar status
  if (outOfRange == 0) {
    // IDEAL - Verde
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    
  } else if (outOfRange == 1) {
    // BOM - Amarelo
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, LOW);
    
  } else {
    // RUIM - Vermelho
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, HIGH);
  }
}
```

### Comportamento dos LEDs

- ✅ **Apenas 1 LED aceso** por vez
- ✅ **Atualização imediata** ao mudar condições
- ✅ **Sincronizado** com LCD e interface web
- ✅ **Baixo consumo** (~60mA total, 20mA por LED)
- ✅ **Teste na inicialização** (Verde→Amarelo→Vermelho)

### Exemplo de Uso

```
Cenário 1: Tudo OK
├─ Temp: 25°C ✓ (18-28°C)
├─ Umid: 50% ✓ (40-60%)
├─ Luz: 1500 ✓ (<3000)
├─ Som: 1800 ✓ (<2000)
└─ LED: 🟢 Verde LIGADO

Cenário 2: Umidade Alta
├─ Temp: 25°C ✓
├─ Umid: 65% ✗ (fora: 40-60%)
├─ Luz: 1500 ✓
├─ Som: 1800 ✓
└─ LED: 🟡 Amarelo LIGADO

Cenário 3: Temp Alta + Umid Baixa
├─ Temp: 30°C ✗ (fora: 18-28°C)
├─ Umid: 35% ✗ (fora: 40-60%)
├─ Luz: 1500 ✓
├─ Som: 1800 ✓
└─ LED: 🔴 Vermelho LIGADO
```

### Posicionamento Sugerido

```
         [PrintSense Device]
    ┌─────────────────────────┐
    │                         │
    │   [LCD Display 20x4]    │
    │                         │
    │   ┌───┐ ┌───┐  ┌───┐    │
    │   │🟢│  │🟡│  │🔴│     │ ← LEDs frontais
    │   └───┘ └───┘  └───┘    │
    │                         │
    │   [Encoder]   [Sensors] │
    └─────────────────────────┘
```

---

## 📺 Display LCD

### Layout 20x4

```
┌────────────────────┐
│PLA * IDEAL      🟢 │  Linha 0: Material, status, indicador LED
│T:25.3°C   18-28°C  │  Linha 1: Temperatura + faixa ideal
│H:76.0%    40-60%   │  Linha 2: Umidade + faixa ideal
│Luz:1770  Som:1974  │  Linha 3: Luminosidade e Som
└────────────────────┘
```

### Símbolos de Status

| Símbolo | Significado | LED Correspondente |
|---------|-------------|--------------------|
| **\*** | IDEAL | 🟢 Verde |
| **!** | BOM | 🟡 Amarelo |
| **X** | RUIM | 🔴 Vermelho |

### Indicador Visual no LCD

No canto direito da primeira linha, aparece um caractere que representa o LED aceso:

```
Status IDEAL:
PLA * IDEAL      ▓  ← Bloco cheio = Verde

Status BOM:
PETG ! BOM       ▒  ← Bloco médio = Amarelo

Status RUIM:
ABS X RUIM       ░  ← Bloco vazio = Vermelho
```

---

## 🌐 Interface Web

### Dashboard Profissional

**Características:**
- ✨ Tema cyberpunk (cyan/verde neon)
- 📊 Gráficos Chart.js em tempo real
- 📱 Responsivo (mobile/desktop)
- 🎨 Animações suaves
- 🔄 Atualização automática (2s)
- 🔴🟡🟢 **Badge de status com cor do LED**

### Acessar Interface

1. Conectar ao WiFi (PrintSense ou sua rede)
2. Abrir navegador
3. Digite: `http://192.168.4.1` (AP) ou `http://[IP_DO_ESP32]`

### Status Badge com Cor do LED

```html
<!-- Verde (IDEAL) -->
<div class="status-badge ideal">
  🟢 IDEAL para PLA
</div>

<!-- Amarelo (BOM) -->
<div class="status-badge bom">
  🟡 BOM para PLA
</div>

<!-- Vermelho (RUIM) -->
<div class="status-badge ruim">
  🔴 RUIM para PLA
</div>
```

---

## 🔌 API REST

### Endpoints Disponíveis

#### **GET** `/api/data`
Retorna dados atuais dos sensores + status LED

**Response:**
```json
{
  "temperature": 25.3,
  "humidity": 76.0,
  "light": 1770,
  "sound": 1974,
  "timestamp": "2024-02-08 22:45:30",
  "material": "PLA",
  "status": "IDEAL",
  "ledStatus": "GREEN",
  "statusDetails": "Condições ideais!",
  "thresholds": {
    "tempMin": 18,
    "tempMax": 28,
    "humMin": 40,
    "humMax": 60
  }
}
```

#### **POST** `/api/material`
Altera o material monitorado

**Request:**
```
POST /api/material
Content-Type: application/x-www-form-urlencoded

material=PETG
```

**Response:**
```json
{
  "success": true,
  "newMaterial": "PETG",
  "status": "BOM",
  "ledStatus": "YELLOW"
}
```

---

## 📁 Estrutura de Arquivos

```
printsense/
├── firmware/
│   ├── PrintSense_LCD_I2C_FIXED.ino    # Firmware principal ⭐
│   ├── config.h                         # Configurações
│   └── tests/
│       ├── Test_Encoder_EC11.ino
│       ├── Test_LEDs.ino                # Teste dos LEDs ⭐
│       ├── Test_SDCard.ino
│       └── Test_DHT22.ino
│
├── web/
│   ├── index_pro.html                  # Interface web
│   ├── script_pro.js                   # JavaScript + Chart.js
│   └── presentation.html               # Página de apresentação
│
├── docs/
│   ├── README.md                       # Este arquivo
│   ├── PINOUT.md                       # Diagrama detalhado
│   ├── SHOPPING_LIST.md                # Lista de compras
│   ├── LED_WIRING.md                   # Guia de conexão LEDs ⭐
│   ├── TROUBLESHOOTING.md              # Solução de problemas
│   └── API_DOCUMENTATION.md            # Documentação da API
│
├── schematics/
│   ├── circuit_full.fzz                # Fritzing completo
│   ├── circuit_full.png                # Imagem do circuito
│   └── led_detail.png                  # Detalhe dos LEDs ⭐
│
├── examples/
│   ├── python_monitor.py               # Cliente Python
│   ├── led_external_control.py         # Controlar LEDs via API ⭐
│   └── data_analysis.ipynb             # Análise de dados
│
└── README.md                           # Este arquivo
```

---

## 🐛 Troubleshooting

### LEDs não acendem

**Causa 1:** Polaridade invertida

**Solução:**
```
Verificar orientação do LED:
- Perna LONGA (+) → Resistor → GPIO
- Perna CURTA (-) → GND
```

**Causa 2:** Resistor errado

**Solução:**
```
Usar resistor 220Ω (vermelho-vermelho-marrom)
Não usar valores muito altos (>1kΩ)
```

**Causa 3:** LED queimado

**Solução:**
```
Testar LED com multímetro em modo diodo
Substituir se necessário
```

### LED fica sempre aceso/apagado

**Causa:** GPIO não configurado

**Solução:**
```cpp
void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  // Testar
  digitalWrite(LED_GREEN, HIGH);
  delay(1000);
  digitalWrite(LED_GREEN, LOW);
}
```

### Todos os LEDs acendem juntos

**Causa:** GND comum incorreto

**Solução:**
```
Verificar que todos os LEDs compartilham o mesmo GND
Usar protoboard para conexões organizadas
```

---

## 🧪 Teste dos LEDs

### Sketch de Teste

```cpp
#define LED_GREEN 2
#define LED_YELLOW 15
#define LED_RED 16

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  Serial.println("Teste de LEDs - PrintSense");
  Serial.println("===========================");
}

void loop() {
  // Verde
  Serial.println("🟢 LED Verde");
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  delay(2000);
  
  // Amarelo
  Serial.println("🟡 LED Amarelo");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, LOW);
  delay(2000);
  
  // Vermelho
  Serial.println("🔴 LED Vermelho");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  delay(2000);
  
  // Todos apagados
  Serial.println("⚫ Todos apagados");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  delay(1000);
}
```

**Comportamento esperado:**
```
🟢 Acende por 2s
🟡 Acende por 2s
🔴 Acende por 2s
⚫ Todos apagam por 1s
(repete)
```

---

## 🗺️ Roadmap

### ✅ v1.0 (Atual)
- [x] Monitoramento de 4 sensores
- [x] Display LCD 20x4
- [x] Encoder rotativo
- [x] Interface web com gráficos
- [x] Logging em SD Card
- [x] WiFi dual mode
- [x] **LEDs indicadores (Verde/Amarelo/Vermelho)**
- [x] 4 perfis de material
- [x] API REST completa

### 🚧 v1.1 (Próximo)
- [ ] Buzzer para alertas sonoros
- [ ] Modo noturno (desligar LEDs)
- [ ] PWM nos LEDs (controle de brilho)
- [ ] Alertas por email/Telegram
- [ ] App mobile (Flutter)

### 🔮 v2.0 (Futuro)
- [ ] Controle de relés (aquecedor/desumidificador)
- [ ] LEDs RGB (mais cores de status)
- [ ] Tira LED WS2812B para efeitos
- [ ] Machine Learning para predição
- [ ] PCB profissional

---

## 🤝 Contribuindo

Contribuições são muito bem-vindas! Áreas que precisam de ajuda:

- 🔴🟡🟢 **LEDs:** Sugestões de novos padrões de status
- 📱 **App Mobile:** Interface para controlar os LEDs
- 🎨 **Design:** Case 3D que mostre os LEDs
- 📊 **Análise:** Dashboard com histórico dos LEDs
- 🧪 **Testes:** Validação em diferentes hardwares

### Como Contribuir

1. Fork o repositório
2. Crie uma branch (`git checkout -b feature/minha-funcionalidade`)
3. Commit suas mudanças (`git commit -m "Adiciona funcionalidade X"`)
4. Push para o GitHub (`git push origin feature/minha-funcionalidade`)
5. Abra um Pull Request

---

## 📄 Licença

Este projeto está licenciado sob a **MIT License**.

---

## 📞 Contato

**Projeto Integrador** - Monitoramento Ambiental para Impressão 3D

- 🌐 Website: [printsense.github.io](https://printsense.github.io)
- 📧 Email: printsense.team@gmail.com
- 💬 Discord: [PrintSense Community](https://discord.gg/printsense)

---

<div align="center">

**Feito com ❤️ pela Equipe PrintSense**

⭐ Se este projeto te ajudou, considere dar uma estrela!

**Status atual:** 🟢 IDEAL | 🟡 BOM | 🔴 RUIM

[⬆ Voltar ao topo](#-printsense---monitor-ambiental-profissional-para-impressão-3d)

</div>
