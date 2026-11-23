#ifndef __BUZZER_IMPL__
#define __BUZZER_IMPL__

#include "Buzzer.h"

class BuzzerImpl : public Buzzer
{
public:
    BuzzerImpl(int pin);
    void on();
    void off();

private:
    int pin;
};

#endif