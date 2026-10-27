#include "mqtt/MQTTManager.h"
#include "kernel/Logger.h"

MQTTManager::MQTTManager(const char* broker, int port, const char* defaultTopic)
    : client(espClient), broker(broker), port(port), defaultTopic(defaultTopic) {}

void MQTTManager::setup() {
    client.setServer(this->broker, this->port);
}

void MQTTManager::loop() {
    while (!client.connected()) {
        Logger.log(String("Attempting to connect to the MQTT broker [") + this->broker + String("]..."));

        // Generates a globally unique Client ID to prevent disconnections in a private Broker better one forever
        String clientId = "ESP32-Dido-Calvitium-Parking-Lot-" + String(random(0xffff), HEX);

        if (client.connect(clientId.c_str())) {
            Logger.log("Connected to the MQTT broker.");
        } else {
            Logger.log( String("Failed to connect to the MQTT broker, rc=") 
                + client.state() + String(" retrying in 5 seconds"));
            delay(5000);
        }
    }
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