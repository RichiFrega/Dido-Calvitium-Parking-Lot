#ifndef __PARKING_CONTEXT__
#define __PARKING_CONTEXT__

#include <Arduino.h>
#include "config.h"

enum GateEvent
{
    EV_NONE,
    EV_ENTER,
    EV_EXIT,
    EV_ABORT,
    EV_GATE_OPEN,
    EV_GATE_CLOSED
}; // events produced by the gatetask, to be sent to the cu

// state shared among tasks (shared variables, module 2.3)
class ParkingContext
{
public:
    ParkingContext(int initialFreeSpots);

    // free spots
    int getFreeSpots();
    void setFreeSpots(int n);

    // air alarm
    bool isAlarmOn();
    void setAlarm(bool on);

    //parking lot closed by an operator
    bool isParkingClosed();
    void setParkingClosed(bool closed);

    // entry role: free spots, no alarm, parking lot open
    bool canEnter();

    // events from the gatetask
    void registerEnter();
    void registerExit();
    void registerAbort();
    void registerGateOpen();
    void registerGateClosed();

    // event comsumption (commtaks), oldest first
    bool hasEvent();
    GateEvent takeEvent();

    // manual opening request: CMD:OPEN (commtask) or button
    void requestOpen();
    bool takeOpenRequest();

private:
    void pushEvent(GateEvent e);

    int freeSpots;
    bool alarmOn;
    bool parkingClosed;

    // circular queue of events
    GateEvent events[EVENT_QUEUE_SIZE];
    uint8_t head;   // index of the oldest event
    uint8_t count;  // number of events in the queue

    bool openRequested;
};

#endif