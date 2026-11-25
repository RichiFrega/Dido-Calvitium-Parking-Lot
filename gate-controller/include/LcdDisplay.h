#ifndef __LCD_DISPLAY__
#define __LCD_DISPLAY__

#include "Display.h"
#include <LiquidCrystal_I2C.h>

class LcdDisplay : public Display
{
public:
    LcdDisplay(uint8_t adress, uint8_t colums, uint8_t rows);
    void init();
    void showText(const __FlashStringHelper *text);
    void showFreeSpots(int freeSpots);
private:
    LiquidCrystal_I2C lcd;
};

#endif