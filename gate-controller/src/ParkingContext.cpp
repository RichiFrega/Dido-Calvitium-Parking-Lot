#include "ParkingContext.h"

ParkingContext::ParkingContext(int initialFreeSpots)
{
    freeSpots = initialFreeSpots;
    alarmOn = false;
    parkingClosed = false; // at startup the parking lot is open
    head = 0;
    count = 0;
    openRequested = false;
    offline = false;
    offlineEntries = 0;
    offlineExits = 0;
}

int ParkingContext::getFreeSpots()
{
    return freeSpots;
}

void ParkingContext::setFreeSpots(int n)
{
    if (n < 0)
    {
        n = 0;
    }
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

bool ParkingContext::isParkingClosed()
{
    return parkingClosed;
}

void ParkingContext::setParkingClosed(bool closed)
{
    parkingClosed = closed;
}

bool ParkingContext::canEnter()
{
    return (freeSpots > 0) && !alarmOn && !parkingClosed;
}

void ParkingContext::registerEnter()
{
    pushEvent(EV_ENTER);

    if (freeSpots > 0)
    {
        freeSpots--;
    }
    if (offline)
    {
        offlineEntries++;
    }
}

void ParkingContext::registerExit()
{
    pushEvent(EV_EXIT);
    freeSpots++;

    if (offline)
    {
        offlineExits++;
    }
}

void ParkingContext::registerAbort()
{
    pushEvent(EV_ABORT);
}

void ParkingContext::registerGateOpen()
{
    pushEvent(EV_GATE_OPEN);
}

void ParkingContext::registerGateClosed()
{
    pushEvent(EV_GATE_CLOSED);
}

bool ParkingContext::hasEvent()
{
    return count > 0;
}

GateEvent ParkingContext::takeEvent()
{
    if (count == 0)
    {
        return EV_NONE;
    }
    GateEvent e = events[head];
    head = (head + 1) % EVENT_QUEUE_SIZE;
    count--;
    return e;
}

void ParkingContext::pushEvent(GateEvent e)
{
    if (count == EVENT_QUEUE_SIZE) // full: drop the oldest event
    {
        head = (head + 1) % EVENT_QUEUE_SIZE;
        count--;
    }
    uint8_t tail = (head + count) % EVENT_QUEUE_SIZE;
    events[tail] = e;
    count++;
}

void ParkingContext::requestOpen()
{
    openRequested = true;
}

bool ParkingContext::takeOpenRequest()
{
    bool r = openRequested;
    openRequested = false;
    return r;
}

bool ParkingContext::isOffline()
{
    return offline;
}

void ParkingContext::setOffline(bool value)
{
    if (value && !offline) // entering degraded mode, counters start from 0
    {
        offlineEntries = 0;
        offlineExits = 0;
    }
    if (!value) // back online
    {
        offlineEntries = 0;
        offlineExits = 0;
    }
    offline = value;
}

int ParkingContext::getOfflineEntries()
{
    return offlineEntries;
}

int ParkingContext::getOfflineExits()
{
    return offlineExits;
}

void ParkingContext::registerButtonOpen() {
  pushEvent(EV_BUTTON_OPEN);
}