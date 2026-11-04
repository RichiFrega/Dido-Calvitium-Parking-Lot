// Dal corso ESIoT, module-lab-1.3 (Prof. Ricci)

#ifndef __SERVO_MOTOR__
#define __SERVO_MOTOR__

class ServoMotor {

public:
  virtual void on() = 0;
  virtual void setPosition(int angle) = 0;
  virtual void off() = 0;
};

#endif
