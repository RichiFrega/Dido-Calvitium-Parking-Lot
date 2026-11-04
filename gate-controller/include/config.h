#ifndef __CONFIG__
#define __CONFIG__

#include <Arduino.h>

/* PIN */
constexpr uint8_t PIN_BUTTON       = 2;     // apertura manuale (INT0)
constexpr uint8_t PIN_PIR          = 3;     // PIR (INT1)

constexpr uint8_t PIN_SONAR_A_TRIG = 4;     // sonar A: lato ingresso
constexpr uint8_t PIN_SONAR_A_ECHO = 5;
constexpr uint8_t PIN_SONAR_B_TRIG = 6;     // sonar B: lato interno
constexpr uint8_t PIN_SONAR_B_ECHO = 7;

constexpr uint8_t PIN_SERVO        = 9;
constexpr uint8_t PIN_BUZZER       = 10;    // buzzer attivo
constexpr uint8_t PIN_LED_GREEN    = 11;
constexpr uint8_t PIN_LED_RED      = 12;

/* SERIALE */
constexpr unsigned long SERIAL_BAUD = 115200UL;

/* SONAR */
constexpr unsigned long SONAR_TIMEOUT_US    = 1500UL;   // da verificare con le misure a 3-4 cm
constexpr unsigned long SONAR_US_PER_CM     = 58UL;     // andata e ritorno
constexpr unsigned long SONAR_DETECT_CM     = 8UL;      // veicolo a 3-4 cm, margine ~2x
constexpr unsigned long SONAR_DETECT_US     = SONAR_DETECT_CM * SONAR_US_PER_CM;
constexpr uint8_t       SONAR_CONFIRM_READS = 2;        // letture per dichiarare "presente"
constexpr uint8_t       SONAR_RELEASE_READS = 3;        // letture per dichiarare "libero"

/* SBARRA */
constexpr uint8_t       BARRIER_ANGLE_CLOSED = 35;
constexpr uint8_t       BARRIER_ANGLE_OPEN   = 155;
constexpr unsigned long BARRIER_TRAVEL_MS    = 650UL;  // ms misurato + margine

/* PULSANTE */
constexpr unsigned long BUTTON_DEBOUNCE_MS = 50UL;

/* TEMPI */
constexpr unsigned long T1_MS        = 10000UL;  // timeout del transito
constexpr unsigned long GATE_TASK_MS = 50UL;     // periodo del gatetask

/* DISPLAY */
constexpr uint8_t LCD_I2C_ADDRESS = 0x27;        // da verificare una volta comprato l'I2C
constexpr uint8_t LCD_COLUMNS     = 16;
constexpr uint8_t LCD_ROWS        = 2;

/* eventi */
constexpr uint8_t EVENT_QUEUE_SIZE = 8;         // events waiting for the CommTaks

/* TEST (senza cu) */
constexpr uint8_t TEST_INITIAL_FREE_SPOTS = 5;   // solo finché la cu non gestisce i posti

#endif