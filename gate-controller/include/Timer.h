// Dal corso ESIoT, module-lab-2.3 (Prof. Ricci)

#ifndef __TIMER__
#define __TIMER__

class Timer {

public:
  Timer();
  void setupFreq(int freq);
  /* period in ms */
  void setupPeriod(int period);
  void waitForNextTick();

};


#endif

