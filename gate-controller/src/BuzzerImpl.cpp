#include "BuzzerImpl.h"
#include <Arduino.h>

BuzzerImpl::BuzzerImpl(int pin)
{
    this->pin = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void BuzzerImpl::on()
{
    digitalWrite(pin, HIGH);
}

void BuzzerImpl::off()
{
    digitalWrite(pin, LOW);
}