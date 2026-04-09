🤖 PrintSense
<div align="center">
![PrintSense Logo](https://img.shields.io/badge/PrintSense-v4.0-667eea?style=for-the-badge&logo=arduino&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32--S3-Hardware-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)
Sistema de Monitoramento Ambiental Profissional para Impressão 3D
Monitore temperatura, umidade, luminosidade e ruído em tempo real para garantir condições ideais de impressão 3D
Documentação • Instalação • APIs • Contribuir
</div>
---
📋 Índice
Sobre o Projeto
Funcionalidades
Aplicações Desenvolvidas
Hardware
Instalação
Firmware ESP32
Desktop App (WPF)
Mobile App (MAUI)
Emulador Python
APIs REST
Exemplos de Uso
Arquitetura
Roadmap
Contribuindo
Licença
Contato
---
🎯 Sobre o Projeto
PrintSense é um sistema completo de monitoramento ambiental desenvolvido para garantir condições ideais durante processos de impressão 3D. O sistema monitora continuamente 4 parâmetros ambientais e indica visualmente se as condições estão adequadas para diferentes tipos de materiais (PLA, PETG, ABS, RESINA).
Por que PrintSense?
🌡️ Temperatura inadequada = impressões falhas
💧 Umidade excessiva = filamentos degradados
☀️ Luz UV = materiais envelhecendo prematuramente
🔊 Ruído excessivo = ambiente inadequado
PrintSense resolve todos esses problemas com monitoramento em tempo real!
---
✨ Funcionalidades
Hardware (ESP32)
✅ Monitoramento de 4 sensores em tempo real
✅ Display LCD 20x4 com informações detalhadas
✅ 3 LEDs indicadores (🟢 IDEAL / 🟡 BOM / 🔴 RUIM)
✅ Encoder rotativo para seleção de material
✅ Logging em SD Card (formato CSV)
✅ WebServer HTTP com API REST completa
✅ Persistência de configurações por material
✅ WiFi integrado para acesso remoto
Desktop Application (C# WPF)
✅ 5 abas completas (Dashboard, Calibração, Configurações, Relatórios, Análise IA)
✅ Gráficos em tempo real com LiveCharts
✅ Machine Learning com ML.NET (89.3% acurácia)
✅ Exportação de relatórios para Excel
✅ Tema dark mode cyberpunk
✅ Recomendações de ROI baseadas em ML
Mobile Application (.NET MAUI)
✅ Multiplataforma (Android, iOS, Windows)
✅ Interface responsiva e moderna
✅ Monitoramento em tempo real
✅ Notificações de status
✅ 70-80% do código WPF reutilizado
Emuladores de Desenvolvimento
✅ Emulador Python V6.0 com interface web
✅ Emulador HTML completo standalone
✅ Persistência em arquivos JSON
✅ 100% compatível com APIs do ESP32
---
🎨 Aplicações Desenvolvidas
Este projeto inclui 5 aplicações completas:
1. 🔧 Firmware ESP32 (C++)
```
📁 PrintSense_v4_0.ino
📊 1,105 linhas de código
💾 Persistência em SD Card
🌐 WebServer HTTP integrado
```
Tecnologias: C++, Arduino IDE, ESP32-S3
Funcionalidades: Leitura de sensores, cálculo de status, API REST, logging
2. 🖥️ Desktop Application (C# WPF)
```
📁 PrintSense.sln
📊 2,000+ linhas de código
🎨 5 abas funcionais
🤖 Machine Learning integrado
```
Tecnologias: C#, WPF, .NET Framework 4.8, Visual Studio 2022
Bibliotecas: LiveCharts, ML.NET, EPPlus, Newtonsoft.Json
Abas Implementadas:
Dashboard - Monitoramento em tempo real com gráficos
Calibração - Ajuste de offsets dos sensores
Configurações - IP, porta, intervalo de atualização
Relatórios - Estatísticas e exportação para Excel
Análise Inteligente - ML.NET com previsões e ROI
3. 📱 Mobile Application (.NET MAUI)
```
📁 PrintSense.Mobile.sln
📊 Arquitetura MVVM
🌐 Multiplataforma
📱 Android + iOS + Windows
```
Tecnologias: C#, .NET MAUI, XAML, .NET 8
Status: Dashboard funcional, outras abas em desenvolvimento
4. 🐍 Emulador Python V6.0
```
📁 emulator_server_v6.py
📊 350 linhas de código
🌐 Interface web integrada
💾 Persistência em JSON
```
Tecnologias: Python 3.7+, HTTPServer (stdlib)
Funcionalidades:
Interface web para editar materiais
API REST completa (9 endpoints)
Persistência em `./data/config_*.json`
Log visual de operações
100% compatível com ESP32
5. 🌐 Emulador HTML Standalone
```
📁 ESP32_Emulator_Complete.html
📊 1,500 linhas de código
🎨 Interface completa
🔧 6 APIs simuladas
```
Tecnologias: HTML5, CSS3, JavaScript (Vanilla)
Funcionalidades: Simulação completa do ESP32 sem servidor
---
🛠️ Hardware
Componentes Necessários
Componente	Quantidade	Preço Estimado
ESP32-S3 WROOM1	1x	R$ 35-50
DHT22 (Temp/Umid)	1x	R$ 25-30
LDR (Sensor de Luz)	1x	R$ 2-5
MAX4466 (Microfone)	1x	R$ 15-25
LCD I2C 20x4	1x	R$ 30-40
Encoder Rotativo EC11	1x	R$ 8-12
LEDs 5mm (3 cores)	3x	R$ 1.50
Módulo SD Card	1x	R$ 8-15
Cartão microSD 8-32GB	1x	R$ 15-25
Resistor 220Ω	3x	R$ 0.30
Resistor 10kΩ	1x	R$ 0.10
Protoboard + Jumpers	1x kit	R$ 25-40
TOTAL	-	R$ 166-271
Pinout ESP32-S3
```
DHT22 Data     → GPIO 17
LDR Analog     → GPIO 4 (ADC)
MAX4466 Analog → GPIO 8 (ADC)
LCD SDA        → GPIO 10
LCD SCL        → GPIO 13
Encoder CLK    → GPIO 7
Encoder DT     → GPIO 6
Encoder SW     → GPIO 5
LED Verde      → GPIO 2 + 220Ω
LED Amarelo    → GPIO 15 + 220Ω
LED Vermelho   → GPIO 16 + 220Ω
SD CLK         → GPIO 39
SD CMD         → GPIO 38
SD D0          → GPIO 40
```
Esquema de Montagem
```
       ┌────────────┐
       │  ESP32-S3  │
       └─────┬──────┘
             │
    ┌────────┼────────┐
    │        │        │
  DHT22    LDR    MAX4466
    │        │        │
    └────────┴────────┘
             │
    ┌────────┼────────┐
    │        │        │
  LCD 20x4  LEDs   SD Card
```
---
🚀 Instalação
Firmware ESP32
Pré-requisitos
Arduino IDE 2.0+
Pacote ESP32 instalado
Bibliotecas: DHT, LiquidCrystal_I2C, ArduinoJson
Passos
Instalar Arduino IDE
```bash
# Download: https://www.arduino.cc/en/software
```
Adicionar ESP32
```
Arquivo → Preferências → URLs Adicionais:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Instalar Bibliotecas
```
Sketch → Incluir Biblioteca → Gerenciar Bibliotecas
Buscar e instalar:
- DHT sensor library
- LiquidCrystal I2C
- ArduinoJson
```
Configurar WiFi
```cpp
// Editar linhas ~70-74 em PrintSense_v4_0.ino
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
```
Upload
```
Ferramentas → Placa → ESP32-S3 Dev Module
Ferramentas → Porta → (selecionar porta COM)
Sketch → Upload (Ctrl+U)
```
Verificar IP
```
Ferramentas → Monitor Serial (115200 baud)
Anotar IP exibido: ex: 192.168.1.100
```
---
Desktop App (WPF)
Pré-requisitos
Windows 10/11
Visual Studio 2022
.NET Framework 4.8
Passos
Clonar Repositório
```bash
git clone https://github.com/seu-usuario/printsense.git
cd printsense/Desktop
```
Abrir Solução
```
Abrir PrintSense.sln no Visual Studio 2022
```
Restaurar Pacotes NuGet
```
Projeto → Gerenciar Pacotes NuGet → Restaurar
```
Compilar e Executar
```
Build → Compilar Solução (Ctrl+Shift+B)
Depurar → Iniciar (F5)
```
Pacotes NuGet Necessários
EPPlus 8.5.0
LiveCharts.Wpf 0.9.7
Microsoft.ML 5.0.0
Newtonsoft.Json 13.0.4
---
Mobile App (MAUI)
Pré-requisitos
Visual Studio 2022
.NET 8 SDK
Workload: .NET MAUI
Passos
Instalar Workload
```bash
dotnet workload install maui
```
Abrir Projeto
```
Abrir PrintSense.Mobile.sln
```
Selecionar Target
```
Android / iOS / Windows
```
Executar
```
F5 (Debug)
```
---
Emulador Python
Pré-requisitos
Python 3.7+
Passos
Executar Emulador
```bash
python3 emulator_server_v6.py
```
Acessar Interface
```
Navegador: http://localhost:8080/
```
Editar Materiais
```
Interface web permite editar configurações de PLA, PETG, ABS, RESINA
Alterações salvas em ./data/config_*.json
```
Estrutura de Arquivos
```
projeto/
├── emulator_server_v6.py
└── data/
    ├── config_PLA.json
    ├── config_PETG.json
    ├── config_ABS.json
    └── config_RESINA.json
```
---
📡 APIs REST
O ESP32 expõe 8 endpoints HTTP:
1. GET /api/data
Retorna dados atuais dos sensores
Resposta:
```json
{
  "temperature": 25.3,
  "humidity": 52.0,
  "light": 1850,
  "soundDB": 58.5,
  "material": "PLA",
  "status": "IDEAL",
  "ledStatus": "GREEN",
  "thresholds": {
    "tempMin": 18,
    "tempMax": 28,
    "humMin": 40,
    "humMax": 60,
    "lightMax": 3000,
    "soundMaxDB": 70
  }
}
```
2. POST /api/material
Altera o material ativo
Requisição:
```json
{
  "material": "PETG"
}
```
Resposta:
```json
{
  "success": true,
  "newMaterial": "PETG",
  "status": "BOM"
}
```
3. GET /api/config
Retorna configuração do material atual
4. POST /api/config
Atualiza e salva configuração no SD Card
Requisição:
```json
{
  "tempMin": 20,
  "tempMax": 30,
  "humMin": 40,
  "humMax": 60
}
```
5. GET /api/calibration
Retorna offsets de calibração
6. POST /api/calibration
Salva novos offsets
7. GET /api/logs
Lista arquivos de log no SD Card
8. GET /api/log?file=YYYYMMDD.csv
Download de log específico
---
💻 Exemplos de Uso
JavaScript (Frontend Web)
```javascript
// Buscar dados do PrintSense
async function atualizarDados() {
    const response = await fetch('http://192.168.1.100/api/data');
    const data = await response.json();
    
    console.log(`Temperatura: ${data.temperature}°C`);
    console.log(`Status: ${data.status}`);
}

// Trocar material
async function trocarMaterial(material) {
    const response = await fetch('http://192.168.1.100/api/material', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ material: material })
    });
    
    const result = await response.json();
    console.log('Material alterado:', result.newMaterial);
}
```
Python
```python
import requests

# Buscar dados
response = requests.get('http://192.168.1.100/api/data')
data = response.json()

print(f"Temperatura: {data['temperature']}°C")
print(f"Status: {data['status']}")

# Trocar material
response = requests.post('http://192.168.1.100/api/material',
    json={'material': 'PETG'})
print(response.json())
```
C# (Desktop/Mobile)
```csharp
using System.Net.Http;
using Newtonsoft.Json;

// Buscar dados
var client = new HttpClient();
var response = await client.GetAsync("http://192.168.1.100/api/data");
var json = await response.Content.ReadAsStringAsync();
var data = JsonConvert.DeserializeObject<SensorData>(json);

Console.WriteLine($"Temperatura: {data.Temperature}°C");
Console.WriteLine($"Status: {data.Status}");
```
curl
```bash
# Buscar dados
curl http://192.168.1.100/api/data

# Trocar material
curl -X POST http://192.168.1.100/api/material \
  -H "Content-Type: application/json" \
  -d '{"material":"PETG"}'

# Atualizar configuração
curl -X POST http://192.168.1.100/api/config \
  -H "Content-Type: application/json" \
  -d '{"tempMin":20,"tempMax":30}'
```
---
🏗️ Arquitetura
Diagrama do Sistema
```
┌─────────────────────────────────────────────────────────────┐
│                      PRINTSENSE v4.0                        │
│                   Arquitetura Completa                      │
└─────────────────────────────────────────────────────────────┘

┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   DHT22     │     │     LDR     │     │  MAX4466    │
│ Temp + Umid │     │     Luz     │     │     Som     │
└──────┬──────┘     └──────┬──────┘     └──────┬──────┘
       │                   │                    │
       └───────────────────┼────────────────────┘
                           │
                     ┌─────▼──────┐
                     │            │
                     │   ESP32-S3 │ ◄──── Encoder (Material)
                     │            │
                     └─────┬──────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
    ┌─────▼─────┐   ┌─────▼─────┐   ┌─────▼─────┐
    │ LCD 20x4  │   │  3 LEDs   │   │  SD Card  │
    │  Display  │   │ 🟢 🟡 🔴  │   │   Logs    │
    └───────────┘   └───────────┘   └───────────┘
                           │
                     ┌─────▼──────┐
                     │    WiFi    │
                     │ WebServer  │
                     │   HTTP:80  │
                     └─────┬──────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
    ┌─────▼─────┐   ┌─────▼─────┐   ┌─────▼─────┐
    │Desktop App│   │Mobile App │   │   Browser │
    │  WPF/C#   │   │ MAUI/C#   │   │   HTML    │
    └───────────┘   └───────────┘   └───────────┘
```
Fluxo de Dados
```
1. Sensores leem dados → ESP32 processa
2. ESP32 calcula status (IDEAL/BOM/RUIM)
3. ESP32 atualiza LCD e LEDs
4. ESP32 salva log no SD Card
5. ESP32 disponibiliza via API REST
6. Clientes consomem API via HTTP
```
Stack Tecnológica
Camada	Tecnologia
Hardware	ESP32-S3, DHT22, LDR, MAX4466
Firmware	C++, Arduino IDE
Comunicação	HTTP REST, JSON, WiFi
Armazenamento	SD Card (FAT32), CSV, JSON
Desktop	C#, WPF, .NET Framework 4.8
Mobile	C#, .NET MAUI, XAML, .NET 8
Emulador	Python 3, HTML5, JavaScript
Machine Learning	ML.NET 5.0 (FastTree)
---
🗺️ Roadmap
✅ v4.0 (Atual)
[x] Firmware ESP32 completo
[x] Persistência em SD Card
[x] Desktop App com 5 abas
[x] Mobile App (base funcional)
[x] Emulador Python V6.0
[x] Machine Learning integrado
🚧 v4.1 (Em Progresso)
[ ] Finalizar abas do Mobile App
[ ] Testes em hardware real
[ ] Calibração automática de sensores
[ ] Notificações push no mobile
[ ] Dashboard web (React/Blazor)
📋 v5.0 (Planejado)
[ ] Modo multi-impressora (vários ESP32)
[ ] Banco de dados (histórico longo)
[ ] Integração WhatsApp/Telegram
[ ] Sistema de login
[ ] API pública documentada
[ ] PCB customizada profissional
🔮 Futuro
[ ] Suporte para mais sensores (CO2, VOC)
[ ] Controle automatizado (relés)
[ ] Integração com OctoPrint
[ ] Análise preditiva de falhas
[ ] Dashboard público (cloud)
---
🤝 Contribuindo
Contribuições são bem-vindas! Siga os passos:
Fork o projeto
Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
Push para a branch (`git push origin feature/AmazingFeature`)
Abra um Pull Request
Diretrizes
Siga o padrão de código existente
Adicione testes quando aplicável
Atualize a documentação
Descreva claramente as mudanças no PR
Áreas que Precisam de Ajuda
📱 Desenvolvimento Mobile (MAUI)
🌐 Frontend Web (React/Vue)
🧪 Testes automatizados
📚 Tradução da documentação
🎨 Design de interface
---
📊 Estatísticas do Projeto
```
📁 Total de Arquivos:        50+
📊 Linhas de Código:         5,000+
🔧 Firmware ESP32:           1,105 linhas
🖥️  Desktop App:             2,000+ linhas
📱 Mobile App:               800+ linhas
🐍 Emuladores:               2,000+ linhas
📚 Documentação:             3,000+ linhas
```
Linguagens
```
C++          ███████████████░░░░░   40%
C#           ████████████░░░░░░░░   35%
Python       ████░░░░░░░░░░░░░░░░   10%
HTML/CSS/JS  ████░░░░░░░░░░░░░░░░   10%
Markdown     █░░░░░░░░░░░░░░░░░░░    5%
```
---
📄 Licença
Este projeto está sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.
```
MIT License

Copyright (c) 2026 Equipe PrintSense

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

[...]
```
---
📞 Contato
Equipe PrintSense
📧 Email: printsense@example.com
🌐 Website: https://printsense.io
💬 Discord: https://discord.gg/printsense
📱 Twitter: @printsense
---
🙏 Agradecimentos
ESP32 Community
Arduino Team
Microsoft .NET Team
Todos os contribuidores open-source
---
<div align="center">
Desenvolvido com ❤️ pela Equipe PrintSense
![Stars](https://img.shields.io/github/stars/seu-usuario/printsense?style=social)
![Forks](https://img.shields.io/github/forks/seu-usuario/printsense?style=social)
![Issues](https://img.shields.io/github/issues/seu-usuario/printsense)
⬆ Voltar ao topo
</div>
