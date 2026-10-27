#ifndef __GAS_SENSOR_MQ2_WOKWI__
#define __GAS_SENSOR_MQ2_WOKWI__

#include "GasSensor.h"

class GasSensorMQ2_Wokwi: public GasSensor {
    private:
        int pin;

    public:
        GasSensorMQ2_Wokwi(int pin);
        virtual float getPPM();
};

#endif