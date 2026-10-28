#include "mqtt/MQTTManager.h"
#include "kernel/Logger.h"

MQTTManager::MQTTManager(const char* broker, int port, const char* defaultTopic)
    : client(espClient), broker(broker), port(port), defaultTopic(defaultTopic) {}

void MQTTManager::setup() {
    client.setServer(this->broker, this->port);
}

bool MQTTManager::tryConnect() {
    String clientId = "ESP32-Dido-Calvitium-Parking-Lot-" + String(random(0xffff), HEX);
    return client.connect(clientId.c_str());
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