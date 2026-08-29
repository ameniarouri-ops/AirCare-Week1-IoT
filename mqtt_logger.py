import paho.mqtt.client as mqtt
import csv
import datetime

CSV_FILE = "mqtt_log.csv"
BROKER = "ad2707b4f5904efb9e949f4ef799303b.s1.eu.hivemq.cloud"
PORT = 8883
TOPIC = "aircare/#"
MQTT_USER = "esp32device"
MQTT_PASS = "AirCare2026_Esp32!"

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker!")
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    topic = msg.topic
    value = msg.payload.decode()
    print(f"[{timestamp}] {topic} -> {value}")
    with open(CSV_FILE, "a", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([timestamp, topic, value])

client = mqtt.Client()
client.username_pw_set(MQTT_USER, MQTT_PASS)
client.tls_set()
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT, 60)
client.loop_forever()