#ifndef __MQTT_MANAGER_H__
#define __MQTT_MANAGER_H__

#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

class MQTTManager {
    private:
        WiFiClient espClient;
        PubSubClient client;

        const char* broker;
        int port;
        const char* defaultTopic;

    public:
        MQTTManager(const char* broker, int port, const char* defaultTopic);

        void setup();

        bool tryConnect();
        bool isConnected();
        void loop();

        bool publish(const char* subTopic, const char* message);
};

#endif