#!/usr/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import random
from datetime import datetime
import sys
from urllib.parse import urlparse

class EmulatorHandler(BaseHTTPRequestHandler):

    state = {
        'temperature': 25.0,
        'humidity': 55.0,
        'light': 1800,
        'soundADC': 580,
        'soundDB': 58.0,
        'material': 'PLA',
        'status': 'IDEAL',
        'ledStatus': 'GREEN',

        # V3+
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

    # ================= CORS =================
    def cors(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Headers", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")

    def do_OPTIONS(self):
        self.send_response(200)
        self.cors()
        self.end_headers()

    # ================= GET =================
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

    # ================= POST =================
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
            self.simulate_update(data)

        else:
            self.send_404()

    # ================= DATA =================
    def send_data(self):

        # Flutuação automática
        self.state['temperature'] += random.uniform(-0.2, 0.2)
        self.state['humidity'] += random.uniform(-0.5, 0.5)
        self.state['soundDB'] += random.uniform(-1, 1)

        # 🔥 Correção: recalcular ADC SEMPRE
        self.state['soundADC'] = int((self.state['soundDB'] - 30) * 20)

        # Aplicar calibração
        temp = self.state['temperature'] + self.state['calibration']['tempOffset']
        hum = self.state['humidity'] + self.state['calibration']['humOffset']
        light = self.state['light'] + self.state['calibration']['lightOffset']
        sound = self.state['soundDB'] + self.state['calibration']['soundOffset']

        self.calculate_status(temp, hum, light, sound)

        response = {
            "temperature": round(temp, 1),
            "humidity": round(hum, 1),
            "light": light,
            "soundADC": self.state["soundADC"],
            "soundDB": round(sound, 1),
            "material": self.state["material"],
            "status": self.state["status"],
            "ledStatus": self.state["ledStatus"],
            "statusDetails": f"Condições {self.state['status'].lower()} para {self.state['material']}",
            "thresholds": self.state["config"],
            "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }

        print(f"[{datetime.now().strftime('%H:%M:%S')}] /api/data → {self.state['status']}")

        self.send_json(response)

    # ================= SIMULATE =================
    def simulate_update(self, data):
        for key in ['temperature', 'humidity', 'light', 'soundDB']:
            if key in data:
                self.state[key] = data[key]

        # Recalcular ADC
        self.state['soundADC'] = int((self.state['soundDB'] - 30) * 20)

        self.send_json({"success": True})

    # ================= MATERIAL =================
    def change_material(self, data):
        material = data.get("material")

        if material not in self.materials:
            self.send_json({"error": "Material inválido"}, 400)
            return

        self.state["material"] = material
        self.state["config"] = self.materials[material]

        print(f"[INFO] Material → {material}")

        self.send_json({"success": True, "material": material})

    # ================= STATUS =================
    def calculate_status(self, temp, hum, light, sound):
        c = self.state["config"]

        # 🔥 Histerese restaurada
        TEMP_TOL = 2
        HUM_TOL = 5

        errors = 0

        if temp < c["tempMin"] - TEMP_TOL or temp > c["tempMax"] + TEMP_TOL:
            errors += 1
        if hum < c["humMin"] - HUM_TOL or hum > c["humMax"] + HUM_TOL:
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

    # ================= LOGS =================
    def send_logs(self):
        self.send_json({"logs": ["20240208.csv", "20240207.csv", "20240206.csv"]})

    def send_log_file(self, filename):
        csv = "timestamp,temperature,humidity,light,soundADC,soundDB,status\n"

        for i in range(24):
            temp = 20 + random.uniform(0, 8)
            hum = 50 + random.uniform(-10, 10)
            csv += f"2024-02-08 {i:02d}:00,{temp:.1f},{hum:.1f},1000,500,60,IDEAL\n"

        self.send_response(200)
        self.send_header("Content-Type", "text/csv")
        self.send_header("Content-Disposition", f'attachment; filename="{filename}"')
        self.cors()
        self.end_headers()
        self.wfile.write(csv.encode())

    # ================= UI =================
    def send_index(self):
        html = """
        <h1>PrintSense Emulator V4</h1>
        <p>Servidor completo rodando</p>
        <pre>
fetch('/api/data')
        </pre>
        """
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.cors()
        self.end_headers()
        self.wfile.write(html.encode())

    # ================= HELPERS =================
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


def run():
    port = 8080

    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except:
            print("Uso: python server.py [porta]")
            return

    print(f"🚀 Rodando em http://localhost:{port}")
    server = HTTPServer(("0.0.0.0", port), EmulatorHandler)
    server.serve_forever()


if __name__ == "__main__":
    run()