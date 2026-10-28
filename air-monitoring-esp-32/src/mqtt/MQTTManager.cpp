#include "mqtt/MQTTManager.h"
#include "kernel/Logger.h"
#include "config.h"

MQTTManager::MQTTManager(const char* broker, int port, const char* defaultTopic)
    : client(espClient), broker(broker), port(port), defaultTopic(defaultTopic) {}

void MQTTManager::setup() {
    client.setServer(this->broker, this->port);
    client.setCallback([this](char* topic, byte* payload, unsigned int len) {
        if (!handler) return;
        String msg;
        for (unsigned int i = 0; i < len; i++) {
            msg += (char)payload[i];
        }
        handler(String(topic), msg);
    });
}

bool MQTTManager::tryConnect() {
    String clientId = "ESP32-Dido-Calvitium-Parking-Lot-" + String(random(0xffff), HEX);
    if (!client.connect(clientId.c_str())) return false;
    String fullTopic = String(this->defaultTopic) + "/" + String(MQTT_TOPIC_CONFIGURATION);
    return client.subscribe(fullTopic.c_str());
}

bool MQTTManager::isConnected() {
    return client.connected();
}

void MQTTManager::loop() {
    client.loop();
}

bool MQTTManager::publish(const char* subTopic, const char* message) {
    if (client.connected()) {
        String fullTopic = String(this->defaultTopic) + "/" + String(subTopic);
        Logger.log(String("Publishing message in [") + fullTopic + String("]: ") + message);
        return client.publish(fullTopic.c_str(), message); 
    }
    Logger.log("Broker not Connected.");
    return false;
}