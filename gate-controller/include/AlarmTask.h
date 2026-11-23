#ifndef __ALARM_TASK__
#define __ALARM_TASK__

#include "Task.h"
#include "Buzzer.h"
#include "ParkingContext.h"

class AlarmTask : public Task
{
public:
    AlarmTask(Buzzer *buzzer, ParkingContext *context);
    void init(int period);
    void tick();

private:
    enum State
    {
        OFF,
        ON
    } state;
    Buzzer *buzzer;
    ParkingContext *context;
    bool sounding;
};

#endif