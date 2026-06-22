#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  Serial.println("AirCare+ Sensor Node - Starting...");
}

void loop() {
  digitalWrite(2, HIGH);
  Serial.println("LED ON - AirCare+ is running!");
  delay(1000);
  
  digitalWrite(2, LOW);
  Serial.println("LED OFF");
  delay(1000);
}