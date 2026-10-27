#include <Arduino.h>
#include <math.h>
#include "devices/GasSensorMQ2_Wokwi.h"
#include "kernel/Logger.h"

GasSensorMQ2_Wokwi::GasSensorMQ2_Wokwi(int pin) : pin(pin) {}

float GasSensorMQ2_Wokwi::getPPM() {
    int adc_value = analogRead(this->pin);

    if (adc_value <= 0) return 0.1; 
    if (adc_value >= 4095) return 100000.0;

    float Rs = (4095.0 - (float)adc_value) / (float)adc_value;

    float m = -0.41006;
    float c = 0.17638;

    float log_Rs = log10(Rs);
    float ppm = pow(10, (log_Rs - c) / m);

    Logger.log(String("Measured ppm:") + ppm);
    return ppm;
}