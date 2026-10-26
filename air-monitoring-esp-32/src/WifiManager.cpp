#include <WifiManager.h>

WifiManager::WifiManager() {

}

void WifiManager::connect(const char* ssid, const char* password) {
    Serial.print(String("Connection to the Wi-Fi:") + ssid +  String(" in progress..."));

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println(String("Connected to Wi-Fi network:") + ssid);
}

boolean WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}