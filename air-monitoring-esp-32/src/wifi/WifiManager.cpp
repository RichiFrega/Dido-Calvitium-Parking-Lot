#include "wifi/WifiManager.h"
#include "kernel/Logger.h"

WifiManager::WifiManager(const char* ssid, const char* password)
    : ssid(ssid), password(password) {}

void WifiManager::begin() {
    Logger.log(String("Connecting to Wi-Fi [") + ssid + "]...");
    WiFi.begin(this->ssid, this->password);
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}