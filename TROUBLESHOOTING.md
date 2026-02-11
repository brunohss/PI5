# 🔧 PrintSense - Guia de Troubleshooting

## 🌡️ Problema: DHT22 retornando NaN

### Sintoma:
```
⚠️  DHT22 retornando NaN - verifique sensor
```

### ✅ Solução Passo a Passo:

#### 1️⃣ ADICIONAR RESISTOR PULL-UP (80% dos casos!)

**O DHT22 REQUER um resistor entre VCC e DATA!**

```
Esquema de Ligação CORRETO:

        VCC (5V ou 3.3V)
         │
         ├──────────────┐
         │              │
        ┌┴┐         ┌───┴───┐
        │R│ 4.7kΩ   │       │
        │ │ ou 10kΩ │ DHT22 │
        └┬┘         │       │
         │          └───┬───┘
         │              │
         └──────────────┼──── GPIO 4 (ESP32)
                        │
                       GND
```

**Resistores que funcionam:**
- ✅ 4.7kΩ (ideal)
- ✅ 10kΩ (também funciona)
- ⚠️ 1kΩ (muito baixo, evitar)
- ❌ SEM resistor = NÃO FUNCIONA

#### 2️⃣ Verificar Conexões

**DHT22 de 4 pinos:**
```
Vista frontal (grade voltada para você):
┌─────────┐
│ 1 2 3 4 │
└─────────┘

Pino 1: VCC  → 5V ou 3.3V (ESP32)
Pino 2: DATA → GPIO 4 + Resistor para VCC
Pino 3: NC   → Não conectar
Pino 4: GND  → GND
```

**DHT22 de 3 pinos (módulo):**
```
┌─────────┐
│ - S + │
└─────────┘

- (GND)   → GND
S (Signal)→ GPIO 4
+ (VCC)   → 5V ou 3.3V
```

*Módulos geralmente JÁ TÊM resistor pull-up embutido!*

#### 3️⃣ Testar com Código de Debug

Faça upload do **Test_DHT22.ino** (incluído no projeto):

```bash
# No Arduino IDE:
1. Abrir Test_DHT22.ino
2. Upload
3. Abrir Serial Monitor (115200 baud)
4. Ver se retorna temperatura/umidade
```

Se continuar NaN:
- ❌ Sensor pode estar queimado
- ❌ Conexões erradas
- ❌ GPIO errado

#### 4️⃣ Testar GPIO Alternativo

Se GPIO 4 não funcionar, tente outro pino:

```cpp
#define DHT_PIN 15  // Em vez de 4
// ou
#define DHT_PIN 16
// ou
#define DHT_PIN 17
```

#### 5️⃣ Verificar Alimentação

DHT22 precisa de alimentação estável:

**Opções:**
- 3.3V ✅ (mais estável no ESP32)
- 5V ✅ (também funciona)

**Teste ambas:**
```cpp
// Versão 1: VCC → 3.3V do ESP32
// Versão 2: VCC → 5V do ESP32 ou fonte externa
```

Se usar fonte externa 5V:
- ⚠️ Conecte GND da fonte com GND do ESP32!

#### 6️⃣ Aumentar Delay de Inicialização

No `setup()`:

```cpp
void setup() {
  Serial.begin(115200);
  delay(2000);  // Aguardar 2 segundos
  
  pinMode(DHT_PIN, INPUT_PULLUP);  // Pull-up interno
  delay(100);
  
  dht.begin();
  delay(2000);  // DHT precisa de tempo!
  
  // Resto do código...
}
```

#### 7️⃣ Verificar se Sensor está Queimado

**Teste com multímetro:**
- Medir resistência entre VCC e GND
- Deve ser alguns kΩ (não infinito, não zero)
- Se 0Ω → Curto-circuito (queimado)
- Se ∞Ω → Circuito aberto (queimado)

**Teste em outro Arduino/ESP:**
- Se funcionar lá → problema no ESP32 atual
- Se não funcionar → sensor queimado

---

## 💾 Problema: SD Card não disponível

### Sintoma:
```
[LOG] SD Card não disponível - pulando log
```

### ✅ Solução Passo a Passo:

#### 1️⃣ DESCOBRIR PINO CS CORRETO

**Use o Test_SDCard.ino:**

```bash
1. Upload do Test_SDCard.ino
2. Abrir Serial Monitor
3. Script vai testar pinos: 5, 10, 13, 15, 21, 22
4. Anotar qual GPIO funcionou
5. Atualizar no código principal
```

Exemplo de saída:
```
Testando GPIO 5... ❌ Falhou
Testando GPIO 10... ✅ ENCONTRADO!
>>> Pino CS correto: GPIO 10
```

Então no código:
```cpp
#define SD_CS 10  // Usar valor descoberto
```

#### 2️⃣ Verificar Pinout da Placa

**ESP32-S3 WROOM com SD integrado:**

Consulte o datasheet ou PCB da sua placa:
- Procure por "SD_CS" ou "SD_D3"
- Normalmente é GPIO 10 ou GPIO 5

**Pinos SPI do SD:**
```
MISO (D0) → GPIO 37 (típico)
MOSI (CMD) → GPIO 35 (típico)
CLK       → GPIO 36 (típico)
CS (D3)   → GPIO 10 (típico) ← ESTE é o importante!
```

#### 3️⃣ Formatar Cartão em FAT32

**MUITO IMPORTANTE!**

❌ **NÃO funciona:**
- exFAT
- NTFS
- Ext4

✅ **Funciona:**
- FAT32

**Windows:**
```
1. Inserir cartão
2. Botão direito → Formatar
3. Sistema: FAT32
4. Iniciar
```

**Mac:**
```
1. Utilitário de Disco
2. Apagar
3. Formato: MS-DOS (FAT)
4. Esquema: Master Boot Record
```

**Linux:**
```bash
sudo mkfs.vfat -F 32 /dev/sdX1
```

#### 4️⃣ Testar Cartão em Outro Dispositivo

- Insira em PC/celular/câmera
- Copie arquivos
- Se não funcionar → cartão defeituoso

**Cartões problemáticos:**
- ❌ Muito antigos (< Classe 4)
- ❌ Muito grandes (> 32GB pode dar problema)
- ❌ Falsificados (comum em cartões baratos)

**Cartões recomendados:**
- ✅ SanDisk 8-16GB Classe 10
- ✅ Kingston 8-16GB Classe 10
- ✅ Samsung EVO 8-16GB

#### 5️⃣ Verificar Inserção do Cartão

- Cartão deve entrar completamente
- Deve fazer "click"
- Se solto → não fará contato

**Teste:**
- Remover e reinserir
- Pressionar até ouvir/sentir click
- Resetar ESP32

#### 6️⃣ Definir Pinos SPI Manualmente

Se usar SD externo (não integrado):

```cpp
#include <SPI.h>

#define SD_MISO 37
#define SD_MOSI 35
#define SD_CLK  36
#define SD_CS   10

void setup() {
  // Inicializar SPI com pinos customizados
  SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("SD falhou!");
  }
}
```

#### 7️⃣ Verificar Velocidade do Clock

SD Card pode não funcionar com clock muito rápido:

```cpp
// Tentar com clock reduzido
if (!SD.begin(SD_CS, SPI, 4000000)) {  // 4MHz em vez de 25MHz
  Serial.println("SD falhou!");
}
```

---

## 🔍 Verificação Rápida (Checklist)

### DHT22:
- [ ] Resistor 4.7kΩ entre VCC e DATA?
- [ ] Conexões firmes?
- [ ] VCC = 3.3V ou 5V?
- [ ] DATA = GPIO 4?
- [ ] GND conectado?
- [ ] Delay de 2s após dht.begin()?

### SD Card:
- [ ] Cartão inserido corretamente?
- [ ] Cartão formatado em FAT32?
- [ ] Pino CS correto? (testado com Test_SDCard.ino?)
- [ ] Cartão funciona em PC?
- [ ] Tamanho ≤ 32GB?

---

## 📞 Se Nada Funcionar:

### DHT22:
1. **Comprar sensor novo** (R$ 15-25)
2. **Testar DHT11** temporariamente (mais barato, menos preciso)
3. **Usar BME280** (melhor, mas I2C)

### SD Card:
1. **Comprar cartão novo** (marcas confiáveis)
2. **Desabilitar logs** temporariamente:
   ```cpp
   // Comentar linha de log no loop:
   // logToSD();
   ```
3. **Usar SPIFFS/LittleFS** (memória interna, limitado)

---

## 🎓 Dicas de Hardware

### Protoboard vs Solda:
- Protoboard: conexões podem ficar soltas
- **Solução:** Soldadura = mais confiável

### Cabos Jumper:
- Cabos longos (>30cm) podem ter problemas
- **Solução:** Use cabos curtos (~15cm)

### Fonte de Alimentação:
- USB do PC pode não fornecer corrente suficiente
- **Solução:** Use fonte 5V 2A dedicada

---

## 📊 Valores Esperados

**DHT22 funcionando:**
```
✓ Temp: 22.5°C | Umid: 45.2% | Luz: 350 lux | Som: 120
```

**DHT22 com problema:**
```
⚠️  DHT22 retornando NaN - verifique sensor
✓ Temp: 0.0°C | Umid: 0.0% | Luz: 350 lux | Som: 120
```

**SD Card funcionando:**
```
[LOG] ✓ Salvo: 2024-02-05 17:30:00,22.5,45.2,350,120,IDEAL
```

**SD Card com problema:**
```
[LOG] SD Card não disponível - pulando log
```

---

**Boa sorte! 🚀**
