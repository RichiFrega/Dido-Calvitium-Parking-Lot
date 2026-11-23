#include "AlarmTask.h"
#include <Arduino.h>

AlarmTask::AlarmTask(Buzzer *buzzer, ParkingContext *context)
{
    this->buzzer = buzzer;
    this->context = context;
}

void AlarmTask::init(int period)
{
    Task::init(period);
    buzzer->off();
    sounding = false;
    state = OFF;
}

void AlarmTask::tick()
{
    switch (state)
    {
    case OFF:
        if (context->isAlarmOn())
        {
            Serial.println(F("# Alarm -> ON"));
            buzzer->on();
            sounding = true;
            state = ON;
        }
        break;

    case ON:
        if (!context->isAlarmOn())
        {
            Serial.println(F("# Alarm -> OFF"));
            buzzer->off();
            sounding = false;
            state = OFF;
        }
        else
        {
            sounding = !sounding;
            if (sounding)
            {
                buzzer->on();
            }
            else
            {
                buzzer->off();
            }
        }
        break;
    }
}