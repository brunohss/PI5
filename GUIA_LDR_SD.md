# 🔧 PrintSense - Guia de Correção (LDR + SD Card)

## ✅ Problemas Corrigidos

1. ✅ **BH1750 removido** → substituído por **LDR**
2. ✅ **SD Card configurado corretamente** para ESP32-S3 WROOM1 N16R8
3. ✅ **Biblioteca SD_MMC** ao invés de SD (mais rápida e usa pinos fixos)

---

## 🔌 CIRCUITO DO LDR

### O que você precisa:
- 1x LDR (fotoresistor) - qualquer modelo
- 1x Resistor 10kΩ (marrom-preto-laranja)

### Montagem:

```
      3.3V
        │
        │
      [LDR]  ← Fotoresistor
        │
        ├──────── GPIO 34 (leitura ADC)
        │
    [10kΩ]  ← Resistor
        │
       GND
```

**Explicação:**
- **LDR em cima** (entre 3.3V e GPIO34)
- **Resistor 10kΩ embaixo** (entre GPIO34 e GND)
- Quando há **mais luz**, LDR tem **menos resistência** → tensão no GPIO34 **diminui** → ADC lê valor **menor**
- Quando há **menos luz**, LDR tem **mais resistência** → tensão no GPIO34 **aumenta** → ADC lê valor **maior**

### Valores Típicos:
- **Muita luz** (sol direto): ADC lê ~100-500
- **Luz ambiente** (sala iluminada): ADC lê ~1000-2000  
- **Pouca luz** (sala escura): ADC lê ~3000-4000
- **Escuro total**: ADC lê ~4095

---

## 💾 SD CARD - ESP32-S3 WROOM1 N16R8

### ⚠️ IMPORTANTE: Pinos FIXOS

O ESP32-S3 tem pinos **FIXOS** para SD Card via SDMMC:

```
Função    GPIO    Descrição
──────────────────────────────
CLK       39      Clock
CMD       38      Command
D0        40      Data 0 (modo 1-bit)
D1        41      Data 1 (modo 4-bit)
D2        42      Data 2 (modo 4-bit)  
D3        43      Data 3 / CS (modo 4-bit)
```

### 📌 Pinout Completo da Sua Placa

```
                ┌─────────────────────┐
                │  ESP32-S3 WROOM1   │
                │      N16R8          │
                │                     │
    [MicroSD]   │  ┌──────────────┐   │
    ┌─────────┐ │  │  SD Slot     │   │
    │ [=====] │◄┼──┤ Embutido     │   │
    └─────────┘ │  └──────────────┘   │
                │                     │
                │  Pinos SDMMC:       │
                │  • CLK  → GPIO 39   │
                │  • CMD  → GPIO 38   │
                │  • D0   → GPIO 40   │
                │  • D1   → GPIO 41   │
                │  • D2   → GPIO 42   │
                │  • D3   → GPIO 43   │
                └─────────────────────┘
```

### 🔍 Verificações:

1. **Cartão inserido?**
   - Inserir até ouvir "click"
   - Face dos contatos para baixo

2. **Cartão formatado?**
   - Deve estar em **FAT32**
   - No PC: clique direito → Formatar → FAT32

3. **Tamanho do cartão:**
   - ✅ Funciona: 4GB, 8GB, 16GB, 32GB
   - ⚠️ Pode dar problema: 64GB+ (precisa FAT32, não exFAT)

4. **Velocidade:**
   - Classe 10 recomendada
   - Marcas: SanDisk, Kingston, Samsung

---

## 🔧 CONEXÕES FINAIS

### DHT22 (Temperatura e Umidade)
```
DHT22 Pino 1 (VCC)  → 5V
DHT22 Pino 2 (DATA) → GPIO 4
DHT22 Pino 3 (NC)   → (não conectar)
DHT22 Pino 4 (GND)  → GND
```

**Nota:** Alguns módulos DHT22 têm resistor pull-up interno. Se o seu não tiver, adicione um resistor de 4.7kΩ entre DATA e VCC.

### LDR (Luminosidade)
```
3.3V → LDR → GPIO 34 → Resistor 10kΩ → GND
```

### KY-037 (Som)
```
KY-037 VCC  → 5V
KY-037 AOUT → GPIO 35
KY-037 GND  → GND
```

### SD Card
**JÁ EMBUTIDO NA PLACA!** Pinos internos (GPIO 38-43).

---

## 📝 CONFIGURAÇÃO NO CÓDIGO

### 1. WiFi

Edite no início do código:

```cpp
// Conectar à sua rede WiFi
const char* ssid = "NOME_DA_SUA_REDE";
const char* password = "SUA_SENHA_WIFI";
```

**OU** use modo AP (cria rede própria):

```cpp
// Comente as linhas acima e descomente:
// const char* ssid = "PrintSense";
// const char* password = "printsense123";
```

### 2. Pinos (já configurado)

```cpp
#define DHT_PIN 4      // DHT22
#define LDR_PIN 34     // LDR
#define SOUND_PIN 35   // Som
// SD Card: pinos fixos (38-43)
```

### 3. Calibração do LDR (opcional)

Se quiser ajustar a conversão lux:

```cpp
int convertLDRtoLux(int rawValue) {
  // Ajuste esses valores conforme seu LDR
  // Teste com luz solar, luz ambiente e escuro
  int lux = map(rawValue, 0, 4095, 1000, 0);
  return lux;
}
```

**Como calibrar:**
1. Abra Serial Monitor (115200 baud)
2. Aponte o LDR para diferentes fontes de luz
3. Anote os valores `raw` que aparecem
4. Ajuste a fórmula conforme necessário

Exemplo:
- Sol direto: raw=200 → quer 1000 lux
- Escuro: raw=3800 → quer 10 lux

```cpp
int lux = map(rawValue, 200, 3800, 1000, 10);
```

---

## 🐛 TROUBLESHOOTING

### Erro: "Erro ao iniciar SD Card!"

**Causas possíveis:**

1. **Cartão não inserido ou mal inserido**
   - Solução: Reinserir até ouvir click

2. **Cartão não formatado em FAT32**
   - Solução: Formatar no PC
   ```
   Windows: clique direito → Formatar → FAT32
   Mac: Utilitário de Disco → Apagar → MS-DOS (FAT)
   Linux: sudo mkfs.vfat -F 32 /dev/sdX
   ```

3. **Cartão corrompido ou defeituoso**
   - Solução: Testar cartão no PC
   - Se não aparece no PC, cartão está ruim

4. **Cartão > 32GB em exFAT**
   - Solução: Forçar FAT32 (use ferramenta como FAT32 Format)

### Erro: "DHT22 retornando NaN"

**Causas:**

1. **Sensor não conectado**
   - Verificar fios no GPIO 4, VCC e GND

2. **Falta de pull-up**
   - Adicionar resistor 4.7kΩ entre DATA e VCC

3. **Sensor queimado**
   - Testar com outro sensor DHT22/DHT11

4. **Sensor precisa estabilizar**
   - Esperar 10-15 segundos após ligar

### Erro: "WiFi não conecta"

1. **SSID/senha errados**
   - Verificar digitação (case-sensitive!)

2. **Rede 5GHz**
   - ESP32 só funciona em 2.4GHz
   - Trocar para rede 2.4GHz

3. **Sinal fraco**
   - Aproximar roteador do ESP32

**Fallback:** Sistema entra automaticamente em modo AP se não conectar

---

## ✅ CHECKLIST DE MONTAGEM

Antes de ligar:

- [ ] DHT22 conectado em GPIO 4 (+ VCC 5V e GND)
- [ ] LDR conectado: 3.3V → LDR → GPIO34 → 10kΩ → GND
- [ ] Sensor som em GPIO 35 (+ VCC 5V e GND)
- [ ] SD Card inserido e formatado FAT32
- [ ] WiFi configurado no código
- [ ] Firmware compilado sem erros
- [ ] USB conectado para ver Serial Monitor

---

## 📊 ESTRUTURA DO SD CARD

Após primeira execução, o sistema cria:

```
/
├── web/
│   ├── index.html      ← Copiar interface aqui
│   ├── style.css       ← Copiar CSS aqui
│   └── script.js       ← Copiar JS aqui
├── logs/
│   └── YYYYMMDD.csv    ← Logs criados automaticamente
└── jobs/
    └── (vazio por enquanto)
```

**IMPORTANTE:** Copie os arquivos `index.html`, `style.css` e `script.js` para a pasta `/web/` do SD Card MANUALMENTE (via PC).

---

## 🎯 TESTE RÁPIDO

1. **Upload do firmware**
2. **Abrir Serial Monitor (115200 baud)**
3. **Verificar mensagens:**

```
✓ [DHT22] OK! Temperatura inicial: 23.5°C
✓ [LDR] OK! Valor inicial: 1523 (0-4095)
✓ [SD CARD] ✓ Montado com sucesso!
✓ [WiFi] ✓ Conectado! IP: 192.168.15.50
✓ [WebServer] ✓ Iniciado!
```

4. **Abrir navegador:** http://IP_MOSTRADO
5. **Verificar dados atualizando**

---

## 💡 DICAS

### LDR muito sensível?
Troque resistor 10kΩ por:
- **100kΩ** = menos sensível (melhor para ambientes claros)
- **1kΩ** = mais sensível (melhor para ambientes escuros)

### Quer usar BH1750 também?
Você pode adicionar **ambos** sensores e comparar:
- LDR em GPIO 34 (analógico)
- BH1750 em I2C (pinos 21/22)

### SD Card lento?
Cartões Classe 10 são mais rápidos. Evite cartões genéricos baratos.

---

## 📞 SUPORTE

Se tiver problemas:
1. Poste a saída do Serial Monitor
2. Descreva sua montagem
3. Foto do circuito ajuda!

**Tudo pronto! Boa sorte! 🚀**
