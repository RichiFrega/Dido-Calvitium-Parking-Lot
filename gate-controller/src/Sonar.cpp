#include "Sonar.h"
#include "Arduino.h"
#include "config.h"

Sonar::Sonar(int triggerPin, int echoPin)
{
    this->triggerPin = triggerPin;
    this->echoPin = echoPin;
    this->nearCount = 0;
    this->farCount = 0;
    this->detected = false;

    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

void Sonar::sample()
{
    // impuso di trigger
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // durata dell'eco in microsecondi (0 = nessun eco entro il timeout)
    unsigned long duration = pulseIn(echoPin, HIGH, SONAR_TIMEOUT_US);

    bool near = (duration > 0) && (duration < SONAR_DETECT_US);

    if (near)
    {
        farCount = 0;
        if (nearCount < SONAR_CONFIRM_READS)
        {
            nearCount++;
        }
        if (nearCount >= SONAR_CONFIRM_READS)
        {
            detected = true;
        }
    }
    else
    {
        nearCount = 0;
        if (farCount < SONAR_RELEASE_READS)
        {
            farCount++;
        }
        if (farCount >= SONAR_RELEASE_READS)
        {
            detected = false;
        }
    }
}

bool Sonar::isDetected()
{
    // Ritorna lo stato filtrato
    return detected;
}