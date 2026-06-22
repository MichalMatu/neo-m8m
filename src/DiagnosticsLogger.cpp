#include "DiagnosticsLogger.h"

#include <Arduino.h>

#include "AppConfig.h"

namespace DiagnosticsLogger {

void printStartup()
{
    Serial.println();
    Serial.println("ESP32 battery OLED base firmware");
    Serial.printf("OLED I2C: SDA=%u SCL=%u\n",
                  AppConfig::OledSdaPin,
                  AppConfig::OledSclPin);
    Serial.printf("Serial Monitor: %lu\n",
                  static_cast<unsigned long>(AppConfig::SerialBaud));
    Serial.println("Scheduler: FreeRTOS display and diagnostics tasks");
}

void printHeartbeat()
{
    Serial.printf("[diag] uptime=%lums oled=sda%u/scl%u\n",
                  static_cast<unsigned long>(millis()),
                  AppConfig::OledSdaPin,
                  AppConfig::OledSclPin);
}

} // namespace DiagnosticsLogger
