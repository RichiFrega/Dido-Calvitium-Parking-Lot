#ifndef __GaTE_TASK__
#define __GaTE_TASK__

#include "ParkingContext.h"
#include "Task.h"
#include "servo_motor.h"
#include "ProximitySensor.h"

class GateTask : public Task
{

public:
    GateTask(ProximitySensor *sensorA, ProximitySensor *sensorB,
             ServoMotor *barrier, ParkingContext *context);
    void init(int period);
    void tick();

private:
    enum State
    {
        CLOSED,
        OPEN,
        WAIT_CLEAR,
        CLOSING
    } state;
    enum Direction
    {
        ENTERING,
        EXITING
    } direction;

    ProximitySensor *sensorA;
    ProximitySensor *sensorB;
    ServoMotor *barrier;
    ParkingContext *context;

    bool passed;              // il veicolo ha raggiunto il sensore opposto
    unsigned long stateStart; // tempo di inizio dello stato corrente

    void setState(State s);
    unsigned long elapsedInState();
    void openFor(Direction d);
};

#endif