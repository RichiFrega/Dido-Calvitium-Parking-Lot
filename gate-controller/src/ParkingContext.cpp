#include "ParkingContext.h"

ParkingContext::ParkingContext(int initialFreeSpots)
{
    freeSpots = initialFreeSpots;
    alarmOn = false;
    pendingEvent = EV_NONE;
}

int ParkingContext::getFreeSpots()
{
    return freeSpots;
}

void ParkingContext::setFreeSpots(int n)
{
    if (n < 0) n = 0;
    freeSpots = n;
}

bool ParkingContext::isAlarmOn()
{
    return alarmOn;
}

void ParkingContext::setAlarm(bool on)
{
    alarmOn = on;
}

bool ParkingContext::canEnter()
{
    return (freeSpots > 0) && !alarmOn;
}

void ParkingContext::registerEnter()
{
    pendingEvent = EV_ENTER;

    if (freeSpots > 0)
        freeSpots--;
}

void ParkingContext::registerExit()
{
    pendingEvent = EV_EXIT;
    freeSpots++;
}

void ParkingContext::registerAbort()
{
    pendingEvent = EV_ABORT;
}

bool ParkingContext::hasEvent()
{
    return pendingEvent != EV_NONE;
}

GateEvent ParkingContext::takeEvent()
{
    GateEvent e = pendingEvent;
    pendingEvent = EV_NONE;
    return e;
}