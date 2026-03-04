# 🤖 PrintSense Emulator - Guia Completo

**Desenvolva o frontend do PrintSense SEM precisar do ESP32 físico!**

---

## 🚀 Quick Start (3 Comandos)

```bash
# 1. Clone e entre na pasta
git clone https://github.com/brunohss/PI5.git
cd PI5

# 2. Terminal 1: Iniciar backend
python emulator_server.py

# 3. Terminal 2: Iniciar frontend
cd web
python -m http.server 5500

# 4. Abrir navegador: http://localhost:5500
```

**Pronto! ✅**

---

## 📖 Explicação Detalhada

### **O que cada comando faz:**

#### **Terminal 1: Backend (porta 8080)**
```bash
python emulator_server.py
```
- Simula a API do ESP32
- Responde requisições GET/POST
- Gera dados dos sensores
- Permite testar sem hardware

#### **Terminal 2: Frontend (porta 5500)**
```bash
cd web
python -m http.server 5500
```
- Serve arquivos HTML/CSS/JS
- Permite acessar pelo navegador
- Atualiza automaticamente (F5)

#### **Navegador**
```
http://localhost:5500
```
- Dashboard funcionando
- Gráficos em tempo real
- LEDs reagindo
- Tudo funcional!

---

## 🎮 Como Usar

### **Modificar Dashboard**
```bash
1. Editar: web/index.html
2. Salvar
3. Recarregar navegador (F5)
4. Ver mudanças!
```

### **Criar Página Nova**
```bash
1. Criar: web/config.html
2. Salvar
3. Acessar: http://localhost:5500/config.html
4. Funciona!
```

### **Testar API**
```bash
curl http://localhost:8080/api/data
```

---

## ✅ Pronto para Usar!

Veja [README principal](README.md) para mais detalhes.
