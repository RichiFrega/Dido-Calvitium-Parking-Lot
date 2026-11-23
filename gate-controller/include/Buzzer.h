#ifndef __BUZZER__
#define __BUZZER__

class Buzzer
{
public:
    virtual void on() = 0;
    virtual void off() = 0;
};

#endif