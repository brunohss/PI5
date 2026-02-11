/*
 * Teste DHT22 - Debug
 * Teste isolado do sensor para diagnosticar problema
 */

#include <DHT.h>

#define DHT_PIN 4      // GPIO 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== TESTE DHT22 ===");
  Serial.print("Pino configurado: GPIO ");
  Serial.println(DHT_PIN);
  
  // Inicializar DHT22
  dht.begin();
  
  Serial.println("DHT22 inicializado!");
  Serial.println("Aguardando 3 segundos...\n");
  delay(3000);
}

void loop() {
  Serial.println("--- Lendo sensor ---");
  
  // Ler temperatura
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  // Debug detalhado
  Serial.print("Temperatura RAW: ");
  Serial.println(temp);
  Serial.print("Umidade RAW: ");
  Serial.println(hum);
  
  // Verificar se leitura foi bem-sucedida
  if (isnan(temp) || isnan(hum)) {
    Serial.println("❌ ERRO: Leitura falhou!");
    Serial.println("\nPossíveis causas:");
    Serial.println("1. Conexão solta");
    Serial.println("2. Falta resistor pull-up (4.7k entre VCC e DATA)");
    Serial.println("3. Sensor defeituoso");
    Serial.println("4. Pino GPIO errado");
    Serial.println("5. Alimentação inadequada\n");
  } else {
    Serial.println("✅ Leitura OK!");
    Serial.print("   Temperatura: ");
    Serial.print(temp);
    Serial.println("°C");
    Serial.print("   Umidade: ");
    Serial.print(hum);
    Serial.println("%\n");
  }
  
  delay(3000);  // Aguardar 3 segundos entre leituras
}
