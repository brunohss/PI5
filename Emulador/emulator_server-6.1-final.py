#!/usr/bin/env python3
"""
PrintSense Emulator Server
Servidor HTTP que simula a API do ESP32 para desenvolvimento frontend
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import random
import time
from datetime import datetime
from urllib.parse import parse_qs, urlparse

class PrintSenseEmulatorHandler(BaseHTTPRequestHandler):
    """Handler para requests HTTP do emulador"""
    
    # Estado do emulador (compartilhado entre requests)
    state = {
        'temperature': 25.0,
        'humidity': 55.0,
        'light': 1800,
        'soundADC': 580,
        'soundDB': 58.0,
        'material': 'PLA',
        'status': 'IDEAL',
        'ledStatus': 'GREEN'
    }
    
    materials = {
        'PLA': {'tempMin': 18, 'tempMax': 28, 'humMin': 40, 'humMax': 60, 'lightMax': 3000, 'soundMaxDB': 70},
        'PETG': {'tempMin': 20, 'tempMax': 30, 'humMin': 30, 'humMax': 50, 'lightMax': 3000, 'soundMaxDB': 70},
        'ABS': {'tempMin': 22, 'tempMax': 32, 'humMin': 20, 'humMax': 40, 'lightMax': 3000, 'soundMaxDB': 70},
        'RESINA': {'tempMin': 20, 'tempMax': 25, 'humMin': 40, 'humMax': 60, 'lightMax': 1000, 'soundMaxDB': 65}
    }
    
    def do_GET(self):
        """Handle GET requests"""
        parsed_path = urlparse(self.path)
        
        if parsed_path.path == '/api/data':
            self.send_api_data()
        elif parsed_path.path == '/api/logs':
            self.send_logs_list()
        elif parsed_path.path.startswith('/api/log/'):
            filename = parsed_path.path.split('/')[-1]
            self.send_log_file(filename)
        elif parsed_path.path == '/':
            self.send_index()
        else:
            self.send_404()
    
    def do_POST(self):
        """Handle POST requests"""
        parsed_path = urlparse(self.path)
        
        if parsed_path.path == '/api/material':
            self.change_material()
        elif parsed_path.path == '/api/simulate':
            self.update_sensors()
        else:
            self.send_404()
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def send_cors_headers(self):
        """Enviar headers CORS para permitir acesso cross-origin"""
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
    
    def send_api_data(self):
        """Enviar dados dos sensores (GET /api/data)"""
        # Adicionar variação aleatória nos sensores
        self.state['temperature'] += random.uniform(-0.2, 0.2)
        self.state['humidity'] += random.uniform(-0.5, 0.5)
        self.state['soundDB'] += random.uniform(-1, 1)
        
        # Calcular status
        self.calculate_status()
        
        data = {
            'temperature': round(self.state['temperature'], 1),
            'humidity': round(self.state['humidity'], 1),
            'light': self.state['light'],
            'soundADC': self.state['soundADC'],
            'soundDB': round(self.state['soundDB'], 1),
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'material': self.state['material'],
            'status': self.state['status'],
            'ledStatus': self.state['ledStatus'],
            'statusDetails': f'Condições {self.state["status"].lower()} para {self.state["material"]}',
            'thresholds': self.materials[self.state['material']]
        }
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_cors_headers()
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode())
        
        print(f"[{datetime.now().strftime('%H:%M:%S')}] GET /api/data → Status: {self.state['status']}")
    
    def change_material(self):
        try:
            content_length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_length).decode()

            print("Body recebido:", body)

            if not body:
                raise Exception("Body vazio")

            data = json.loads(body)

            material = data.get("material")

            if not material:
                raise Exception("Material não enviado")

            if material not in self.materials:
                raise Exception("Material inválido")

            self.state['material'] = material
            self.calculate_status()

            response = {
                "success": True,
                "material": material,
                "status": self.state['status']
            }

            self.send_response(200)

        except Exception as e:
            print("ERRO INTERNO:", e)

            response = {
                "success": False,
                "error": str(e)
            }

            self.send_response(400)

        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()

        self.wfile.write(json.dumps(response).encode())
    
    def update_sensors(self):
        """Atualizar valores dos sensores (POST /api/simulate)"""
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode()
        
        try:
            data = json.loads(body)
            
            if 'temperature' in data:
                self.state['temperature'] = float(data['temperature'])
            if 'humidity' in data:
                self.state['humidity'] = float(data['humidity'])
            if 'light' in data:
                self.state['light'] = int(data['light'])
            if 'soundDB' in data:
                self.state['soundDB'] = float(data['soundDB'])
                self.state['soundADC'] = int((data['soundDB'] - 30) * 20)
            
            self.calculate_status()
            
            response = {
                'success': True,
                'updatedState': self.state
            }
            self.send_response(200)
            
        except Exception as e:
            response = {
                'success': False,
                'error': str(e)
            }
            self.send_response(400)
        
        self.send_header('Content-Type', 'application/json')
        self.send_cors_headers()
        self.end_headers()
        self.wfile.write(json.dumps(response, indent=2).encode())
    
    def send_logs_list(self):
        """Listar logs disponíveis (GET /api/logs)"""
        logs = [
            '20240208.csv',
            '20240207.csv',
            '20240206.csv'
        ]
        
        response = {'logs': logs}
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_cors_headers()
        self.end_headers()
        self.wfile.write(json.dumps(response, indent=2).encode())
    
    def send_log_file(self, filename):
        """Enviar arquivo de log (GET /api/log/YYYYMMDD.csv)"""
        # Gerar log CSV de exemplo
        csv_data = "timestamp,temperature,humidity,light,soundADC,soundDB,status\n"
        
        for i in range(24):
            temp = 20 + random.uniform(0, 8)
            hum = 50 + random.uniform(-15, 15)
            light = 500 + int(random.uniform(0, 3000))
            sound_db = 50 + random.uniform(0, 20)
            sound_adc = int((sound_db - 30) * 20)
            status = random.choice(['IDEAL', 'IDEAL', 'BOM', 'RUIM'])
            
            csv_data += f"2024-02-08 {i:02d}:00:00,{temp:.1f},{hum:.1f},{light},{sound_adc},{sound_db:.1f},{status}\n"
        
        self.send_response(200)
        self.send_header('Content-Type', 'text/csv')
        self.send_header('Content-Disposition', f'attachment; filename="{filename}"')
        self.send_cors_headers()
        self.end_headers()
        self.wfile.write(csv_data.encode())
        
        print(f"[{datetime.now().strftime('%H:%M:%S')}] GET /api/log/{filename}")
    
    def send_index(self):
        """Enviar página index (GET /)"""
        html = """
        <!DOCTYPE html>
        <html>
        <head>
            <title>PrintSense Emulator API</title>
            <style>
                body { font-family: Arial; max-width: 800px; margin: 50px auto; padding: 20px; background: #f5f5f5; }
                h1 { color: #667eea; }
                .endpoint { background: white; padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #667eea; }
                .method { color: #4caf50; font-weight: bold; }
                code { background: #f0f0f0; padding: 2px 6px; border-radius: 3px; }
            </style>
        </head>
        <body>
            <h1>🤖 PrintSense Emulator API</h1>
            <p>Servidor de desenvolvimento para frontend do PrintSense</p>
            
            <div class="endpoint">
                <p><span class="method">GET</span> <code>/api/data</code></p>
                <p>Retorna dados atuais dos sensores em JSON</p>
            </div>
            
            <div class="endpoint">
                <p><span class="method">POST</span> <code>/api/material</code></p>
                <p>Muda material. Parâmetro: <code>material=PLA|PETG|ABS|RESINA</code></p>
            </div>
            
            <div class="endpoint">
                <p><span class="method">POST</span> <code>/api/simulate</code></p>
                <p>Atualiza sensores. Body JSON: <code>{"temperature": 25, "humidity": 50, ...}</code></p>
            </div>
            
            <div class="endpoint">
                <p><span class="method">GET</span> <code>/api/logs</code></p>
                <p>Lista arquivos de log disponíveis</p>
            </div>
            
            <div class="endpoint">
                <p><span class="method">GET</span> <code>/api/log/YYYYMMDD.csv</code></p>
                <p>Download de arquivo de log específico</p>
            </div>
            
            <h2>Teste Rápido</h2>
            <p>Abra o console do navegador (F12) e execute:</p>
            <code style="display: block; padding: 10px; background: #333; color: #0f0;">
fetch('http://localhost:8080/api/data')<br>
  .then(r => r.json())<br>
  .then(data => console.log(data));
            </code>
        </body>
        </html>
        """
        
        self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.send_cors_headers()
        self.end_headers()
        self.wfile.write(html.encode())
    
    def send_404(self):
        """Enviar erro 404"""
        self.send_response(404)
        self.send_header('Content-Type', 'application/json')
        self.send_cors_headers()
        self.end_headers()
        
        error = {'error': 'Endpoint não encontrado', 'path': self.path}
        self.wfile.write(json.dumps(error).encode())
    
    def calculate_status(self):
        """Calcular status baseado nos sensores"""
        mat = self.materials[self.state['material']]
        out_of_range = 0
        
        # Tolerâncias (histerese)
        TEMP_TOL = 2.0
        HUM_TOL = 5.0
        LIGHT_TOL = 200
        SOUND_TOL = 5.0
        
        if (self.state['temperature'] < (mat['tempMin'] - TEMP_TOL) or 
            self.state['temperature'] > (mat['tempMax'] + TEMP_TOL)):
            out_of_range += 1
        
        if (self.state['humidity'] < (mat['humMin'] - HUM_TOL) or 
            self.state['humidity'] > (mat['humMax'] + HUM_TOL)):
            out_of_range += 1
        
        if self.state['light'] > (mat['lightMax'] + LIGHT_TOL):
            out_of_range += 1
        
        if self.state['soundDB'] > (mat['soundMaxDB'] + SOUND_TOL):
            out_of_range += 1
        
        if out_of_range == 0:
            self.state['status'] = 'IDEAL'
            self.state['ledStatus'] = 'GREEN'
        elif out_of_range == 1:
            self.state['status'] = 'BOM'
            self.state['ledStatus'] = 'YELLOW'
        else:
            self.state['status'] = 'RUIM'
            self.state['ledStatus'] = 'RED'
    
    def log_message(self, format, *args):
        """Sobrescrever log padrão para não poluir console"""
        pass  # Comentar esta linha para ver logs detalhados


def run_server(port=8080):
    """Iniciar servidor"""
    server_address = ('', port)
    httpd = HTTPServer(server_address, PrintSenseEmulatorHandler)
    
    print("=" * 60)
    print("🤖 PrintSense Emulator Server")
    print("=" * 60)
    print(f"\n✅ Servidor iniciado em: http://localhost:{port}")
    print(f"📡 API disponível em: http://localhost:{port}/api/data")
    print("\n💡 Endpoints disponíveis:")
    print("   GET  /api/data           - Dados dos sensores")
    print("   POST /api/material       - Mudar material")
    print("   POST /api/simulate       - Atualizar sensores")
    print("   GET  /api/logs           - Listar logs")
    print("   GET  /api/log/FILE.csv   - Download de log")
    print("\n⌨️  Pressione Ctrl+C para parar o servidor\n")
    print("=" * 60)
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\n🛑 Servidor encerrado pelo usuário")
        httpd.shutdown()


if __name__ == '__main__':
    import sys
    
    port = 8080
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except:
            print("Uso: python emulator_server.py [porta]")
            sys.exit(1)
    
    run_server(port)
