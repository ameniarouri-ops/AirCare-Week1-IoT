import paho.mqtt.client as mqtt
import csv
import datetime

CSV_FILE = "mqtt_log.csv"
BROKER = "localhost"
PORT = 1883
TOPIC = "aircare/#"

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
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT, 60)
client.loop_forever()