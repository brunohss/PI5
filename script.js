// ==================== VARIÁVEIS GLOBAIS ====================
let currentMaterial = 'PLA';
let updateInterval = null;
let isConnected = true;

// Thresholds (sincronizado com ESP32)
const materialThresholds = {
    'PLA': { tempMin: 18, tempMax: 28, humMin: 40, humMax: 60, lightMax: 500, soundMax: 70 },
    'PETG': { tempMin: 20, tempMax: 30, humMin: 30, humMax: 50, lightMax: 500, soundMax: 70 },
    'ABS': { tempMin: 22, tempMax: 32, humMin: 20, humMax: 40, lightMax: 500, soundMax: 70 },
    'RESINA': { tempMin: 20, tempMax: 25, humMin: 40, humMax: 60, lightMax: 100, soundMax: 60 }
};

// ==================== INICIALIZAÇÃO ====================
document.addEventListener('DOMContentLoaded', function() {
    console.log('PrintSense - Interface carregada');
    
    // Carregar material salvo
    const savedMaterial = localStorage.getItem('selectedMaterial');
    if (savedMaterial && materialThresholds[savedMaterial]) {
        currentMaterial = savedMaterial;
        document.getElementById('materialSelect').value = savedMaterial;
    }
    
    // Iniciar atualização automática
    startAutoUpdate();
    
    // Primeira atualização imediata
    updateData();
});

// ==================== ATUALIZAÇÃO DE DADOS ====================
function startAutoUpdate() {
    // Atualizar a cada 2 segundos
    updateInterval = setInterval(updateData, 2000);
}

function stopAutoUpdate() {
    if (updateInterval) {
        clearInterval(updateInterval);
        updateInterval = null;
    }
}

async function updateData() {
    try {
        const response = await fetch('/api/data');
        
        if (!response.ok) {
            throw new Error('Erro na resposta do servidor');
        }
        
        const data = await response.json();
        
        // Atualizar status de conexão
        setConnectionStatus(true);
        
        // Atualizar valores dos sensores
        updateSensorValues(data);
        
        // Atualizar status geral
        updateGeneralStatus(data);
        
        // Atualizar timestamp
        document.getElementById('lastUpdate').textContent = data.timestamp;
        
    } catch (error) {
        console.error('Erro ao buscar dados:', error);
        setConnectionStatus(false);
    }
}

// ==================== ATUALIZAÇÃO DE VALORES ====================
function updateSensorValues(data) {
    // Temperatura
    const temp = parseFloat(data.temperature);
    document.getElementById('temperature').textContent = 
        isNaN(temp) ? '--' : temp.toFixed(1) + '°C';
    
    const tempThresh = materialThresholds[currentMaterial];
    document.getElementById('tempRange').textContent = 
        `Ideal: ${tempThresh.tempMin}-${tempThresh.tempMax}°C`;
    
    updateSensorStatus('temp', temp, tempThresh.tempMin, tempThresh.tempMax);
    
    // Umidade
    const hum = parseFloat(data.humidity);
    document.getElementById('humidity').textContent = 
        isNaN(hum) ? '--' : hum.toFixed(1) + '%';
    
    document.getElementById('humRange').textContent = 
        `Ideal: ${tempThresh.humMin}-${tempThresh.humMax}%`;
    
    updateSensorStatus('hum', hum, tempThresh.humMin, tempThresh.humMax);
    
    // Luminosidade
    const light = parseInt(data.light);
    document.getElementById('light').textContent = 
        isNaN(light) ? '--' : light + ' lux';
    
    updateSensorStatus('light', light, 0, tempThresh.lightMax);
    
    // Ruído
    const sound = parseInt(data.sound);
    document.getElementById('sound').textContent = 
        isNaN(sound) ? '--' : sound;
    
    updateSensorStatus('sound', sound, 0, tempThresh.soundMax);
}

function updateSensorStatus(sensorType, value, min, max) {
    const statusElement = document.getElementById(sensorType + 'Status');
    const cardElement = document.getElementById(sensorType + 'Card');
    
    if (isNaN(value)) {
        statusElement.textContent = '⚠️ Sem dados';
        statusElement.className = 'sensor-status warning';
        return;
    }
    
    let status = 'ok';
    let text = '✅ OK';
    
    if (value < min) {
        status = 'warning';
        text = '⬇️ Abaixo do ideal';
    } else if (value > max) {
        status = 'error';
        text = '⬆️ Acima do ideal';
    }
    
    statusElement.textContent = text;
    statusElement.className = 'sensor-status ' + status;
}

// ==================== STATUS GERAL ====================
function updateGeneralStatus(data) {
    const statusCard = document.getElementById('statusCard');
    const statusTitle = document.getElementById('statusTitle');
    const statusDetails = document.getElementById('statusDetails');
    
    const status = data.status.toLowerCase();
    
    // Remover todas as classes de status
    statusCard.className = 'status-card';
    
    // Adicionar classe apropriada
    statusCard.classList.add(status);
    
    // Emoji baseado no status
    let emoji = '✅';
    if (status === 'bom') emoji = '⚠️';
    if (status === 'ruim') emoji = '❌';
    
    statusTitle.textContent = `${emoji} ${data.status} para ${data.material}`;
    statusDetails.textContent = data.statusDetails;
}

// ==================== TROCA DE MATERIAL ====================
async function changeMaterial() {
    const select = document.getElementById('materialSelect');
    const material = select.value;
    
    try {
        const response = await fetch('/api/material', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: 'material=' + encodeURIComponent(material)
        });
        
        if (!response.ok) {
            throw new Error('Erro ao alterar material');
        }
        
        const result = await response.json();
        
        if (result.success) {
            currentMaterial = material;
            localStorage.setItem('selectedMaterial', material);
            
            // Feedback visual
            showNotification(`Material alterado para ${material}`, 'success');
            
            // Atualizar imediatamente
            updateData();
        }
        
    } catch (error) {
        console.error('Erro ao trocar material:', error);
        showNotification('Erro ao alterar material', 'error');
    }
}

// ==================== LOGS ====================
async function showLogs() {
    try {
        const response = await fetch('/api/logs');
        const data = await response.json();
        
        const logsList = document.getElementById('logsList');
        const logsFiles = document.getElementById('logsFiles');
        
        logsFiles.innerHTML = '';
        
        if (data.logs && data.logs.length > 0) {
            data.logs.forEach(log => {
                const li = document.createElement('li');
                li.textContent = log;
                li.onclick = () => downloadLog(log);
                logsFiles.appendChild(li);
            });
            
            logsList.style.display = 'block';
        } else {
            logsFiles.innerHTML = '<li>Nenhum log disponível</li>';
            logsList.style.display = 'block';
        }
        
    } catch (error) {
        console.error('Erro ao buscar logs:', error);
        showNotification('Erro ao carregar logs', 'error');
    }
}

async function downloadLog(filename) {
    try {
        window.location.href = `/api/log?file=${encodeURIComponent(filename)}`;
        showNotification(`Baixando ${filename}...`, 'success');
    } catch (error) {
        console.error('Erro ao baixar log:', error);
        showNotification('Erro ao baixar arquivo', 'error');
    }
}

async function downloadCurrentLog() {
    const today = new Date();
    const filename = today.toISOString().split('T')[0].replace(/-/g, '') + '.csv';
    downloadLog(filename);
}

// ==================== UTILITÁRIOS ====================
function setConnectionStatus(connected) {
    isConnected = connected;
    const statusElement = document.getElementById('connectionStatus');
    
    if (connected) {
        statusElement.textContent = '● Online';
        statusElement.className = 'status-online';
    } else {
        statusElement.textContent = '● Offline';
        statusElement.className = 'status-offline';
    }
}

function showNotification(message, type = 'info') {
    // Criar elemento de notificação
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    
    // Estilos inline
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 15px 25px;
        border-radius: 8px;
        color: white;
        font-weight: 600;
        z-index: 1000;
        animation: slideInRight 0.3s ease-out;
        box-shadow: 0 4px 12px rgba(0,0,0,0.3);
    `;
    
    if (type === 'success') {
        notification.style.background = '#10b981';
    } else if (type === 'error') {
        notification.style.background = '#ef4444';
    } else {
        notification.style.background = '#667eea';
    }
    
    document.body.appendChild(notification);
    
    // Remover após 3 segundos
    setTimeout(() => {
        notification.style.animation = 'slideOutRight 0.3s ease-out';
        setTimeout(() => {
            document.body.removeChild(notification);
        }, 300);
    }, 3000);
}

function clearData() {
    if (confirm('Limpar cache local? Isso não afetará os dados salvos no SD Card.')) {
        localStorage.clear();
        showNotification('Cache limpo!', 'success');
        setTimeout(() => {
            location.reload();
        }, 1000);
    }
}

// ==================== ATALHOS DE TECLADO ====================
document.addEventListener('keydown', function(e) {
    // R = Refresh manual
    if (e.key === 'r' || e.key === 'R') {
        e.preventDefault();
        updateData();
        showNotification('Dados atualizados!', 'info');
    }
    
    // L = Mostrar logs
    if (e.key === 'l' || e.key === 'L') {
        e.preventDefault();
        showLogs();
    }
    
    // 1-4 = Trocar material rapidamente
    if (e.key >= '1' && e.key <= '4') {
        const materials = ['PLA', 'PETG', 'ABS', 'RESINA'];
        const idx = parseInt(e.key) - 1;
        document.getElementById('materialSelect').value = materials[idx];
        changeMaterial();
    }
});

// ==================== ANIMAÇÕES CSS ADICIONAIS ====================
const style = document.createElement('style');
style.textContent = `
    @keyframes slideInRight {
        from {
            transform: translateX(400px);
            opacity: 0;
        }
        to {
            transform: translateX(0);
            opacity: 1;
        }
    }
    
    @keyframes slideOutRight {
        from {
            transform: translateX(0);
            opacity: 1;
        }
        to {
            transform: translateX(400px);
            opacity: 0;
        }
    }
`;
document.head.appendChild(style);

// ==================== MONITORAMENTO DE VISIBILIDADE ====================
document.addEventListener('visibilitychange', function() {
    if (document.hidden) {
        stopAutoUpdate();
        console.log('Página oculta - pausando atualizações');
    } else {
        startAutoUpdate();
        updateData();
        console.log('Página visível - retomando atualizações');
    }
});

// ==================== LOG DE INICIALIZAÇÃO ====================
console.log('PrintSense JavaScript carregado');
console.log('Material inicial:', currentMaterial);
console.log('Atalhos disponíveis:');
console.log('  R - Atualizar dados manualmente');
console.log('  L - Mostrar logs');
console.log('  1-4 - Trocar material (PLA/PETG/ABS/RESINA)');
