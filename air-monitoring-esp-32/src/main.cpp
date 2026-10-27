#include <Arduino.h>
#include "config.h"
#include "kernel/Logger.h"
#include "wifi/WifiManager.h"
#include "mqtt/MQTTManager.h"
#include "devices/GasSensor.h"
#include "devices/GasSensorMQ2_Wokwi.h"

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MQTTManager mqttManager(MQTT_BROKER, MQTT_PORT, MQTT_TOPIC);
GasSensor* gasSensor;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Logger.log("ESP32 Air Monitoring System started.");

  wifiManager.connect();
  mqttManager.setup();
  gasSensor = new GasSensorMQ2_Wokwi(GAS_PIN);
}

void loop() {
  wifiManager.loop();
  mqttManager.loop();

  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    lastTime = millis();
    float ppmValue = gasSensor->getPPM();
    mqttManager.publish(gasSensor->getTopic(), String(ppmValue).c_str());
  }
}

