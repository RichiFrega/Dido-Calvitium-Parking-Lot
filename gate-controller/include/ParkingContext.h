#ifndef __PARKING_CONTEXT__
#define __PARKING_CONTEXT__

enum GateEvent
{
    EV_NONE,
    EV_ENTER,
    EV_EXIT,
    EV_ABORT
}; // eventi prodotti dal gatetask

// stato condiviso tra i task (shared variables, modulo 2.3)
class ParkingContext
{
public:
    ParkingContext(int initialFreeSpots);

    // posti liveri
    int getFreeSpots();
    void setFreeSpots(int n);

    // allarme
    bool isAlarmOn();
    void setAlarm(bool on);

    // regola di ingresso: posti disponibili e allarme spento
    bool canEnter();

    // eventi della sbarra
    void registerEnter();
    void registerExit();
    void registerAbort();

    // consumo dell'evento
    bool hasEvent();
    GateEvent takeEvent();

private:
    int freeSpots;
    bool alarmOn;
    GateEvent pendingEvent;
};

#endif