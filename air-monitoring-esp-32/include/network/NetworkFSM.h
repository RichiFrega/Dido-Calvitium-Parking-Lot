#ifndef __NETWORK_FSM__
#define __NETWORK_FSM__

#include "wifi/WifiManager.h"
#include "mqtt/MQTTManager.h"

class NetworkFSM {

    public:
        enum State { CONNECTING, CONNECTED, NETWORK_ERROR };

        NetworkFSM(WifiManager& wifi, MQTTManager& mqtt, int greenLed, int redLed);
        void setup();
        void step();
        bool isConnected() { return state == CONNECTED; }

    private:
        WifiManager& wifi;
        MQTTManager& mqtt;
        int greenLed, redLed;
        State state;
        unsigned long stateSince;

        void enter(State s);
};

#endif