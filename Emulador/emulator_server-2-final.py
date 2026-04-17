#!/usr/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
from datetime import datetime

class EmulatorHandler(BaseHTTPRequestHandler):

    state = {
        "temperature": 25.0,
        "humidity": 50,
        "light": 1500,
        "soundDB": 55,
        "material": "PLA",
        "calibration": {
            "tempOffset": 0,
            "humOffset": 0,
            "lightOffset": 0,
            "soundOffset": 0
        },
        "config": {
            "tempMin": 18,
            "tempMax": 28,
            "humMin": 40,
            "humMax": 60,
            "lightMax": 3000,
            "soundMaxDB": 70
        },
        "system": {
            "machineName": "PrintSense 3D",
            "interval": 2,
            "logs": True,
            "alertLimit": 5
        }
    }

    materials = {
        "PLA": {"tempMin": 18, "tempMax": 28, "humMin": 40, "humMax": 60, "lightMax": 3000, "soundMaxDB": 70},
        "PETG": {"tempMin": 20, "tempMax": 30, "humMin": 30, "humMax": 50, "lightMax": 3000, "soundMaxDB": 70},
        "ABS": {"tempMin": 22, "tempMax": 32, "humMin": 20, "humMax": 40, "lightMax": 3000, "soundMaxDB": 70},
        "RESINA": {"tempMin": 20, "tempMax": 25, "humMin": 40, "humMax": 60, "lightMax": 1000, "soundMaxDB": 65}
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
        if self.path == "/api/data":
            self.send_data()

        elif self.path == "/api/config":
            self.send_json(self.state["config"])

        elif self.path == "/api/calibration":
            self.send_json(self.state["calibration"])
        elif self.path == "/api/system":
            self.send_json(self.state["system"])
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
            self.state.update(data)
            self.send_json({"success": True})
        elif self.path == "/api/system":
            self.state["system"] = data
            self.send_json({"success": True})
        else:
            self.send_404()

    # ================= FUNÇÕES =================
    def send_data(self):
        temp = self.state["temperature"] + self.state["calibration"]["tempOffset"]
        hum = self.state["humidity"] + self.state["calibration"]["humOffset"]
        light = self.state["light"] + self.state["calibration"]["lightOffset"]
        sound = self.state["soundDB"] + self.state["calibration"]["soundOffset"]

        status = self.calculate_status(temp, hum, light, sound)

        response = {
            "temperature": temp,
            "humidity": hum,
            "light": light,
            "soundDB": sound,
            "material": self.state["material"],
            "status": status,
            "timestamp": datetime.now().strftime("%H:%M:%S"),
            "thresholds": self.state["config"]
        }

        self.send_json(response)

    def change_material(self, data):
        material = data.get("material")

        if material not in self.materials:
            self.send_json({"error": "Material inválido"}, 400)
            return

        self.state["material"] = material
        self.state["config"] = self.materials[material]

        self.send_json({"success": True, "material": material})

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
            return "IDEAL"
        elif errors == 1:
            return "BOM"
        else:
            return "RUIM"

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


# ================= START =================
def run():
    server = HTTPServer(("0.0.0.0", 8080), EmulatorHandler)

    print("\n🚀 Servidor rodando:")
    print("👉 http://localhost:8080")
    print("👉 http://SEU_IP:8080")
    print("\nAPI:")
    print("GET  /api/data")
    print("POST /api/material")
    print("GET  /api/config")
    print("POST /api/config")
    print("GET  /api/calibration")
    print("POST /api/calibration\n")

    server.serve_forever()


if __name__ == "__main__":
    run()