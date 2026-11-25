#include "ParkingTask.h"
#include <Arduino.h>

ParkingTask::ParkingTask(Light *greenLed, Light *redLed, ParkingContext *context, Display *display)
{
    this->greenLed = greenLed;
    this->redLed = redLed;
    this->context = context;
    this->display = display;
}

void ParkingTask::init(int period)
{
    Task::init(period);
    display->init();
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
        display->showText(F("EVACUARE"));
    }
    else if (shownOffline)
    {
        display->showText(F("OFFLINE"));
    }
    else if (state == CLOSED)
    {
        display->showText(F("CHIUSO"));
    }
    else if (state == FULL)
    {
        display->showText(F("PARCHEGGIO PIENO"));
    }
    else
    {
        display->showFreeSpots(shownFreeSpots);
    }
}

void ParkingTask::tick()
{
    State next = computeState();
    if (next != state)
    {
        setState(next); // transition
    }
    else if (context->getFreeSpots() != shownFreeSpots || context->isAlarmOn() != shownAlarm || context->isOffline() != shownOffline)
    {
        showMessage(); // self-loop: refresh only
    }
}