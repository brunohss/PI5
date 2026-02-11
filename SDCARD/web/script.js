// PrintSense - Dashboard Pro com Gráficos
let currentMaterial = 'PLA';
let historyChart = null;
let tempHistory = [];
let humHistory = [];
let lightHistory = [];
let soundHistory = [];
let timeLabels = [];
const MAX_DATA_POINTS = 30;

// Configurações de materiais
const materials = {
  'PLA': { tempMin: 18, tempMax: 28, humMin: 40, humMax: 60, lightMax: 3000, soundMax: 2000 },
  'PETG': { tempMin: 20, tempMax: 30, humMin: 30, humMax: 50, lightMax: 3000, soundMax: 2000 },
  'ABS': { tempMin: 22, tempMax: 32, humMin: 20, humMax: 40, lightMax: 3000, soundMax: 2000 },
  'RESINA': { tempMin: 20, tempMax: 25, humMin: 40, humMax: 60, lightMax: 1000, soundMax: 1500 }
};

// ========== INICIALIZAÇÃO DO GRÁFICO ==========
function initChart() {
  const ctx = document.getElementById('historyChart');
  if (!ctx) return;
  
  historyChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: timeLabels,
      datasets: [
        {
          label: 'Temperatura (°C)',
          data: tempHistory,
          borderColor: '#00d9ff',
          backgroundColor: 'rgba(0, 217, 255, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          yAxisID: 'y'
        },
        {
          label: 'Umidade (%)',
          data: humHistory,
          borderColor: '#00ff88',
          backgroundColor: 'rgba(0, 255, 136, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          yAxisID: 'y'
        },
        {
          label: 'Luz (lux/10)',
          data: lightHistory,
          borderColor: '#ffd700',
          backgroundColor: 'rgba(255, 215, 0, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: false,
          yAxisID: 'y1',
          hidden: true
        },
        {
          label: 'Som/10',
          data: soundHistory,
          borderColor: '#ff3366',
          backgroundColor: 'rgba(255, 51, 102, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: false,
          yAxisID: 'y1',
          hidden: true
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: {
        mode: 'index',
        intersect: false,
      },
      plugins: {
        legend: {
          display: true,
          position: 'top',
          labels: {
            color: '#e0f7ff',
            font: {
              family: 'Outfit',
              size: 12
            },
            usePointStyle: true,
            padding: 15
          }
        },
        tooltip: {
          backgroundColor: 'rgba(10, 14, 39, 0.9)',
          titleColor: '#00d9ff',
          bodyColor: '#e0f7ff',
          borderColor: '#00d9ff',
          borderWidth: 1,
          padding: 12,
          displayColors: true,
          callbacks: {
            label: function(context) {
              let label = context.dataset.label || '';
              if (label) {
                label += ': ';
              }
              if (context.parsed.y !== null) {
                // Ajustar valores divididos por 10
                if (label.includes('Luz')) {
                  label += (context.parsed.y * 10).toFixed(0) + ' lux';
                } else if (label.includes('Som')) {
                  label += (context.parsed.y * 10).toFixed(0);
                } else if (label.includes('Umidade')) {
                  label += context.parsed.y.toFixed(1) + '%';
                } else {
                  label += context.parsed.y.toFixed(1) + '°C';
                }
              }
              return label;
            }
          }
        }
      },
      scales: {
        x: {
          grid: {
            color: 'rgba(0, 217, 255, 0.1)',
            drawBorder: false
          },
          ticks: {
            color: '#8899aa',
            font: {
              family: 'JetBrains Mono',
              size: 10
            },
            maxRotation: 0,
            autoSkip: true,
            maxTicksLimit: 10
          }
        },
        y: {
          type: 'linear',
          display: true,
          position: 'left',
          grid: {
            color: 'rgba(0, 217, 255, 0.1)',
            drawBorder: false
          },
          ticks: {
            color: '#00d9ff',
            font: {
              family: 'JetBrains Mono',
              size: 11
            },
            callback: function(value) {
              return value + '°C / %';
            }
          },
          min: 0,
          max: 100
        },
        y1: {
          type: 'linear',
          display: true,
          position: 'right',
          grid: {
            drawOnChartArea: false,
          },
          ticks: {
            color: '#ffd700',
            font: {
              family: 'JetBrains Mono',
              size: 11
            },
            callback: function(value) {
              return value * 10;
            }
          }
        }
      },
      animation: {
        duration: 750,
        easing: 'easeInOutQuart'
      }
    }
  });
}

// ========== ATUALIZAÇÃO DE DADOS ==========
function updateData() {
  fetch('/api/data')
    .then(response => response.json())
    .then(data => {
      // Atualizar valores
      const tempValue = !isNaN(data.temperature) ? data.temperature.toFixed(1) + '°C' : '--';
      const humValue = !isNaN(data.humidity) ? data.humidity.toFixed(1) + '%' : '--';
      
      document.getElementById('temperature').textContent = tempValue;
      document.getElementById('humidity').textContent = humValue;
      document.getElementById('light').textContent = data.light;
      document.getElementById('sound').textContent = data.sound;
      
      // Atualizar faixas ideais
      const mat = materials[data.material] || materials['PLA'];
      document.getElementById('tempRange').textContent = `Ideal: ${mat.tempMin}-${mat.tempMax}°C`;
      document.getElementById('humRange').textContent = `Ideal: ${mat.humMin}-${mat.humMax}%`;
      document.getElementById('lightRange').textContent = `< ${mat.lightMax} lux`;
      document.getElementById('soundRange').textContent = `< ${mat.soundMax}`;
      
      // Atualizar status
      document.getElementById('statusTitle').textContent = data.status + ' para ' + data.material;
      document.getElementById('statusDetails').textContent = data.statusDetails || 'Condições ideais!';
      
      // Atualizar timestamp
      document.getElementById('lastUpdate').textContent = data.timestamp;
      
      // Sincronizar material
      currentMaterial = data.material;
      const selectElement = document.getElementById('materialSelect');
      if (selectElement && selectElement.value !== data.material) {
        selectElement.value = data.material;
      }
      
      // Atualizar badges
      updateSensorBadges(data, mat);
      
      // Atualizar card de status
      updateStatusCard(data.status);
      
      // Atualizar gráfico
      updateChart(data);
    })
    .catch(error => {
      console.error('Erro ao buscar dados:', error);
      document.getElementById('connectionStatus').textContent = '● OFFLINE';
    });
}

function updateSensorBadges(data, mat) {
  // Temperatura
  const tempBadge = document.getElementById('tempBadge');
  if (tempBadge) {
    if (data.temperature >= mat.tempMin && data.temperature <= mat.tempMax) {
      tempBadge.textContent = 'OK';
      tempBadge.className = 'metric-badge ok';
    } else if (data.temperature >= mat.tempMin - 2 && data.temperature <= mat.tempMax + 2) {
      tempBadge.textContent = 'ATENÇÃO';
      tempBadge.className = 'metric-badge warning';
    } else {
      tempBadge.textContent = 'CRÍTICO';
      tempBadge.className = 'metric-badge error';
    }
  }
  
  // Umidade
  const humBadge = document.getElementById('humBadge');
  if (humBadge) {
    if (data.humidity >= mat.humMin && data.humidity <= mat.humMax) {
      humBadge.textContent = 'OK';
      humBadge.className = 'metric-badge ok';
    } else if (data.humidity >= mat.humMin - 5 && data.humidity <= mat.humMax + 5) {
      humBadge.textContent = 'ATENÇÃO';
      humBadge.className = 'metric-badge warning';
    } else {
      humBadge.textContent = 'CRÍTICO';
      humBadge.className = 'metric-badge error';
    }
  }
  
  // Luz
  const lightBadge = document.getElementById('lightBadge');
  if (lightBadge) {
    if (data.light <= mat.lightMax) {
      lightBadge.textContent = 'OK';
      lightBadge.className = 'metric-badge ok';
    } else if (data.light <= mat.lightMax * 1.2) {
      lightBadge.textContent = 'ALTO';
      lightBadge.className = 'metric-badge warning';
    } else {
      lightBadge.textContent = 'MUITO ALTO';
      lightBadge.className = 'metric-badge error';
    }
  }
  
  // Som
  const soundBadge = document.getElementById('soundBadge');
  if (soundBadge) {
    if (data.sound <= mat.soundMax) {
      soundBadge.textContent = 'OK';
      soundBadge.className = 'metric-badge ok';
    } else if (data.sound <= mat.soundMax * 1.2) {
      soundBadge.textContent = 'ALTO';
      soundBadge.className = 'metric-badge warning';
    } else {
      soundBadge.textContent = 'MUITO ALTO';
      soundBadge.className = 'metric-badge error';
    }
  }
}

function updateStatusCard(status) {
  const statusCard = document.getElementById('statusCard');
  if (statusCard) {
    statusCard.classList.remove('ideal', 'bom', 'ruim');
    
    if (status === 'IDEAL') {
      statusCard.classList.add('ideal');
    } else if (status === 'BOM') {
      statusCard.classList.add('bom');
    } else {
      statusCard.classList.add('ruim');
    }
  }
}

function updateChart(data) {
  if (!historyChart) return;
  
  // Adicionar timestamp
  const now = new Date();
  const timeLabel = now.toLocaleTimeString('pt-BR', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  
  timeLabels.push(timeLabel);
  tempHistory.push(data.temperature);
  humHistory.push(data.humidity);
  lightHistory.push(data.light / 10); // Dividir por 10 para escala
  soundHistory.push(data.sound / 10); // Dividir por 10 para escala
  
  // Manter apenas últimos pontos
  if (timeLabels.length > MAX_DATA_POINTS) {
    timeLabels.shift();
    tempHistory.shift();
    humHistory.shift();
    lightHistory.shift();
    soundHistory.shift();
  }
  
  historyChart.update('none'); // Atualização sem animação
}

// ========== FUNÇÕES DE MATERIAL ==========
function changeMaterial() {
  const selectElement = document.getElementById('materialSelect');
  if (!selectElement) return;
  
  const material = selectElement.value;
  
  fetch('/api/material', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded',
    },
    body: 'material=' + encodeURIComponent(material)
  })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      currentMaterial = material;
      console.log('Material alterado para:', material);
      updateData();
      showNotification('Material alterado para ' + material, 'success');
    } else {
      showNotification('Erro ao alterar material', 'error');
    }
  })
  .catch(error => {
    console.error('Erro na requisição:', error);
    showNotification('Erro de conexão', 'error');
  });
}

// ========== FUNÇÕES DE LOGS ==========
function showLogs() {
  fetch('/api/logs')
    .then(response => response.json())
    .then(data => {
      if (data.logs && data.logs.length > 0) {
        displayLogsModal(data.logs);
      } else {
        showNotification('Nenhum log disponível', 'info');
      }
    })
    .catch(error => {
      console.error('Erro ao buscar logs:', error);
      showNotification('Erro ao buscar logs', 'error');
    });
}

function displayLogsModal(logs) {
  let modal = document.getElementById('logsModal');
  
  if (!modal) {
    modal = document.createElement('div');
    modal.id = 'logsModal';
    modal.className = 'modal';
    document.body.appendChild(modal);
  }
  
  let logsList = '<div class="modal-content">';
  logsList += '<span class="close-modal" onclick="closeLogsModal()">&times;</span>';
  logsList += '<h2>📁 Logs Disponíveis</h2>';
  logsList += '<div class="logs-list">';
  
  logs.forEach(log => {
    logsList += `
      <div class="log-item">
        <span class="log-name">📄 ${log}</span>
        <button class="btn-download" onclick="downloadLog('${log}')">
          ⬇️ Download
        </button>
      </div>
    `;
  });
  
  logsList += '</div></div>';
  
  modal.innerHTML = logsList;
  modal.style.display = 'block';
}

function closeLogsModal() {
  const modal = document.getElementById('logsModal');
  if (modal) {
    modal.style.display = 'none';
  }
}

function downloadLog(filename) {
  const url = '/api/log?file=' + encodeURIComponent(filename);
  const link = document.createElement('a');
  link.href = url;
  link.download = filename;
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  showNotification('Download iniciado: ' + filename, 'success');
}

function downloadToday() {
  const today = new Date();
  const filename = today.getFullYear() + 
                   String(today.getMonth() + 1).padStart(2, '0') + 
                   String(today.getDate()).padStart(2, '0') + '.csv';
  downloadLog(filename);
}

// ========== NOTIFICAÇÕES ==========
function showNotification(message, type = 'info') {
  const notification = document.createElement('div');
  notification.className = 'notification notification-' + type;
  notification.textContent = message;
  document.body.appendChild(notification);
  
  setTimeout(() => notification.classList.add('show'), 10);
  setTimeout(() => {
    notification.classList.remove('show');
    setTimeout(() => document.body.removeChild(notification), 300);
  }, 3000);
}

// ========== ESTILOS MODAIS ==========
function addModalStyles() {
  if (document.getElementById('modal-styles')) return;
  
  const style = document.createElement('style');
  style.id = 'modal-styles';
  style.textContent = `
    .modal {
      display: none;
      position: fixed;
      z-index: 1000;
      left: 0;
      top: 0;
      width: 100%;
      height: 100%;
      overflow: auto;
      background-color: rgba(0,0,0,0.8);
    }
    
    .modal-content {
      background: rgba(15, 20, 45, 0.95);
      margin: 5% auto;
      padding: 30px;
      border: 2px solid #00d9ff;
      border-radius: 20px;
      width: 80%;
      max-width: 600px;
      color: #e0f7ff;
      backdrop-filter: blur(20px);
    }
    
    .modal-content h2 {
      font-family: 'Orbitron', sans-serif;
      background: linear-gradient(135deg, #00d9ff, #00ff88);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      margin-bottom: 20px;
    }
    
    .close-modal {
      color: #8899aa;
      float: right;
      font-size: 35px;
      font-weight: bold;
      cursor: pointer;
      transition: color 0.3s;
    }
    
    .close-modal:hover {
      color: #00d9ff;
    }
    
    .logs-list {
      margin-top: 20px;
      max-height: 400px;
      overflow-y: auto;
    }
    
    .log-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 15px;
      margin: 10px 0;
      background: rgba(0, 217, 255, 0.05);
      border: 1px solid rgba(0, 217, 255, 0.2);
      border-radius: 12px;
      transition: all 0.3s;
    }
    
    .log-item:hover {
      background: rgba(0, 217, 255, 0.1);
      border-color: #00d9ff;
      transform: translateX(5px);
    }
    
    .log-name {
      font-family: 'JetBrains Mono', monospace;
    }
    
    .btn-download {
      padding: 8px 16px;
      background: linear-gradient(135deg, #00d9ff, #0066ff);
      color: #050817;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-weight: bold;
      transition: all 0.3s;
    }
    
    .btn-download:hover {
      transform: scale(1.05);
      box-shadow: 0 5px 15px rgba(0, 217, 255, 0.4);
    }
    
    .notification {
      position: fixed;
      top: 20px;
      right: 20px;
      padding: 15px 25px;
      border-radius: 12px;
      color: white;
      font-weight: bold;
      z-index: 2000;
      opacity: 0;
      transform: translateY(-20px);
      transition: all 0.3s;
      font-family: 'Outfit', sans-serif;
    }
    
    .notification.show {
      opacity: 1;
      transform: translateY(0);
    }
    
    .notification-success {
      background: linear-gradient(135deg, #00ff88, #00d9ff);
      color: #050817;
    }
    
    .notification-error {
      background: linear-gradient(135deg, #ff3366, #ff0066);
    }
    
    .notification-info {
      background: linear-gradient(135deg, #00d9ff, #0066ff);
      color: #050817;
    }
  `;
  
  document.head.appendChild(style);
}

// ========== INICIALIZAÇÃO ==========
document.addEventListener('DOMContentLoaded', function() {
  console.log('PrintSense Dashboard Pro inicializado');
  
  initChart();
  updateData();
  setInterval(updateData, 2000);
  addModalStyles();
  
  window.onclick = function(event) {
    const modal = document.getElementById('logsModal');
    if (event.target === modal) {
      closeLogsModal();
    }
  };
});

// ========== EXPORTAR FUNÇÕES ==========
window.changeMaterial = changeMaterial;
window.showLogs = showLogs;
window.closeLogsModal = closeLogsModal;
window.downloadLog = downloadLog;
window.downloadToday = downloadToday;
