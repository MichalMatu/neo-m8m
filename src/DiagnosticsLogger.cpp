#include "DiagnosticsLogger.h"

#include <Arduino.h>

#include "AppConfig.h"

namespace DiagnosticsLogger {

void printStartup()
{
    Serial.println();
    Serial.println("ESP32 LOLIN32 battery base firmware");
    Serial.printf("Serial Monitor: %lu\n",
                  static_cast<unsigned long>(AppConfig::SerialBaud));
    Serial.println("Board: ESP32 LOLIN32 with LiPo charger");
    Serial.println("Scheduler: FreeRTOS diagnostics task");
}

void printHeartbeat()
{
    Serial.printf("[diag] uptime=%lums board=lolin32-battery\n",
                  static_cast<unsigned long>(millis()));
}

} // namespace DiagnosticsLogger
