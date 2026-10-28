#include <Arduino.h>
#include "network/NetworkFSM.h"
#include "kernel/Logger.h"
#include "config.h"

NetworkFSM::NetworkFSM(WifiManager& wifi, MQTTManager& mqtt, int greenLed, int redLed)
    : wifi(wifi), mqtt(mqtt), greenLed(greenLed), redLed(redLed) {}

void NetworkFSM::setup() {
    pinMode(this->greenLed, OUTPUT);
    pinMode(this->redLed, OUTPUT);
    this->wifi.begin();
    this->mqtt.setup();
    enter(CONNECTING);
}

void NetworkFSM::enter(State s) {
    state = s;
    stateSince = millis();
    digitalWrite(greenLed, s == CONNECTED ? HIGH : LOW);
    digitalWrite(redLed, s == CONNECTED ? LOW : HIGH);
}

void NetworkFSM::step() {
    switch (state) {
        case CONNECTING:
            if (wifi.isConnected()) {
                if (mqtt.isConnected() || mqtt.tryConnect()) {
                    enter(CONNECTED);
                    Logger.log("WiFi and MQTT connected.");
                }
                else {
                    enter(NETWORK_ERROR);
                }
            }
            else if (millis() - stateSince > 15000) {
                enter(NETWORK_ERROR);
            }
            break;
        
        case CONNECTED:
            if (!wifi.isConnected() || !mqtt.isConnected()) {
                enter(NETWORK_ERROR);
                Logger.log(":::: NETWORK ERROR ::::");
            }
            else {
                mqtt.loop();
            }
            break;

        case NETWORK_ERROR:
            if (millis() - stateSince > RETRY_RECONNECTION_DELAY_MS) {
                if (!wifi.isConnected()) {
                    wifi.begin();
                }
                if (!mqtt.isConnected()) {
                    mqtt.tryConnect();
                }
                enter(CONNECTING);
            }
            break;
    }
}