#ifndef __TEMP_SENSOR__
#define __TEMP_SENSOR__

class TempSensor {

    private:
        const char* topic = "temp";

    public:
        virtual float getTemperature() = 0;

        const char* getTopic() {
            return this->topic;
        }
};

#endif