#include "GateTask.h"
#include <Arduino.h>
#include "config.h"

GateTask::GateTask(ProximitySensor *sensorA, ProximitySensor *sensorB,
                   ServoMotor *barrier, ParkingContext *context)
{
    this->sensorA = sensorA;
    this->sensorB = sensorB;
    this->barrier = barrier;
    this->context = context;
}

void GateTask::setState(State s)
{
    state = s;
    stateStart = millis();

    switch (s)
    {
    case CLOSED:
        Serial.println(F("# Gate -> CLOSED"));
        break;
    case OPENING:
        if (direction == ENTERING)
            Serial.println(F("# Gate -> OPENING (entering)"));
        else if (direction == EXITING)
            Serial.println(F("# Gate -> OPENING (exinting)"));
        else
            Serial.println(F("# Gate -> OPENING (manual)"));
        break;
    case OPEN:
        Serial.println(direction == ENTERING
                           ? F("# Gate -> OPEN (entering)")
                           : F("# Gate -> OPEN (exiting)"));
        break;
    case WAIT_CLEAR:
        Serial.println(F("# Gate -> WAIT_CLEAR"));
        break;
    case CLOSING:
        Serial.println(F("# Gate -> CLOSING"));
        break;
    }
}

unsigned long GateTask::elapsedInState()
{
    return millis() - stateStart;
}

void GateTask::init(int period)
{
    Task::init(period);
    barrier->on();
    barrier->setPosition(BARRIER_ANGLE_CLOSED);
    passed = false;
    direction = ENTERING;
    setState(CLOSED);
}

void GateTask::openFor(Direction d)
{
    barrier->setPosition(BARRIER_ANGLE_OPEN);
    direction = d;
    passed = false;
    setState(OPENING);
}

void GateTask::tick()
{
    sensorA->sample();
    sensorB->sample();
    bool a = sensorA->isDetected();
    bool b = sensorB->isDetected();

    bool openRequest = context->takeOpenRequest(); // leggo ad ogni tick

    switch (state)
    {
    case CLOSED:
    {
        if (a && !b && context->canEnter())
        {
            openFor(ENTERING);
        }
        else if (!a && b)
        {
            openFor(EXITING);
        }
        else if (openRequest)
        {
            openFor(MANUAL); // non controllo canEnter()
        }

        break;
    }

    case OPENING:
    {
        if (elapsedInState() >= BARRIER_TRAVEL_MS)
        {
            context->registerGateOpen();
            setState(OPEN);
        }

        break;
    }

    case OPEN:
    {
        if (direction == MANUAL)
        {
            if (elapsedInState() >= T1_MS)
            {
                setState(WAIT_CLEAR); // transito non contato
            }
        }

        bool opposite = (direction == ENTERING) ? b : a;

        if (opposite)
        {
            passed = true; // il veicolo è al sensore opposto
        }
        else if (passed)
        {
            if (direction == ENTERING) // ha raggiunto l'opposto e l'ha liberato
            {
                context->registerEnter();
            }
            else
            {
                context->registerExit();
            }
            setState(WAIT_CLEAR);
        }
        else if (elapsedInState() > T1_MS) // mai arrivato entro T1
        {
            context->registerAbort();
            setState(WAIT_CLEAR);
        }

        break;
    }

    case WAIT_CLEAR:
    {
        if (!a && !b) // entrambi i sensori liberi
        {
            barrier->setPosition(BARRIER_ANGLE_CLOSED);
            setState(CLOSING);
        }

        break;
    }

    case CLOSING:
    {
        if (elapsedInState() >= BARRIER_TRAVEL_MS) // attesa per la chiusura
        {
            context->registerGateClosed();
            setState(CLOSED);
        }
        else if (a || b) // ostacolo durante la chiusura
        {
            barrier->setPosition(BARRIER_ANGLE_OPEN);
            setState(WAIT_CLEAR);
        }

        break;
    }
    }
}