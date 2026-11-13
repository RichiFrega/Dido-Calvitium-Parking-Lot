#include <Arduino.h>
#include "config.h"
#include "Scheduler.h"
#include "Sonar.h"
#include "servo_motor_impl.h"
#include "ParkingContext.h"
#include "GateTask.h"

/*
TEST ONLY: proximity sensor driven from the keyboard.
It implements the same interface as Sonar, so the GateTask
cannot tell the difference.
*/
class FakeProximitySensor : public ProximitySensor {
public:
  FakeProximitySensor() { detected = false; }
  void sample() { }                         // nothing to measure
  bool isDetected() { return detected; }
  void toggle() { detected = !detected; }
private:
  bool detected;
};

// true = sensor A is fake too (key 'a'), false = A is the real sonar
constexpr bool FAKE_A = false;

Scheduler sched;
ParkingContext* context;
FakeProximitySensor* fakeA = nullptr;
FakeProximitySensor* fakeB = nullptr;

const __FlashStringHelper* eventName(GateEvent e) {
  switch (e) {
    case EV_ENTER:       return F("EV:ENTER");
    case EV_EXIT:        return F("EV:EXIT");
    case EV_ABORT:       return F("EV:ABORT");
    case EV_GATE_OPEN:   return F("GATE:OPEN");
    case EV_GATE_CLOSED: return F("GATE:CLOSED");
    default:             return F("NONE");
  }
}

void printStatus() {
  Serial.print(F("# free: "));
  Serial.print(context->getFreeSpots());
  Serial.print(F("  alarm: "));
  Serial.print(context->isAlarmOn());
  Serial.print(F("  closed: "));
  Serial.print(context->isParkingClosed());
  Serial.print(F("  canEnter: "));
  Serial.print(context->canEnter());
  Serial.print(F("  B: "));
  Serial.println(fakeB->isDetected());
}

void printHelp() {
  Serial.println(F("# keys: b = toggle B, a = toggle A (if fake),"));
  Serial.println(F("#       f = parking full, r = reset free spots,"));
  Serial.println(F("#       x = alarm on/off, p = parking closed/open,"));
  Serial.println(F("#       s = status, h = help"));
  Serial.println(F("#       o = manual open"));
}

void handleCommand(char c) {
  switch (c) {
    case 'b': fakeB->toggle(); printStatus(); break;
    case 'a': if (fakeA != nullptr) { fakeA->toggle(); } printStatus(); break;
    case 'f': context->setFreeSpots(0); printStatus(); break;
    case 'r': context->setFreeSpots(TEST_INITIAL_FREE_SPOTS); printStatus(); break;
    case 'x': context->setAlarm(!context->isAlarmOn()); printStatus(); break;
    case 'p': context->setParkingClosed(!context->isParkingClosed()); printStatus(); break;
    case 's': printStatus(); break;
    case 'h': printHelp(); break;
    case 'o': context->requestOpen(); break;
    default: break;
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  ProximitySensor* sensorA;
  if (FAKE_A) {
    fakeA = new FakeProximitySensor();
    sensorA = fakeA;
  } else {
    sensorA = new Sonar(PIN_SONAR_A_TRIG, PIN_SONAR_A_ECHO);
  }
  fakeB = new FakeProximitySensor();

  ServoMotor* barrier = new ServoMotorImpl(PIN_SERVO);
  context = new ParkingContext(TEST_INITIAL_FREE_SPOTS);

  sched.init(GATE_TASK_MS);
  Task* gateTask = new GateTask(sensorA, fakeB, barrier, context);
  gateTask->init(GATE_TASK_MS);
  sched.addTask(gateTask);

  Serial.println(F("# GateTask test started"));
  printHelp();
  printStatus();
}

void loop() {
  sched.schedule();

  // keyboard commands
  while (Serial.available()) {
    handleCommand(Serial.read());
  }

  // temporary: the events will be consumed by the commtask
  while (context->hasEvent()) {
    GateEvent e = context->takeEvent();
    Serial.print(F("# EVENT "));
    Serial.print(eventName(e));
    Serial.print(F("  free: "));
    Serial.println(context->getFreeSpots());
  }
}