#!/usr/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import random
from datetime import datetime
from urllib.parse import urlparse

class EmulatorHandler(BaseHTTPRequestHandler):

    # ==================== STATE ====================
    state = {
        'temperature': 25.0,
        'humidity': 55.0,
        'light': 1800,
        'soundADC': 580,
        'soundDB': 58.0,
        'material': 'PLA',
        'status': 'IDEAL',
        'ledStatus': 'GREEN',

        # 🔥 NOVO (V2)
        'calibration': {
            'tempOffset': 0,
            'humOffset': 0,
            'lightOffset': 0,
            'soundOffset': 0
        },
        'config': {
            'tempMin': 18,
            'tempMax': 28,
            'humMin': 40,
            'humMax': 60,
            'lightMax': 3000,
            'soundMaxDB': 70
        }
    }

    materials = {
        'PLA': {'tempMin': 18, 'tempMax': 28, 'humMin': 40, 'humMax': 60, 'lightMax': 3000, 'soundMaxDB': 70},
        'PETG': {'tempMin': 20, 'tempMax': 30, 'humMin': 30, 'humMax': 50, 'lightMax': 3000, 'soundMaxDB': 70},
        'ABS': {'tempMin': 22, 'tempMax': 32, 'humMin': 20, 'humMax': 40, 'lightMax': 3000, 'soundMaxDB': 70},
        'RESINA': {'tempMin': 20, 'tempMax': 25, 'humMin': 40, 'humMax': 60, 'lightMax': 1000, 'soundMaxDB': 65}
    }

    # ==================== CORS ====================
    def cors(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Headers", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")

    def do_OPTIONS(self):
        self.send_response(200)
        self.cors()
        self.end_headers()

    # ==================== GET ====================
    def do_GET(self):
        path = urlparse(self.path).path

        if path == "/":
            self.send_index()

        elif path == "/api/data":
            self.send_data()

        elif path == "/api/logs":
            self.send_logs()

        elif path.startswith("/api/log/"):
            filename = path.split("/")[-1]
            self.send_log_file(filename)

        elif path == "/api/config":
            self.send_json(self.state["config"])

        elif path == "/api/calibration":
            self.send_json(self.state["calibration"])

        else:
            self.send_404()

    # ==================== POST ====================
    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length).decode() if length else "{}"
        data = json.loads(body)

        if self.path == "/api/material":
            self.change_material(data)

        elif self.path == "/api/config":
            self.state["config"].update(data)
            self.send_json({"success": True})

        elif self.path == "/api/calibration":
            self.state["calibration"].update(data)
            self.send_json({"success": True})

        elif self.path == "/api/simulate":
            self.state.update(data)
            self.send_json({"success": True})

        else:
            self.send_404()

    # ==================== DATA ====================
    def send_data(self):

        # 🔥 RESTAURADO (V1) → flutuação automática
        self.state['temperature'] += random.uniform(-0.2, 0.2)
        self.state['humidity'] += random.uniform(-0.5, 0.5)
        self.state['soundDB'] += random.uniform(-1, 1)

        # 🔥 NOVO (V2) → aplicar calibração
        temp = self.state["temperature"] + self.state["calibration"]["tempOffset"]
        hum = self.state["humidity"] + self.state["calibration"]["humOffset"]
        light = self.state["light"] + self.state["calibration"]["lightOffset"]
        sound = self.state["soundDB"] + self.state["calibration"]["soundOffset"]

        self.calculate_status(temp, hum, light, sound)

        response = {
            "temperature": round(temp, 1),
            "humidity": round(hum, 1),
            "light": light,
            "soundADC": self.state["soundADC"],  # RESTAURADO
            "soundDB": round(sound, 1),
            "material": self.state["material"],
            "status": self.state["status"],
            "ledStatus": self.state["ledStatus"],  # RESTAURADO
            "statusDetails": f"Condições {self.state['status'].lower()} para {self.state['material']}",  # RESTAURADO
            "thresholds": self.state["config"],  # V2
            "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }

        print(f"[{datetime.now().strftime('%H:%M:%S')}] GET /api/data → {self.state['status']}")

        self.send_json(response)

    # ==================== MATERIAL ====================
    def change_material(self, data):
        material = data.get("material")

        if material not in self.materials:
            self.send_json({"error": "Material inválido"}, 400)
            return

        self.state["material"] = material
        self.state["config"] = self.materials[material]  # V2

        print(f"[INFO] Material alterado para {material}")

        self.send_json({"success": True, "material": material})

    # ==================== LOGS ====================
    def send_logs(self):
        logs = ["20240208.csv", "20240207.csv"]
        self.send_json({"logs": logs})

    def send_log_file(self, filename):
        csv = "timestamp,temperature,humidity,light,soundADC,soundDB,status\n"

        for i in range(24):
            temp = 20 + random.uniform(0, 8)
            hum = 50 + random.uniform(-10, 10)
            csv += f"2024-02-08 {i:02d}:00,{temp:.1f},{hum:.1f},1000,500,60,IDEAL\n"

        self.send_response(200)
        self.send_header("Content-Type", "text/csv")
        self.cors()
        self.end_headers()
        self.wfile.write(csv.encode())

    # ==================== STATUS ====================
    def calculate_status(self, temp, hum, light, sound):
        c = self.state["config"]

        errors = 0
        if temp < c["tempMin"] or temp > c["tempMax"]:
            errors += 1
        if hum < c["humMin"] or hum > c["humMax"]:
            errors += 1
        if light > c["lightMax"]:
            errors += 1
        if sound > c["soundMaxDB"]:
            errors += 1

        if errors == 0:
            self.state["status"] = "IDEAL"
            self.state["ledStatus"] = "GREEN"
        elif errors == 1:
            self.state["status"] = "BOM"
            self.state["ledStatus"] = "YELLOW"
        else:
            self.state["status"] = "RUIM"
            self.state["ledStatus"] = "RED"

    # ==================== UI ====================
    def send_index(self):
        html = """
        <h1>🚀 PrintSense Emulator</h1>
        <p>Servidor ativo</p>
        <ul>
            <li><a href="/api/data">/api/data</a></li>
            <li><a href="/api/logs">/api/logs</a></li>
            <li><a href="/api/config">/api/config</a></li>
        </ul>
        """
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.cors()
        self.end_headers()
        self.wfile.write(html.encode())

    # ==================== HELPERS ====================
    def send_json(self, data, code=200):
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.cors()
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())

    def send_404(self):
        self.send_json({"error": "Endpoint não encontrado"}, 404)

    def log_message(self, *args):
        return


# ==================== RUN ====================
def run():
    server = HTTPServer(("0.0.0.0", 8080), EmulatorHandler)
    print("🚀 Rodando em http://localhost:8080")
    server.serve_forever()

if __name__ == "__main__":
    run()