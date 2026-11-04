// Dal corso ESIoT, module-lab-2.1 (Prof. Ricci)

#include "ButtonImpl.h"
#include "Arduino.h"

ButtonImpl::ButtonImpl(int pin){
  this->pin = pin;
  pinMode(pin, INPUT);
}

bool ButtonImpl::isPressed(){
  return digitalRead(pin) == HIGH;
}


