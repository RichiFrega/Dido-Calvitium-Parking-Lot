#include <Arduino.h>
#include "config.h"
#include "Scheduler.h"
#include "Sonar.h"
#include "servo_motor_impl.h"
#include "Led.h"
#include "BuzzerImpl.h"
#include "ButtonEvent.h"
#include "ParkingContext.h"
#include "GateTask.h"
#include "ParkingTask.h"
#include "AlarmTask.h"
#include "LcdDisplay.h"

// SOLO PER TEST!!!

class FakeProximitySensor : public ProximitySensor {
public:
  FakeProximitySensor() { detected = false; }
  void sample() { }
  bool isDetected() { return detected; }
  void toggle() { detected = !detected; }
private:
  bool detected;
};

/* true = sensor driven by the keyboard, false = real sonar */
constexpr bool FAKE_A = false;
constexpr bool FAKE_B = false;

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
    case EV_BUTTON_OPEN: return F("BTN:OPEN");
    case EV_SLEEP:       return F("SLEEP");
    case EV_WAKE:        return F("WAKE");
    default:             return F("NONE");
  }
}

void printStatus() {
  Serial.print(F("# free: "));      Serial.print(context->getFreeSpots());
  Serial.print(F("  alarm: "));     Serial.print(context->isAlarmOn());
  Serial.print(F("  closed: "));    Serial.print(context->isParkingClosed());
  Serial.print(F("  offline: "));   Serial.print(context->isOffline());
  Serial.print(F(" (e: "));         Serial.print(context->getOfflineEntries());
  Serial.print(F(" u: "));          Serial.print(context->getOfflineExits());
  Serial.print(F(")  canEnter: ")); Serial.print(context->canEnter());
  if (fakeA != nullptr) { Serial.print(F("  A: ")); Serial.print(fakeA->isDetected()); }
  if (fakeB != nullptr) { Serial.print(F("  B: ")); Serial.print(fakeB->isDetected()); }
  Serial.println();
}

void printHelp() {
  Serial.println(F("# keys: a/b = toggle fake sensor A/B"));
  Serial.println(F("#       f = full, r = reset free spots"));
  Serial.println(F("#       x = alarm on/off, p = parking closed/open"));
  Serial.println(F("#       n = offline on/off, o = manual open (CMD:OPEN)"));
  Serial.println(F("#       s = status, h = help"));
}

void handleCommand(char c) {
  switch (c) {
    case 'a': if (fakeA != nullptr) { fakeA->toggle(); } printStatus(); break;
    case 'b': if (fakeB != nullptr) { fakeB->toggle(); } printStatus(); break;
    case 'f': context->setFreeSpots(0); printStatus(); break;
    case 'r': context->setFreeSpots(TEST_INITIAL_FREE_SPOTS); printStatus(); break;
    case 'x': context->setAlarm(!context->isAlarmOn()); printStatus(); break;
    case 'p': context->setParkingClosed(!context->isParkingClosed()); printStatus(); break;
    case 'n': context->setOffline(!context->isOffline()); printStatus(); break;
    case 'o': context->requestOpen(); break;
    case 's': printStatus(); break;
    case 'h': printHelp(); break;
    default: break;
  }
}

ProximitySensor* makeSensor(bool fake, FakeProximitySensor** fakeOut, int trig, int echo) {
  if (fake) {
    *fakeOut = new FakeProximitySensor();
    return *fakeOut;
  }
  return new Sonar(trig, echo);
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  /* devices */
  ProximitySensor* sensorA = makeSensor(FAKE_A, &fakeA, PIN_SONAR_A_TRIG, PIN_SONAR_A_ECHO);
  ProximitySensor* sensorB = makeSensor(FAKE_B, &fakeB, PIN_SONAR_B_TRIG, PIN_SONAR_B_ECHO);
  ServoMotor* barrier = new ServoMotorImpl(PIN_SERVO);
  Light* greenLed = new Led(PIN_LED_GREEN);
  Light* redLed = new Led(PIN_LED_RED);
  Buzzer* buzzer = new BuzzerImpl(PIN_BUZZER);
  buttonEventInit(PIN_BUTTON);

  /* shared state */
  context = new ParkingContext(TEST_INITIAL_FREE_SPOTS);

  /* tasks: base period = GCD of the periods (50 ms) */
  sched.init(GATE_TASK_MS);

  Task* gateTask = new GateTask(sensorA, sensorB, barrier, context);
  gateTask->init(GATE_TASK_MS);
  sched.addTask(gateTask);

  Display* display = new LcdDisplay(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

  Task* parkingTask = new ParkingTask(greenLed, redLed, context, display);
  parkingTask->init(PARKING_TASK_MS);
  sched.addTask(parkingTask);

  Task* alarmTask = new AlarmTask(buzzer, context);
  alarmTask->init(ALARM_TASK_MS);
  sched.addTask(alarmTask);

  Serial.println(F("# Gate Controller test started"));
  printHelp();
  printStatus();
}

void loop() {
  sched.schedule();

  /* manual-open button (interrupt flag) */
  if (buttonEventTake()) {
    Serial.println(F("# button pressed"));
    context->registerButtonOpen();
    context->requestOpen();
  }

  /* keyboard commands */
  while (Serial.available()) {
    handleCommand(Serial.read());
  }

  /* temporary: the CommTask will send these events to the CU */
  while (context->hasEvent()) {
    GateEvent e = context->takeEvent();
    Serial.print(F("# EVENT "));
    Serial.print(eventName(e));
    Serial.print(F("  free: "));
    Serial.println(context->getFreeSpots());
  }
}