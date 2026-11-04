// Dal corso ESIoT, module-lab-2.1 (Prof. Ricci)

#ifndef __LIGHT__
#define __LIGHT__

class Light {
public:
  virtual void switchOn() = 0;
  virtual void switchOff() = 0;
};

#endif

