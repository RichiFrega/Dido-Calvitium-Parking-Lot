#include "WifiManager.h"
#include "Logger.h"

WifiManager::WifiManager(const char* ssid, const char* password)
    : ssid(ssid), password(password) {}

void WifiManager::connect() {
    Logger.log(String("Connection to the Wi-Fi [") + this->ssid +  String("] in progress..."));

    WiFi.begin(this->ssid, this->password);

    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Logger.log(String("Connected to Wi-Fi network:") + this->ssid);
}

void WifiManager::loop(){
    if (WiFi.status() != WL_CONNECTED){
        Logger.log("Wi-Fi Disconnected... Trying Reconnection.");
        connect();
    }
}