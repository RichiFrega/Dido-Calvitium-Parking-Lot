#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WifiManager {
    public:
        WifiManager();

        boolean isConnected();
        void connect(const char* ssid, const char* password);
};

#endif