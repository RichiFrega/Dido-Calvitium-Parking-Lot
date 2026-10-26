#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WifiManager {
    public:
        WifiManager();

        void connect(const char* ssid, const char* password);
        void loop(const char* ssid, const char* password);
};

#endif