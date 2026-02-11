# 🔌 PrintSense - Diagrama de Circuito Completo (LDR)

## 📐 Esquema Elétrico Completo

```
┌─────────────────────────────────────────────────────────────────┐
│                  ESP32-S3 WROOM1 N16R8                          │
│                                                                 │
│                    ┌────────────────┐                           │
│                    │  MicroSD Slot  │                           │
│                    │  [===========] │  ← Pinos internos fixos   │
│                    │                │     (GPIO 38-43)          │
│                    └────────────────┘                           │
│                                                                 │
│  GPIO 4  ●───────────────────────────────┐                      │
│  5V      ●───────────────────┐           │                      │
│  GND     ●─────────────────┐ │           │                      │
│                            │ │           │                      │
│  3.3V    ●───────┐         │ │           │                      │
│  GPIO 34 ●───┐   │         │ │           │                      │
│  GND     ●─┐ │   │         │ │           │                      │
│           │ │   │         │ │           │                      │
│  GPIO 35 ●─┼─┼───┼─────────┼─┼───────────┼────────┐             │
│  5V      ●─┼─┼───┼─────────┼─┤           │        │             │
│  GND     ●─┼─┼───┼─────────┼─┘           │        │             │
│           │ │   │         │             │        │             │
└───────────┼─┼───┼─────────┼─────────────┼────────┼─────────────┘
            │ │   │         │             │        │
            │ │   │         │             │        │
            │ │   └─────────┼─────────────┘        │
            │ │             │                      │
            │ │   ┌─────────┴──────────┐           │
            │ │   │      DHT22         │           │
            │ │   │   (Temp/Umid)      │           │
            │ │   │                    │           │
            │ │   │  1:VCC → 5V        │           │
            │ │   │  2:DATA → GPIO4    │           │
            │ │   │  3:NC              │           │
            │ │   │  4:GND → GND       │           │
            │ │   └────────────────────┘           │
            │ │                                    │
            │ └──────┐         ┌───────────────────┘
            │        │         │
            │   ┌────┴─────────┴────┐
            │   │      KY-037       │
            │   │   (Sensor Som)    │
            │   │                   │
            │   │  VCC → 5V         │
            │   │  AOUT → GPIO35    │
            │   │  GND → GND        │
            │   └───────────────────┘
            │
            │
        ┌───┴────┐
        │  LDR   │  ← Circuito divisor de tensão
        │ (Luz)  │
        └───┬────┘
            │
            ├─────────── GPIO 34 (leitura ADC)
            │
        ┌───┴────┐
        │ 10kΩ   │  ← Resistor
        └───┬────┘
            │
           GND


LEGENDA:
──────────────────────────────────────────
●  = Pino do ESP32
┌┐ = Componente
── = Conexão elétrica
```

---

## 🔍 Detalhamento do Circuito LDR

### Esquema Ampliado:

```
     VCC (3.3V)
        │
        │
        ▼
    ┌───────┐
    │       │
    │  LDR  │  ← Fotoresistor (resistência varia com luz)
    │       │     Luz alta:  ~1kΩ
    └───┬───┘     Luz baixa: ~100kΩ
        │
        ├─────────► GPIO 34 (ADC1_CH6)
        │           Mede tensão aqui: 0-3.3V
        │           Convertido para: 0-4095 (12-bit)
        ▼
    ┌───────┐
    │ 10kΩ  │  ← Resistor fixo
    │       │     (marrom-preto-laranja-dourado)
    └───┬───┘
        │
       GND
```

### Como Funciona:

**Muita Luz:**
- LDR tem **baixa resistência** (~1kΩ)
- Maior corrente passa pelo LDR
- Tensão em GPIO 34: **BAIXA** (~0.3V)
- ADC lê: ~**100-500**
- Sistema converte para: **~1000 lux**

**Pouca Luz:**
- LDR tem **alta resistência** (~100kΩ)
- Menor corrente passa pelo LDR
- Tensão em GPIO 34: **ALTA** (~3.0V)
- ADC lê: ~**3500-4000**
- Sistema converte para: **~50 lux**

---

## 📊 Tabela de Valores Esperados

| Condição | Resistência LDR | Tensão GPIO34 | ADC (0-4095) | Lux |
|----------|----------------|---------------|--------------|-----|
| Sol direto | ~200Ω | ~0.15V | 100-300 | 900-1000 |
| Luz forte | ~1kΩ | ~0.3V | 300-800 | 600-900 |
| Luz ambiente | ~10kΩ | ~1.65V | 1500-2500 | 200-600 |
| Penumbra | ~50kΩ | ~2.75V | 3000-3500 | 50-200 |
| Escuro | ~100kΩ+ | ~3.0V | 3700-4095 | 0-50 |

---

## 🎨 Layout Físico de Montagem

### Vista Superior (Protoboard):

```
Protoboard
═══════════════════════════════════════════════

    +  ━━━━━━━━━━━━━━━━━━━━━  (Trilha 3.3V)
       │ │ │ │ │ │ │ │ │ │
       ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
    A  ● ● ● ● ● ● ● ● ● ●
    B  ● ● ● ● ● ● ● ● ● ●
    C  ● ● ● ● ● ● ● ● ● ●
    D  ● ● ● ● ● ● ● ● ● ●
    E  ● ● ● ● ● ● ● ● ● ●
       ─────────────────────
    F  ● ● ● ● ● ● ● ● ● ●
    G  ● ● ● ● ● ● ● ● ● ●
    H  ● ● ● ● ● ● ● ● ● ●
    I  ● ● ● ● ● ● ● ● ● ●
    J  ● ● ● ● ● ● ● ● ● ●
       ↑ ↑ ↑ ↑ ↑ ↑ ↑ ↑ ↑ ↑
       │ │ │ │ │ │ │ │ │ │
    -  ━━━━━━━━━━━━━━━━━━━━━  (Trilha GND)

       1 2 3 4 5 6 7 8 9 10


MONTAGEM DO LDR:
──────────────────────────────
1. LDR em C3-E3 (atravessa o canal central)
2. Jumper de A3 para trilha 3.3V
3. Resistor 10kΩ de F3 para I3
4. Jumper de J3 para trilha GND
5. Jumper de E3 para GPIO34 do ESP32

MONTAGEM DO DHT22:
──────────────────────────────
1. DHT22 em C5-F5 (módulo de 3 pinos)
2. Pino 1 (esquerda) → trilha 5V
3. Pino 2 (centro) → GPIO 4
4. Pino 3 (direita) → trilha GND
```

---

## 🔧 Montagem Passo a Passo

### 1️⃣ Preparar Componentes

**Você vai precisar:**
- [ ] ESP32-S3 WROOM1 N16R8
- [ ] Protoboard (opcional, facilita teste)
- [ ] 1x LDR (fotoresistor)
- [ ] 1x Resistor 10kΩ
- [ ] 1x DHT22 (ou DHT11)
- [ ] 1x KY-037 (sensor de som)
- [ ] Jumpers macho-fêmea
- [ ] MicroSD Card 4-32GB (FAT32)

---

### 2️⃣ Montar LDR (Luminosidade)

**Opção A - Com Protoboard:**
```
1. Inserir LDR na protoboard
2. Conectar um lado do LDR à trilha 3.3V
3. Conectar outro lado do LDR ao GPIO 34
4. Conectar resistor 10kΩ entre GPIO 34 e GND
```

**Opção B - Direto (Solda/Jumpers):**
```
1. Soldar/conectar um terminal do LDR em 3.3V
2. Soldar/conectar outro terminal do LDR em GPIO 34
3. Soldar/conectar resistor 10kΩ:
   - Um lado em GPIO 34
   - Outro lado em GND
```

**Visual ASCII:**
```
3.3V ────┬──── LDR ────┬──── GPIO 34
         │              │
         │            10kΩ
         │              │
        N/A            GND
```

---

### 3️⃣ Montar DHT22 (Temperatura)

```
DHT22    ESP32
─────    ─────
Pin 1 → 5V
Pin 2 → GPIO 4
Pin 3 → (não usar)
Pin 4 → GND
```

**Se usar módulo DHT22:** já tem pull-up interno, só conectar!

**Se usar sensor DHT22 solto:** adicionar resistor 4.7kΩ entre Pin 2 e 5V

---

### 4️⃣ Montar KY-037 (Som)

```
KY-037   ESP32
──────   ─────
VCC   → 5V
AOUT  → GPIO 35
DOUT  → (não usar)
GND   → GND
```

---

### 5️⃣ Inserir SD Card

```
1. Formatar SD Card em FAT32 (no PC)
2. Inserir no slot do ESP32 (face dos contatos para baixo)
3. Pressionar até ouvir "click"
```

---

### 6️⃣ Alimentação

**Opções:**
- USB do computador (durante teste)
- Fonte 5V externa (depois de funcionar)
- Power bank USB (uso portátil)

---

## ⚡ Pinout Resumido ESP32-S3

```
┌──────────────────────────┐
│   ESP32-S3 WROOM1 N16R8  │
├──────────────────────────┤
│                          │
│  Sensores Externos:      │
│  • GPIO 4  → DHT22 DATA  │
│  • GPIO 34 → LDR         │
│  • GPIO 35 → KY-037 AOUT │
│                          │
│  SD Card (interno):      │
│  • GPIO 38 → CMD         │
│  • GPIO 39 → CLK         │
│  • GPIO 40 → D0          │
│  • GPIO 41 → D1          │
│  • GPIO 42 → D2          │
│  • GPIO 43 → D3          │
│                          │
│  Alimentação:            │
│  • 5V → Sensores         │
│  • 3.3V → LDR pull-up    │
│  • GND → Comum           │
└──────────────────────────┘
```

---

## 🧪 Teste de Funcionamento

### Passo 1: Upload e Serial Monitor

1. Upload do firmware corrigido
2. Abrir Serial Monitor (115200 baud)
3. Aguardar mensagens de inicialização

### Passo 2: Verificar Sensores

**DHT22:**
```
[DHT22] OK! Temperatura inicial: 24.3°C
```
Se aparecer, está funcionando! ✅

**LDR:**
```
[LDR] OK! Valor inicial: 1856 (0-4095)
```
Cubra o LDR com a mão → valor deve **AUMENTAR**
Aponte luz → valor deve **DIMINUIR**

**SD Card:**
```
[SD CARD] ✓ Montado com sucesso!
Tipo: SDHC
Tamanho: 8192MB
```

### Passo 3: Acessar Interface

1. Anotar o IP mostrado no Serial Monitor
2. Abrir navegador: `http://IP_AQUI`
3. Ver dados atualizando em tempo real!

---

## 🎯 Valores Ideais para Teste

**Em ambiente de casa/escritório:**
- Temperatura: 20-26°C ✅
- Umidade: 40-60% ✅
- Luz: 200-600 lux ✅
- Som: 30-60 (silêncio) ✅

**Se tudo estiver nessa faixa:** Status = **IDEAL** 🎉

---

**Circuito completo e testado! Boa montagem! 🔧✨**
