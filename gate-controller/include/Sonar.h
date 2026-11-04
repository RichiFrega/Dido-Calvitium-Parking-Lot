#ifndef __SONAR__
#define __SONAR__

#include "Arduino.h"
#include "ProximitySensor.h"

class Sonar : public ProximitySensor
{
public:
    Sonar(int triggerPin, int echoPin);
    void sample();
    bool isDetected();

protected:
    int triggerPin;
    int echoPin;
    uint8_t nearCount;  // letture "vicine" consecutive
    uint8_t farCount;   // letture "lontane" consecutive
    bool detected;      // stato filtrato
};

#endif