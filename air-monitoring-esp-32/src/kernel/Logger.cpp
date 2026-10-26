#include "Logger.h"

LoggerService Logger;

void LoggerService::log(const String& msg) {
    Serial.println("lo:" + msg);
}