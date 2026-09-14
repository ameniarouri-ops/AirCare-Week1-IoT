#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* mqtt_server = "a8984e032441467797687728dde49d14.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "esp32device";
const char* mqtt_pass = "AirCare2026!";

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define PMS_RX 16
#define PMS_TX 17
HardwareSerial pmsSerial(2);

WiFiClientSecure espClient;
PubSubClient client(espClient);
WiFiManager wifiManager;

unsigned long lastMsg = 0;
const long interval = 5000;

struct PMS7003Data {
  uint16_t pm1_0;
  uint16_t pm2_5;
  uint16_t pm10;
  bool valid;
};

void setup_wifi() {
  wifiManager.setConfigPortalTimeout(180);

  if (!wifiManager.autoConnect("AirCarePlus-Setup")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }

  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT connecting...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 5s");
      delay(5000);
    }
  }
}

PMS7003Data readPMS7003() {
  PMS7003Data data = {0, 0, 0, false};
  if (pmsSerial.available() < 32) return data;

  while (pmsSerial.available() >= 2) {
    byte b1 = pmsSerial.read();
    if (b1 == 0x42 && pmsSerial.peek() == 0x4D) {
      pmsSerial.read();
      break;
    }
  }

  if (pmsSerial.available() < 30) return data;

  byte buf[30];
  pmsSerial.readBytes(buf, 30);

  uint16_t frameLen = (buf[0] << 8) | buf[1];
  if (frameLen != 28) return data;

  uint16_t calcSum = 0x42 + 0x4D;
  for (int i = 0; i < 28; i++) calcSum += buf[i];
  uint16_t rxSum = (buf[28] << 8) | buf[29];
  if (calcSum != rxSum) return data;

  data.pm1_0 = (buf[8] << 8) | buf[9];
  data.pm2_5 = (buf[10] << 8) | buf[11];
  data.pm10  = (buf[12] << 8) | buf[13];
  data.valid = true;
  return data;
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  dht.begin();
  pmsSerial.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
  Serial.println("AirCare+ Week 7 - AP Config Portal init");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      client.publish("aircare/temperature", String(t, 1).c_str());
      client.publish("aircare/humidity", String(h, 1).c_str());
      Serial.printf("DHT -> Temp: %.1fC  Hum: %.1f%% | ", t, h);
    }

    PMS7003Data pms = readPMS7003();
    if (pms.valid) {
      client.publish("aircare/pm1", String(pms.pm1_0).c_str());
      client.publish("aircare/pm25", String(pms.pm2_5).c_str());
      client.publish("aircare/pm10", String(pms.pm10).c_str());
      Serial.printf("PMS -> PM1.0:%d PM2.5:%d PM10:%d\n", pms.pm1_0, pms.pm2_5, pms.pm10);
    } else {
      Serial.println("PMS -> no valid frame");
    }
  }
}