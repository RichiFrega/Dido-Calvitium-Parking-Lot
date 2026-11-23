#include "ButtonEvent.h"
#include <Arduino.h>
#include "config.h"

static volatile bool pressed = false;
static volatile unsigned long lastPressMs = 0;

// ISR
static void onButtonPressed()
{
    unsigned long now = millis();
    if (now - lastPressMs >= BUTTON_DEBOUNCE_MS)
    {
        pressed = true;
        lastPressMs = now;
    }
}

void buttonEventInit(int pin)
{
    pinMode(pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(pin), onButtonPressed, RISING);
}

bool buttonEventTake()
{
    noInterrupts();
    bool p = pressed;
    pressed = false;
    interrupts();
    return p;
}