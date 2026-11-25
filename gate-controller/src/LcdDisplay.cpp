#include "LcdDisplay.h"

LcdDisplay::LcdDisplay(uint8_t address, uint8_t columns, uint8_t rows)
    : lcd(address, columns, rows) {}

void LcdDisplay::init()
{
    lcd.init();
    lcd.backlight();
}

void LcdDisplay::showText(const __FlashStringHelper *text)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(text);
}

void LcdDisplay::showFreeSpots(int freeSpots)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Posti liberi:"));
    lcd.setCursor(0, 1);
    lcd.print(freeSpots);
}