#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include <Arduino.h>
#include <WiFi.h>

class WifiManager {
    private:
        const char* ssid;
        const char* password;

    public:
        WifiManager(const char* ssid, const char* password);

        void begin();
        bool isConnected();
};

#endif