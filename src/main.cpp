#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define WDT_TIMEOUT 30

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "192.168.1.100";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
  } else {
    Serial.println(" WiFi FAILED — will retry next loop.");
  }
}

void connectMQTT() {
  if (client.connected()) return;
  Serial.print("Connecting to MQTT...");
  if (client.connect("ESP32Client")) {
    Serial.println(" Connected!");
  } else {
    Serial.println(" MQTT FAILED — will retry next loop.");
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  dht.begin();
  connectWiFi();
  client.setServer(mqtt_server, 1883);
  connectMQTT();
}

void loop() {
  esp_task_wdt_reset();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost — reconnecting...");
    connectWiFi();
  }

  if (!client.connected()) {
    Serial.println("MQTT lost — reconnecting...");
    connectMQTT();
  }

  client.loop();
  delay(5000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("ERROR: Failed to read from DHT22 sensor!");
    return;
  }

  char tempStr[8];
  char humStr[8];
  dtostrf(temperature, 4, 1, tempStr);
  dtostrf(humidity, 4, 1, humStr);

  client.publish("aircare/temperature", tempStr);
  client.publish("aircare/humidity", humStr);

  Serial.print("Published — Temp: ");
  Serial.print(tempStr);
  Serial.print(" C  |  Humidity: ");
  Serial.println(humStr);
}