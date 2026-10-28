#include <Arduino.h>
#include "devices/TempSensorTMP36.h"
#include "kernel/Logger.h"

/**
 * TMP36: sensing interval [-40°C,  +125°C] mapped linearly in [0.1Vdc, 1.7Vdc]
 * so 
 * - 0°C => tension: 500mV
 * - a change of 1 °C => change of 10mV
 * - precision: +- 2°C
 * formula °C = ((ADC value * ADC precision) - Offset) / stepVoltPerDegree
 * formula °C = ((ADC value * 0.000805) - 0.5) / 0.01
 * where:
 * -  ADC value = analogue value read 
 * - 0.000805 =  3.3Vdc/4095 = ADC precision
 * - 0.5 = Tension at zero degree
 * - 0.01 = stepVoltPerDegree
 */

#define numberOfSample 5

TempSensorTMP36::TempSensorTMP36(int pin) : pin(pin) {}

float TempSensorTMP36::getTemperature() {
    float values[numberOfSample];

    for (int i = 0; i < numberOfSample; i++) {
        int adc_value = analogRead(this->pin);

        float tempInCelsius = ((adc_value * 0.000805) - 0.5) / 0.01;
        values[i] = tempInCelsius;

        Logger.log(i+1 + String("° Measured Temperature:") + tempInCelsius + String("°C"));
    }
    float sum = 0;

    for (int i = 0; i < numberOfSample; i++) {
        sum += values[i];
    }
    
    Logger.log(String("Average Temp:") + sum/numberOfSample + String("°C"));
    return sum/numberOfSample;
}