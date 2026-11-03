// Dal corso ESIoT, module-lab-2.1 (Prof. Ricci)

#ifndef __LED__
#define __LED__

#include "Light.h"

class Led: public Light {
public:
  Led(int pin);
  void switchOn();
  void switchOff();
private:
  int pin;
};

#endif
