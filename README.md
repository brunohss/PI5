# 🖨️ PrintSense - Sistema IoT de Monitoramento para Impressoras 3D

<div align="center">

![PrintSense Logo](https://img.shields.io/badge/PrintSense-IoT%20Monitoring-667eea?style=for-the-badge)
![ESP32](https://img.shields.io/badge/ESP32--S3-Powered-orange?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Sistema inteligente de monitoramento ambiental para otimização de impressão 3D**

[📖 Documentação](#-documentação) • [🚀 Quick Start](#-quick-start) • [🎯 Funcionalidades](#-funcionalidades) • [🛠️ Desenvolvimento](#️-desenvolvimento)

</div>

---

## 📋 Sobre o Projeto

O **PrintSense** é um sistema IoT completo que monitora condições ambientais (temperatura, umidade, luminosidade e ruído) em tempo real para otimizar a qualidade de impressões 3D. Desenvolvido como Projeto Integrador do curso de Engenharia de Computação da UNIVESP.

### 🎯 Problema Resolvido

Impressões 3D falham frequentemente devido a condições ambientais inadequadas:
- 🔥 **Temperatura alta** → Warping e delaminação
- 💧 **Umidade elevada** → Absorção de água no filamento
- ☀️ **Luz excessiva** → Degradação de resinas fotossensíveis
- 📢 **Vibrações/ruído** → Imperfeições na superfície

### ✅ Solução

Sistema de monitoramento em tempo real que:
- Detecta condições inadequadas **antes** da falha
- Alerta via LEDs (🟢 IDEAL, 🟡 BOM, 🔴 RUIM)
- Registra dados históricos em SD Card
- Permite análise e otimização de processos
- Interface web para visualização e controle

---

## 🎯 Funcionalidades

### 🔧 Hardware
- ✅ **4 Sensores Integrados:**
  - DHT22 (Temperatura e Umidade)
  - LDR (Luminosidade)
  - MAX4466 (Som/Vibração com conversão dB SPL)
  - Sistema de LEDs indicadores
- ✅ **Display LCD I2C 20x4** - Visualização local
- ✅ **Encoder Rotativo EC11** - Navegação e seleção de material
- ✅ **SD Card** - Armazenamento de logs (CSV)
- ✅ **WiFi** - Interface web e API REST

### 💻 Software
- ✅ **Firmware ESP32** em C++ (Arduino)
- ✅ **Interface Web Responsiva** (HTML5 + CSS3 + JavaScript ES6)
- ✅ **Gráficos em Tempo Real** (Chart.js)
- ✅ **API REST Completa** (GET/POST)
- ✅ **Sistema de Logs** estruturado
- ✅ **Calibração de Sensores** ajustável
- ✅ **Thresholds Customizados** por material

### 📊 Materiais Suportados
- **PLA** (Temp: 18-28°C, Umid: 40-60%)
- **PETG** (Temp: 20-30°C, Umid: 30-50%)
- **ABS** (Temp: 22-32°C, Umid: 20-40%)
- **RESINA** (Temp: 20-25°C, Umid: 40-60%, Luz: <1000)

---

## 🚀 Quick Start

### 📋 Pré-requisitos

- **Hardware:** ESP32-S3, sensores (ver lista completa abaixo)
- **Software:** 
  - Arduino IDE 2.x ou superior
  - Python 3.8+ (para desenvolvimento)
  - Git

### 🔌 Hardware Necessário

| Componente | Quantidade | Função |
|------------|------------|--------|
| ESP32-S3 DevKit | 1 | Microcontrolador principal |
| DHT22 | 1 | Sensor de temperatura e umidade |
| LDR 5mm | 1 | Sensor de luminosidade |
| MAX4466 | 1 | Sensor de som (microfone eletreto) |
| LCD I2C 20x4 | 1 | Display local |
| Encoder EC11 | 1 | Entrada rotativa + botão |
| SD Card Module | 1 | Armazenamento de logs |
| LEDs 5mm | 3 | Verde, Amarelo, Vermelho |
| Resistores | 6 | 220Ω (3x LEDs), 10kΩ (3x pull-up) |
| Protoboard + Jumpers | 1 kit | Montagem |

### 📥 Instalação (Hardware Real)

#### 1. Clone o repositório
```bash
git clone https://github.com/brunohss/PI5.git
cd PI5
```

#### 2. Configure Arduino IDE
```
1. Instalar ESP32 Board Support:
   - File → Preferences
   - Additional Board Manager URLs: 
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   - Tools → Board → Boards Manager → Instalar "esp32"

2. Selecionar placa:
   - Tools → Board → ESP32 Arduino → ESP32S3 Dev Module

3. Instalar bibliotecas:
   - Sketch → Include Library → Manage Libraries
   - Instalar: DHT sensor library, Adafruit Unified Sensor, LiquidCrystal I2C
```

#### 3. Preparar SD Card
```bash
# Formatar SD Card como FAT32
# Criar estrutura de pastas:
/
├── web/
│   ├── index.html
│   ├── style.css
│   └── script.js
└── logs/
```

#### 4. Upload Web Interface
```bash
# Copiar arquivos da pasta 'web/' do repositório para SD Card
cp -r web/* /path/to/sdcard/web/
```

#### 5. Configurar WiFi
```cpp
// No arquivo firmware/PrintSense.ino
const char* ssid = "SEU_WIFI_AQUI";
const char* password = "SUA_SENHA_AQUI";
```

#### 6. Upload do Firmware
```
1. Conectar ESP32 via USB
2. Arduino IDE → Sketch → Upload
3. Aguardar compilação e upload
4. Abrir Serial Monitor (115200 baud)
5. Verificar IP atribuído
6. Acessar: http://IP_DO_ESP32
```

---

## 🧪 Desenvolvimento (Emulador)

Para desenvolver a interface **SEM precisar do hardware físico**, use nosso emulador completo!

### 🎮 Iniciar Emulador

#### Passo 1: Clone o repositório (se ainda não fez)
```bash
git clone https://github.com/brunohss/PI5.git
cd PI5
```

#### Passo 2: Iniciar servidor de emulação (backend)
```bash
# Terminal 1 - Raiz do projeto
python emulator_server.py

# Saída esperada:
# ✅ Servidor rodando em: http://localhost:8080
# 📡 API disponível em: http://localhost:8080/api/data
```

#### Passo 3: Iniciar servidor web (frontend)
```bash
# Terminal 2 - Entrar na pasta web
cd web
python -m http.server 5500

# Saída esperada:
# Serving HTTP on :: port 5500 (http://[::]:5500/) ...
```

#### Passo 4: Acessar interface
```
Abrir navegador em: http://localhost:5500
```

### 🎛️ Usando o Emulador

O emulador permite:
- ✅ Simular todos os sensores (temperatura, umidade, luz, som)
- ✅ Testar diferentes materiais (PLA, PETG, ABS, RESINA)
- ✅ Ver LEDs reagindo em tempo real
- ✅ Testar cenários pré-definidos (quente, úmido, ruidoso, etc.)
- ✅ Desenvolver frontend sem ESP32 físico
- ✅ Criar novas páginas (configuração, calibração, histórico)

### 📊 APIs Disponíveis (Emulador)

```bash
# Obter dados dos sensores
curl http://localhost:8080/api/data

# Mudar material
curl -X POST http://localhost:8080/api/material -d "material=ABS"

# Simular valores customizados
curl -X POST http://localhost:8080/api/simulate \
  -H "Content-Type: application/json" \
  -d '{"temperature": 30, "humidity": 70}'

# Obter configurações
curl http://localhost:8080/api/config

# Listar logs
curl http://localhost:8080/api/logs
```

### 🔧 Estrutura do Emulador

```
PI5/
├── emulator_server.py        ← Servidor backend (porta 8080)
├── web/                       ← Frontend (porta 5500)
│   ├── index.html            ← Dashboard principal
│   ├── style.css             ← Estilos
│   └── script.js             ← Lógica do frontend
└── firmware/                  ← Código ESP32
    └── PrintSense.ino
```

### 💡 Workflow de Desenvolvimento

```
┌──────────────────────────────────────────────┐
│  DESENVOLVENDO COM EMULADOR                  │
├──────────────────────────────────────────────┤
│                                              │
│  1. Rodar emulator_server.py (backend)      │
│  2. Rodar http.server (frontend)            │
│  3. Editar web/index.html                   │
│  4. Salvar e recarregar página (F5)         │
│  5. Ver mudanças instantaneamente!          │
│  6. Iterar rapidamente                      │
│                                              │
│  Quando pronto:                              │
│  7. Copiar web/* para SD Card               │
│  8. Upload firmware para ESP32              │
│  9. Funciona igual ao emulador! ✅          │
│                                              │
└──────────────────────────────────────────────┘
```

---

## 🛠️ Estrutura do Projeto

```
PI5/
├── firmware/
│   ├── PrintSense.ino          # Firmware principal ESP32
│   ├── config.h                # Configurações (WiFi, pins, etc)
│   └── README.md               # Documentação do firmware
├── web/
│   ├── index.html              # Dashboard principal
│   ├── style.css               # Estilos CSS
│   ├── script.js               # JavaScript do frontend
│   └── assets/                 # Imagens, ícones, etc
├── emulator_server.py          # Servidor emulador (desenvolvimento)
├── docs/
│   ├── hardware/               # Esquemáticos, diagramas
│   ├── manual/                 # Manual do usuário
│   └── api/                    # Documentação da API
├── tests/                      # Scripts de teste
├── examples/                   # Exemplos de uso
├── LICENSE                     # Licença MIT
└── README.md                   # Este arquivo
```

---

## 📡 API REST

### Endpoints Principais

#### `GET /api/data`
Retorna dados atuais dos sensores
```json
{
  "temperature": 25.3,
  "humidity": 55.2,
  "light": 1850,
  "soundADC": 580,
  "soundDB": 58.5,
  "timestamp": "2024-02-08 22:45:30",
  "material": "PLA",
  "status": "IDEAL",
  "ledStatus": "GREEN"
}
```

#### `POST /api/material`
Altera material sendo impresso
```bash
curl -X POST http://ESP32_IP/api/material -d "material=PETG"
```

#### `GET /api/logs`
Lista arquivos de log disponíveis
```json
{
  "logs": ["20240208.csv", "20240207.csv", "20240206.csv"]
}
```

#### `GET /api/log/YYYYMMDD.csv`
Download de log específico
```
timestamp,temperature,humidity,light,soundADC,soundDB,status
2024-02-08 00:00:00,22.5,72.0,450,520,56.2,BOM
2024-02-08 01:00:00,21.8,74.0,380,480,54.8,IDEAL
...
```

### Documentação Completa da API
Ver: [docs/api/README.md](docs/api/README.md)

---

## 🔌 Esquema de Ligação

### Pinout ESP32-S3

| Componente | Pino ESP32 | Descrição |
|------------|------------|-----------|
| DHT22 | GPIO 4 | Dados (temperatura/umidade) |
| LDR | GPIO 34 (ADC) | Leitura analógica (luz) |
| MAX4466 | GPIO 35 (ADC) | Leitura analógica (som) |
| LCD I2C | GPIO 21 (SDA) | Dados I2C |
| LCD I2C | GPIO 22 (SCL) | Clock I2C |
| Encoder CLK | GPIO 25 | Clock do encoder |
| Encoder DT | GPIO 26 | Data do encoder |
| Encoder SW | GPIO 27 | Botão do encoder |
| LED Verde | GPIO 13 | Indicador IDEAL |
| LED Amarelo | GPIO 12 | Indicador BOM |
| LED Vermelho | GPIO 14 | Indicador RUIM |
| SD Card | SDMMC (built-in) | 4-bit mode |

### Diagrama Completo
Ver: [docs/hardware/schematic.pdf](docs/hardware/schematic.pdf)

---

## 📊 Uso

### Interface Web

#### Dashboard Principal
- **Temperatura atual** em °C
- **Umidade atual** em %
- **Luminosidade** (0-4095)
- **Ruído** em dB SPL
- **Status global** (IDEAL/BOM/RUIM)
- **Gráficos em tempo real** (Chart.js)
- **Histórico** dos últimos 20 pontos

#### Display LCD Local
```
┌────────────────────┐
│ PrintSense v1.0    │
│ Material: PLA      │
│ T:25.3C H:55% IDEAL│
│ Luz:1850 Som:58dB  │
└────────────────────┘
```

#### LEDs Indicadores
- 🟢 **Verde (IDEAL):** Todas condições perfeitas
- 🟡 **Amarelo (BOM):** 1 parâmetro fora do ideal
- 🔴 **Vermelho (RUIM):** 2+ parâmetros fora do ideal

### Encoder Rotativo
- **Girar:** Navegar entre materiais
- **Pressionar:** Confirmar seleção
- **Longo press (2s):** Menu de configurações

### Logs CSV
Formato: `YYYYMMDD.csv`
```csv
timestamp,temperature,humidity,light,soundADC,soundDB,status
2024-02-08 22:45:30,25.3,76.0,1850,580,58.5,IDEAL
2024-02-08 22:46:30,25.4,75.8,1845,620,60.2,BOM
```

---

## 🔬 Calibração

### Temperatura (DHT22)
```cpp
#define TEMP_OFFSET 0.0  // Ajustar se necessário (±X °C)
```

### Umidade (DHT22)
```cpp
#define HUM_OFFSET 0.0   // Ajustar se necessário (±X %)
```

### Som (MAX4466)
Conversão ADC → dB SPL:
```cpp
// Tabela de calibração (ajustar com decibelímetro de referência)
{50, 30.0},    // ADC 50 = 30 dB
{150, 40.0},   // ADC 150 = 40 dB
{600, 60.0},   // ADC 600 = 60 dB (padrão)
{1000, 70.0}   // ADC 1000 = 70 dB
```

### Luz (LDR)
Ajustar resistor série (10kΩ padrão) ou escala no código:
```cpp
int light = analogRead(LDR_PIN);  // 0-4095
// Ajustar thresholds no código se necessário
```

Ver: [docs/manual/calibration.md](docs/manual/calibration.md)

---

## 🧪 Testes

### Teste Unitário de Sensores
```bash
cd tests
python test_sensors.py
```

### Teste de Integração
```bash
python test_integration.py
```

### Teste de Stress (24h)
```bash
python test_stress.py --duration 24
```

---

## 🤝 Contribuindo

Contribuições são bem-vindas! Por favor:

1. Fork o projeto
2. Crie uma branch (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

### Diretrizes
- ✅ Código limpo e comentado
- ✅ Testes unitários
- ✅ Documentação atualizada
- ✅ Commits semânticos

---

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja [LICENSE](LICENSE) para detalhes.

---

## 👥 Equipe

**Projeto Integrador V - UNIVESP 2024**

- **Valdir Silva** - Firmware e Hardware
- **Lívia Rodrigues** - Interface Web
- **Bruno Castro** - Design e UX
- **Danielle Costa** - Análise de Dados
- **Letícia Santos** - Documentação
- **Jonathan Silva** - Testes
- **Willians Souza** - Hardware
- **Bruno Silva** - Integração

**Orientadora:** Profa. Danielly Soares

---

## 🙏 Agradecimentos

- UNIVESP - Universidade Virtual do Estado de São Paulo
- Comunidade ESP32
- Bibliotecas Open Source utilizadas
- Farm de Impressão 3D (testes em campo)

---

## 📞 Contato

- **GitHub:** [@brunohss](https://github.com/brunohss)
- **Email:** contato@printsense.com.br
- **Projeto:** [PI5](https://github.com/brunohss/PI5)

---

## 🔗 Links Úteis

- [Documentação Completa](docs/)
- [Manual do Usuário](docs/manual/)
- [API Reference](docs/api/)
- [Troubleshooting](docs/troubleshooting.md)
- [FAQ](docs/faq.md)
- [Changelog](CHANGELOG.md)

---

<div align="center">

**⭐ Se este projeto te ajudou, considere dar uma estrela!**

Made with ❤️ by UNIVESP Team

</div>
