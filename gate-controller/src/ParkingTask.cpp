#include "ParkingTask.h"
#include <Arduino.h>

ParkingTask::ParkingTask(Light *greenLed, Light *redLed, ParkingContext *context)
{
    this->greenLed = greenLed;
    this->redLed = redLed;
    this->context = context;
}

void ParkingTask::init(int period)
{
    Task::init(period);
    shownFreeSpots = context->getFreeSpots();
    shownAlarm = context->isAlarmOn();
    shownOffline = context->isOffline();
    setState(computeState());
}

// CLOSED has priority over FULL and AVAILABLE
ParkingTask::State ParkingTask::computeState()
{
    if (context->isParkingClosed())
    {
        return CLOSED;
    }
    else if (context->getFreeSpots() == 0)
    {
        return FULL;
    }
    else
    {
        return AVAILABLE;
    }
}

void ParkingTask::setState(State s)
{
    state = s;
    if (s == AVAILABLE)
    {
        greenLed->switchOn();
        redLed->switchOff();
    }
    else
    {
        greenLed->switchOff();
        redLed->switchOn();
    }
    showMessage();
}

// message priority: EVACUATE > OFFLINE > CLOSED > FULL > free spots
void ParkingTask::showMessage()
{
    shownFreeSpots = context->getFreeSpots();
    shownAlarm = context->isAlarmOn();
    shownOffline = context->isOffline();

    if (shownAlarm)
    {
        Serial.println(F("# LCD: EVACUARE"));
    }
    else if (shownOffline)
    {
        Serial.println(F("# LCD: OFFLINE"));
    }
    else if (state == CLOSED)
    {
        Serial.println(F("# LCD: PARCHEGGIO CHIUSO"));
    }
    else if (state == FULL)
    {
        Serial.println(F("# LCD: PARCHEGGIO PIENO"));
    }
    else
    {
        Serial.print(F("# LCD: Posti liberi: "));
        Serial.println(shownFreeSpots);
    }
}

void ParkingTask::tick()
{
    State next = computeState();
    if (next != state)
    {
        setState(next); // transition
    }
    else if (context->getFreeSpots() != shownFreeSpots
            || context->isAlarmOn() != shownAlarm
            || context->isOffline() != shownOffline)
    {
        showMessage(); // self-loop: refresh only
    }
}