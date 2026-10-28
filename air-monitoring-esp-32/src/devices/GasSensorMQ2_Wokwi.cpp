#include <Arduino.h>
#include <math.h>
#include "devices/GasSensorMQ2_Wokwi.h"
#include "kernel/Logger.h"

/**
 * MQ2 Wokwi: sensing interval [0,1ppm,  100.000ppm] logarithmically mapped in [3.857(RS/RO), 0.01336(RS/RO)]
 * 
 * Rs = ((Vc / VRL) - 1) * RL
 * Rs = ((4095 / ADC value) - 1) * RL
 * For convenience Wokwi sets RL and RO both equal to 1
 * so:
 *      Rs = ((4095 - ADC value) / ADC value)
 * 
 * y = m * x + c
 * log10(RS/RO) = m * log10(ppm) + c
 * but RO = 1
 * so:
 *      log10(RS) = m * log10(ppm) + c
 * 
 * ppm = 10^((log10(RS) - c) / m)
 * 
 * ADC value = 843 -> 0,1ppm
 * ADC value = 4041 -> 100.000ppm
 * 
 * Rs1 = (4095 - 843) / 843 = 3,8576
 * Rs2 = (4095 - 4041) / 4041 = 0,01336
 * 
 * m = (log10(Rs2) - log10(Rs1)) / (log10(100.000) - log10(0.1)) = -0,41006
 * c = log10(3,8576) - (m * log10(0,1)) = 0,17638
 * 
 * so:
 *      m = -0,41006
 *      c = 0,17638
 *      Rs = ((4095 - ADC value) / ADC value)
 *      ppm = 10^((log10(RS) - c) / m)
 */

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