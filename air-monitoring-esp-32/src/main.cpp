#include <Arduino.h>
#include "config.h"
#include "Logger.h"
#include "WifiManager.h"
#include "MQTTManager.h"

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MQTTManager mqttManager(MQTT_BROKER, MQTT_PORT, MQTT_TOPIC);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Logger.log("ESP32 Air Monitoring System started.");

  wifiManager.connect();
  mqttManager.setup();
}

void loop() {
  wifiManager.loop();
  mqttManager.loop();

  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    lastTime = millis();

    mqttManager.publish("gas", "450");
  }
}

