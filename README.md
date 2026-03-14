# 🖨️ PrintSense v4.0 FreeRTOS

<div align="center">

![PrintSense](https://img.shields.io/badge/PrintSense-v4.0%20FreeRTOS-blueviolet?style=for-the-badge&logo=espressif)
![ESP32](https://img.shields.io/badge/ESP32--S3-N16R8-orange?style=for-the-badge&logo=espressif)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-Dual--Core-success?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Sistema Profissional de Monitoramento Ambiental para Impressão 3D**

*Projeto Integrador V - Engenharia de Computação - UNIVESP 2026*

[🚀 Quick Start](#-quick-start) • [📖 Documentação](#-documentação) • [🎯 Funcionalidades](#-funcionalidades) • [🏗️ Arquitetura](#️-arquitetura)

</div>

---

## 🌟 Destaques

- ✨ **Arquitetura FreeRTOS Dual-Core**: 6 tasks paralelas otimizadas
- ⚡ **Resposta Instantânea**: Encoder a 100 Hz - 200x mais rápido
- 🌐 **Interface Web Profissional**: Dashboard Chart.js em tempo real
- 📊 **Logging Inteligente**: CSV assíncrono com timestamp NTP
- 🎛️ **4 Perfis de Material**: PLA, PETG, ABS, Resina
- 🔒 **Thread-Safe**: Mutexes + Queues + Event Groups
- 💾 **SD Card SDMMC**: Sistema de arquivos FAT32

---

## 🎯 Sobre o Projeto

**PrintSense** monitora condições ambientais (temperatura, umidade, luz, som) para otimizar impressão 3D. Usando arquitetura FreeRTOS dual-core no ESP32-S3, alerta quando condições estão inadequadas para cada tipo de material.

### Problema Resolvido
- 🔥 Temperatura inadequada → Warping
- 💧 Umidade elevada → Filamento absorve água
- ☀️ Luz excessiva → Degradação resina
- 📢 Vibrações → Imperfeições

---

## 🛠️ Tecnologias

### Hardware
- **ESP32-S3 N16R8** (Dual Xtensa LX7, 240MHz, 512KB+8MB PSRAM)
- **DHT22** (Temp/Umid), **LDR** (Luz), **MAX4466** (Som)
- **LCD I2C 20x4**, **Encoder EC11**, **3x LEDs**
- **SD Card SDMMC**

### Software
- **FreeRTOS** v10.4.3 - RTOS
- **Arduino Core** ESP32 - Framework
- **ArduinoJson**, **WiFi**, **WebServer** - Libs
- **Chart.js** - Frontend
- **HTTP REST API** - Comunicação

---

## 🏗️ Arquitetura FreeRTOS

### Dual-Core Distribution

```
CORE 0 (Protocol)           CORE 1 (Application)
┌──────────────────┐        ┌──────────────────┐
│ 🌐 WebServer (P2)│        │ 🌡️ Sensores (P3) │
│ 💾 Logging (P1)  │        │ 📺 LCD (P2)      │
└──────────────────┘        │ 💡 LEDs (P2)     │
                            │ 🎛️ Encoder (P3)  │
                            └──────────────────┘
```

### Tasks

| Task | Core | Priority | Cycle | Stack |
|------|------|----------|-------|-------|
| Sensores | 1 | 3 | 1000ms | 4KB |
| LCD | 1 | 2 | 500ms | 3KB |
| LEDs | 1 | 2 | 100ms | 2KB |
| **Encoder** | 1 | 3 | **10ms** | 2KB |
| WebServer | 0 | 2 | 1ms | 4KB |
| Logging | 0 | 1 | 60s | 3KB |

---

## 🚀 Quick Start

### Instalação

```bash
# 1. Clone
git clone https://github.com/brunohss/PI5.git
cd PI5

# 2. Prepare SD Card (FAT32)
mkdir -p /sdcard/web /sdcard/logs
cp web/* /sdcard/web/

# 3. Configure WiFi (firmware linha 24-25)
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

# 4. Upload via Arduino IDE
# 5. Acesse: http://ESP32_IP
```

### Arduino IDE Setup

```
1. Board Manager: ESP32S3 Dev Module
2. Libraries: DHT, LiquidCrystal_I2C, ArduinoJson
3. Upload Speed: 921600
4. Serial: 115200 baud
```

---

## 💻 Uso

### Encoder
1. **Pressionar** → Abre menu
2. **Girar** → Navega materiais
3. **Pressionar** → Confirma

**Resposta: 10ms!** ⚡

### LEDs
- 🟢 Verde: IDEAL (0 fora)
- 🟡 Amarelo: BOM (1 fora)
- 🔴 Vermelho: RUIM (2+ fora)

### Interface Web
```
http://192.168.1.100
```
- Dashboard tempo real
- Gráficos Chart.js
- Download logs CSV

---

## 📡 API REST

### GET /api/data
```json
{
  "temperature": 25.3,
  "humidity": 55.2,
  "soundDB": 60.5,
  "status": "IDEAL",
  "material": "PLA",
  "thresholds": {...}
}
```

### POST /api/material
```json
{"material": "PETG"}
```

### GET /api/logs
```json
{"logs": ["20260315.csv", ...]}
```

### GET /api/log?file=X.csv
Download CSV

---

## 📊 Performance

| Métrica | v3.1 | v4.0 | Melhoria |
|---------|------|------|----------|
| Encoder | 2000ms | 10ms | **200x** |
| CPU Uso | 50% | 18.5% | **2.7x** |
| RAM | 120KB | 118KB | Otim |

---

## 🐛 Troubleshooting

### Encoder Lento
- Verificar prioridade (P3)
- Adicionar pull-up 10kΩ + 100nF

### Web Sem Dados
```bash
curl http://ESP32_IP/api/data
```

Ver: [docs/WEB_TROUBLESHOOTING.md](docs/WEB_TROUBLESHOOTING.md)

---

## 👥 Equipe

**UNIVESP 2026**

- Valdir Silva - Firmware/FreeRTOS
- Bruno Silva - Hardware
- Lívia Rodrigues - Web
- Bruno Castro - UX
- Danielle Costa - Dados
- Letícia Santos - Docs
- Jonathan Silva - QA
- Willians Souza - Hardware / Modelagem 3D

**Orientadora:** Profa. Danielly Soares

---

## 📄 Licença

MIT License - Copyright (c) 2026 PrintSense Team

---

## 🔗 Links

- [Docs Completas](docs/)
- [Guia FreeRTOS](docs/FREERTOS_GUIDE.md)
- [API Reference](docs/api/)
- [Hardware](docs/hardware/)

---

<div align="center">

**⭐ Star o projeto se te ajudou!**

Made with ❤️ by UNIVESP Team

</div>
