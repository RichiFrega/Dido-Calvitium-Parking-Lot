#include <Arduino.h>
#include <ArduinoJson.h>
#include <atomic>
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
TaskHandle_t Task2;
void Task1comm(void*);
void Task2sampling(void*);

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MQTTManager mqttManager(MQTT_BROKER, MQTT_PORT, MQTT_TOPIC);
GasSensor* gasSensor;
TempSensor* tempSensor;
NetworkFSM networkFSM(wifiManager, mqttManager, GREEN_LED, RED_LED);

struct Sample { float temp; float co; };
QueueHandle_t samplingQueue;

//Atomic, because the value is written by Task1 on CPU0 but is read by Task2 on CPU1.
std::atomic<uint32_t> samplingPeriodMs{5000};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Logger.log("ESP32 Air Monitoring System started.");

  gasSensor = new GasSensorMQ2_Wokwi(GAS_PIN);
  tempSensor = new TempSensorTMP36(TEMP_PIN);

  samplingQueue = xQueueCreate(10, sizeof(Sample));
  xTaskCreatePinnedToCore(Task1comm, "Task1", 8192, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(Task2sampling, "Task2", 4096, NULL, 2, &Task2, 1);

  mqttManager.onMessage([](const String& topic, const String& payload) {
    if (topic.endsWith("/config")) {
      long ms = payload.toInt();
      if (ms >= 500 && ms <= 60000) {
        samplingPeriodMs = (uint32_t)ms;
        Logger.log(String("Sampling period set to:") + ms + "ms, running on core " + xPortGetCoreID());
      }
      else {
        Logger.log("Configuration ignored: invalid value.");
      }
    }
  });
}

void loop() {
    Logger.log(String("This is the main loop running on core ") + xPortGetCoreID());
    delay(10000);
}

void Task1comm(void*) {
  Logger.log(String("Task1 is running on core: ") + xPortGetCoreID());
  networkFSM.setup();
  for (;;) {
    networkFSM.step();
    Sample s;
    // Check "samplingQueue", take the data and put in "&s", "100" time to wait to se if something is coming.
    if (xQueueReceive(samplingQueue, &s, pdMS_TO_TICKS(100)) && networkFSM.isConnected()) {
      JsonDocument doc;
      doc["temp"] = s.temp;
      doc["co"] = s.co;
      char buf[64];
      serializeJson(doc, buf);
      mqttManager.publish(MQTT_TOPIC_DATA, buf);
    }
    // Pause me for 20 milliseconds, starting from this exact moment.
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void Task2sampling(void*) {
  TickType_t last = xTaskGetTickCount();
  Logger.log(String("Task2 is running on core: ") + xPortGetCoreID());
  for (;;) {
    Sample s = { tempSensor->getTemperature(), gasSensor->getPPM() };

    // Put "&s" in the queue "samplingQueue", which as 10 slot, "0" indicate that if the queue is full discard the data and move on.
    xQueueSend(samplingQueue, &s, 0);
    // Absolute Delay, Wake me up when exactly 5000 milliseconds have passed since the time recorded in last.
    if (xTaskDelayUntil(&last, pdMS_TO_TICKS(samplingPeriodMs)) == pdFALSE){
      Logger.log("WARNING: Deadline Miss in Task Sampling! System overloaded!");
    }
  }
}