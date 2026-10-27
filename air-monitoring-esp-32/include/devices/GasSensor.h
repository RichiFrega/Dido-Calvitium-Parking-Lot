#ifndef __GAS_SENSOR__
#define __GAS_SENSOR__

class GasSensor {

    private:
        const char* topic = "gas";

    public:
        virtual float getPPM() = 0;

        const char* getTopic() {
            return this->topic;
        }
};

#endif