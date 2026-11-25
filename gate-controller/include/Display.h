#ifndef __DISPLAY__
#define __DISPLAY__
#include <Arduino.h>

class Display
{
public:
    virtual void init() = 0;
    virtual void showText(const __FlashStringHelper *text) = 0;
    virtual void showFreeSpots(int freeSpots) = 0;
};

#endif