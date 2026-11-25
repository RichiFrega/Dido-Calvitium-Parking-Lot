#ifndef __PARKING_TASK__
#define __PARKING_TASK__

#include "Task.h"
#include "Light.h"
#include "ParkingContext.h"
#include "Display.h"

/*
 * Parking lot state (AVAILABLE / FULL / CLOSED:
 * status LEDs and messages for the user.
 */
class ParkingTask : public Task
{
public:
    ParkingTask(Light *greenLed, Light *redLed, ParkingContext *context, Display* display);
    void init(int period);
    void tick();

private:
    enum State
    {
        AVAILABLE,
        FULL,
        CLOSED
    } state;

    Light *greenLed;
    Light *redLed;
    ParkingContext *context;
    Display* display;

    int shownFreeSpots; // last number shown to the user
    bool shownAlarm;    // last alarm state shown
    bool shownOffline;  // last offline state shown

    State computeState();
    void setState(State s);
    void showMessage();
};

#endif