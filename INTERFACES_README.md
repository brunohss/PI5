# 🎨 PrintSense - Interfaces Disponíveis

Este projeto inclui **DUAS interfaces web** e uma **página de apresentação** profissional!

---

## 📄 Arquivos Disponíveis

### 1. 📊 **presentation.html** (NOVO!)
**Página de apresentação para mostrar aos amigos**

Uma landing page moderna e impactante para apresentar o projeto PrintSense.

**Características:**
- ✨ Design futurista com animações
- 🎯 Layout profissional tipo "tech startup"
- 📱 100% responsivo
- 🎨 Tema cyberpunk/tech (azul neon + roxo)
- ⚡ Animações suaves ao scroll
- 📊 Tabelas comparativas de materiais
- 💰 Seção de preço e CTA
- 🔥 Efeito grid animado no background

**Uso:**
- Abra diretamente no navegador
- Ideal para apresentar o projeto
- Pode hospedar online ou mostrar localmente

**Fontes usadas:**
- Orbitron (títulos - estilo tech/gaming)
- Fira Code (corpo - fonte mono profissional)

---

### 2. 🖥️ **index.html** (Original)
**Interface funcional do dashboard - Versão 1**

A interface web original que roda no ESP32.

**Características:**
- 🎨 Design colorido com gradientes
- 📊 Cards de sensores com animações
- 📈 Sistema de status visual
- 💡 Dicas e recomendações
- 📱 Responsivo

**Tema:** Colorido e amigável

---

### 3. 🌙 **index_modern.html** (NOVO!)
**Interface funcional moderna - Versão 2**

Dashboard moderno inspirado em projetos IoT profissionais.

**Características:**
- 🌑 Tema dark mode (azul escuro)
- 💎 Design minimalista e elegante
- ⚡ Layout tipo "tech dashboard"
- 🎯 Ênfase em legibilidade
- 📊 Cards com hover effects suaves
- 🔵 Accent color: cyan (#06b6d4)
- 📱 Totalmente responsivo

**Fontes usadas:**
- Space Grotesk (títulos - moderna e tech)
- JetBrains Mono (números/código)

**Diferenças da versão original:**
- Tema escuro por padrão
- Visual mais "sério" e profissional
- Menos gradientes, mais flat design
- Tipografia mono para valores técnicos
- Header sticky (fixo no topo)
- Status banner mais destacado

---

## 🎯 Qual Interface Usar?

### Para ESP32 (WebServer):
Você pode escolher **qualquer uma das duas**:

#### Opção A: Original (index.html)
- Mais colorida
- Visual "amigável"
- Boa para ambientes casuais

#### Opção B: Moderna (index_modern.html)
- Visual profissional
- Estilo "lab/farm" industrial
- Melhor para ambientes sérios
- Inspirada em projetos como estufa de filamentos

**Como usar:**
1. Renomeie `index_modern.html` para `index.html`
2. Copie para `/web/` no SD Card
3. O ESP32 vai servir automaticamente

### Para Apresentação:
Use **presentation.html**:
- Abra direto no navegador
- Mostre aos amigos/colegas
- Perfeita para pitch do projeto

---

## 🔄 Como Trocar de Interface

### No SD Card:
```bash
# Backup da original
mv index.html index_original.html

# Usar a moderna
cp index_modern.html index.html

# Copiar para SD Card
cp index.html /media/SD_CARD/web/
```

### Ou manter ambas:
Edite o firmware ESP32 para servir a versão desejada:
```cpp
server.on("/", HTTP_GET, []() {
    File file = SD.open("/web/index_modern.html", FILE_READ);
    server.streamFile(file, "text/html");
    file.close();
});
```

---

## 🎨 Personalização

### Interface Moderna (index_modern.html)

**Mudar cores:**
```css
:root {
    --bg-primary: #0f172a;      /* Fundo principal */
    --accent: #06b6d4;          /* Cor de destaque */
    --success: #10b981;         /* Verde - OK */
    --warning: #f59e0b;         /* Amarelo - Atenção */
    --danger: #ef4444;          /* Vermelho - Erro */
}
```

**Temas alternativos:**

**Tema Green Matrix:**
```css
--bg-primary: #0d1117;
--accent: #00ff41;
--success: #39ff14;
```

**Tema Purple Haze:**
```css
--bg-primary: #1a0a2e;
--accent: #b388ff;
--success: #69f0ae;
```

**Tema Orange Energy:**
```css
--bg-primary: #1c1c1e;
--accent: #ff9500;
--success: #30d158;
```

### Presentation

**Mudar esquema de cores:**
```css
:root {
    --primary: #00d4ff;    /* Azul cyan */
    --secondary: #ff006e;  /* Rosa */
    --accent: #8338ec;     /* Roxo */
}
```

---

## 📱 Comparação Visual

```
┌─────────────────────────────────────┐
│     index.html (Original)           │
├─────────────────────────────────────┤
│ Tema: Colorido                      │
│ Fundo: Gradiente roxo               │
│ Cards: Brancos com sombras          │
│ Estilo: Amigável                    │
│ Público: Geral                      │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│   index_modern.html (Moderna)       │
├─────────────────────────────────────┤
│ Tema: Dark Mode                     │
│ Fundo: Azul escuro (#0f172a)        │
│ Cards: Bordas cyan com hover        │
│ Estilo: Profissional                │
│ Público: Tech/Industrial            │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│   presentation.html (Landing)       │
├─────────────────────────────────────┤
│ Tema: Cyberpunk                     │
│ Fundo: Grid animado                 │
│ Efeitos: Muita animação             │
│ Estilo: Marketing                   │
│ Público: Apresentação/Demo          │
└─────────────────────────────────────┘
```

---

## 🚀 Recomendações

### Para Farm 3D Pessoal:
→ Use **index.html** (original)
- Visual amigável
- Cores alegres

### Para Farm 3D Profissional:
→ Use **index_modern.html**
- Visual sério
- Estilo industrial/lab

### Para Apresentar o Projeto:
→ Use **presentation.html**
- Impressiona
- Marketing perfeito

### Para Mostrar Funcionando:
→ Use qualquer dashboard + abra no celular
- Demonstre responsividade
- Mostre atualização em tempo real

---

## 💡 Dica Pro

**Combine ambas!**

Tenha as duas interfaces no SD Card:
- `/web/index.html` - Interface padrão
- `/web/dark.html` - Interface moderna
- `/web/presentation.html` - Landing page

Adicione botão para trocar:
```html
<a href="/dark.html" class="btn">🌙 Tema Escuro</a>
<a href="/index.html" class="btn">☀️ Tema Claro</a>
```

---

**Escolha a que mais combina com seu estilo! 🎨✨**
