# ⚡ PrintSense - Guia de Início Rápido

## 🎯 Objetivo
Em 30 minutos você terá seu monitor ambiental funcionando!

---

## 📦 Checklist Pré-Instalação

- [ ] ESP32-S3 WROOM1 (com slot MicroSD)
- [ ] Sensor DHT22
- [ ] Sensor BH1750
- [ ] Sensor KY-037 (som)
- [ ] MicroSD Card (4GB+, FAT32)
- [ ] Cabos jumper
- [ ] Cabo USB para programação
- [ ] Computador com Arduino IDE ou PlatformIO

---

## 🔧 Passo 1: Montagem do Hardware (10 min)

### Conexões Essenciais

```
DHT22:
  VCC  → 5V (ESP32)
  DATA → GPIO 4
  GND  → GND

BH1750:
  VCC → 3.3V
  SDA → GPIO 21
  SCL → GPIO 22
  GND → GND

KY-037:
  VCC  → 5V
  AOUT → GPIO 34
  GND  → GND
```

**Dica:** Use fita isolante ou termo-retrátil para proteger conexões.

---

## 💾 Passo 2: Preparar MicroSD (5 min)

1. **Formatar** em FAT32
2. Criar pastas:
   ```
   /web/
   /logs/
   /jobs/
   ```
3. Copiar para `/web/`:
   - `index.html`
   - `style.css`
   - `script.js`

---

## 💻 Passo 3: Upload do Firmware (10 min)

### Opção A: Arduino IDE

1. Instalar placas ESP32:
   - **File → Preferences**
   - Adicionar URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - **Tools → Board → Boards Manager** → Buscar "ESP32" → Install

2. Instalar bibliotecas:
   - **Sketch → Include Library → Manage Libraries**
   - Instalar: `DHT sensor library`, `BH1750`, `ArduinoJson`

3. Configurar placa:
   - **Tools → Board:** "ESP32S3 Dev Module"
   - **Flash Size:** "8MB"
   - **Partition Scheme:** "Default 4MB"

4. Abrir `PrintSense_ESP32.ino`

5. **Ajustar configurações WiFi** (linhas 15-17):
   ```cpp
   const char* ssid = "PrintSense";
   const char* password = "printsense123";
   ```

6. **Upload** (Ctrl+U)

### Opção B: PlatformIO

```bash
# Clonar ou criar projeto
mkdir PrintSense && cd PrintSense

# Copiar platformio.ini e código
cp platformio.ini .
cp PrintSense_ESP32.ino src/main.cpp

# Upload
pio run --target upload
pio device monitor  # Ver Serial Monitor
```

---

## 🌐 Passo 4: Acesso à Interface (5 min)

1. **Resetar** o ESP32
2. **Conectar ao WiFi:**
   - SSID: `PrintSense`
   - Senha: `printsense123`
3. **Abrir navegador:**
   - URL: `http://192.168.4.1`
4. **Aguardar** carregamento da interface

**Pronto!** 🎉 Você já pode monitorar as condições ambientais.

---

## 🧪 Teste Rápido

1. **Verificar sensores:**
   - Temperatura deve aparecer valor real
   - Umidade entre 30-70% (típico)
   - Luminosidade varia com luz ambiente
   - Som aumenta ao fazer barulho próximo

2. **Testar seleção de material:**
   - Trocar para "ABS"
   - Status deve mudar conforme ambiente

3. **Verificar logs:**
   - Esperar 1 minuto
   - Clicar "Ver Logs Salvos"
   - Deve aparecer arquivo do dia (YYYYMMDD.csv)

---

## 🔍 Troubleshooting Rápido

### ❌ "Sem WiFi PrintSense"
→ Verificar upload bem-sucedido
→ Resetar ESP32

### ❌ "Temperatura/Umidade em --"
→ Conferir conexões DHT22
→ Pino correto (GPIO 4)

### ❌ "Página não carrega"
→ Verificar arquivos no SD (/web/)
→ Inserir SD corretamente

### ❌ "Logs não aparecem"
→ Aguardar 1 minuto (intervalo de log)
→ Verificar pasta /logs/ existe

---

## 📊 Uso Diário

### Antes de Imprimir:
1. Abrir interface: `http://192.168.4.1`
2. Selecionar material (ex: PLA)
3. Verificar status:
   - ✅ **IDEAL** → Pode imprimir!
   - ⚠️ **BOM** → Avaliar se fatores críticos
   - ❌ **RUIM** → Ajustar ambiente antes

### Análise de Defeitos:
1. Anotar horário da impressão
2. Após impressão, baixar log do dia
3. Abrir CSV no Excel/Sheets
4. Filtrar pelo horário
5. Comparar com faixas ideais

---

## 🎯 Próximos Passos

- [ ] Testar com todos os materiais (PLA/PETG/ABS/Resina)
- [ ] Criar histórico de 1 semana
- [ ] Correlacionar defeitos com ambiente
- [ ] Ajustar ambiente conforme necessário
- [ ] (Opcional) Adicionar sensor de vibração

---

## 📞 Precisa de Ajuda?

**Serial Monitor (115200 baud):**
```
Tools → Serial Monitor
```
O ESP32 imprime mensagens de debug úteis.

**Principais mensagens:**
- ✅ "WiFi conectado" → Tudo OK
- ⚠️ "Erro ao iniciar SD" → Verificar cartão
- ⚠️ "Erro ao iniciar BH1750" → Verificar I2C

---

## 🎓 Dicas de Uso Avançado

### 1. Criar "Receitas" de Ambiente

Após algumas semanas, você terá dados para criar receitas:
```
PLA Premium:
- Temp: 22-24°C (sweet spot)
- Umid: 45-50% (menos stringing)
- Luz: < 300 lux
```

### 2. Identificar Padrões

Exemplos:
- "Warping sempre ocorre quando temp < 20°C"
- "Bolhas em PETG quando umid > 55%"
- "Resina cura melhor entre 22-23°C"

### 3. Otimizar Setup

Use dados para:
- Definir horários ideais (menos variação térmica)
- Justificar compra de climatizador
- Configurar dry box automática

---

**Boas impressões! 🖨️✨**

---

_PrintSense v1.0 - Monitor Ambiental para Impressão 3D_
