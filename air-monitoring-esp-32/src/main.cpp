#include <Arduino.h>
#include "config.h"
#include "kernel/Logger.h"
#include "wifi/WifiManager.h"
#include "mqtt/MQTTManager.h"
#include "network/NetworkFSM.h"
#include "devices/GasSensor.h"
#include "devices/TempSensor.h"
#include "devices/GasSensorMQ2_Wokwi.h"
#include "devices/TempSensorTMP36.h"

TaskHandle_t Task1;
void Task1comm(void*);

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MQTTManager mqttManager(MQTT_BROKER, MQTT_PORT, MQTT_TOPIC);
GasSensor* gasSensor;
TempSensor* tempSensor;
NetworkFSM networkFSM(wifiManager, mqttManager, GREEN_LED, RED_LED);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Logger.log("ESP32 Air Monitoring System started.");

  gasSensor = new GasSensorMQ2_Wokwi(GAS_PIN);
  tempSensor = new TempSensorTMP36(TEMP_PIN);

  xTaskCreatePinnedToCore(Task1comm, "Task1", 8192, NULL, 1, &Task1, 0);  
}

void loop() {

  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    lastTime = millis();
    float ppmValue = gasSensor->getPPM();
    float tempValue = tempSensor->getTemperature();
    mqttManager.publish(gasSensor->getTopic(), String(ppmValue).c_str());
    mqttManager.publish(tempSensor->getTopic(), String(tempValue).c_str());
  }
}

void Task1comm(void*) {
  Logger.log(String("Task1 is running on core: ") + xPortGetCoreID());
  networkFSM.setup();
  for (;;) {
    networkFSM.step();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
