#ifndef __CONFIG__
#define __CONFIG__

#define GAS_PIN 34
#define TEMP_PIN 35
#define RED_LED 26
#define GREEN_LED 27

#define RETRY_RECONNECTION_DELAY_MS 5000
//Password da spostare in un file escluso tramite .gitignore
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define MQTT_BROKER "broker.mqtt-dashboard.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "Dido-Calvitium-Parking-Lot"
#define MQTT_TOPIC_DATA "data"
#define MQTT_TOPIC_CONFIGURATION "config"

#endif
