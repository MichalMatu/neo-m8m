#include "DiagnosticsLogger.h"

#include <Arduino.h>

#include "AppConfig.h"

namespace DiagnosticsLogger {

namespace {

bool isButtonPressed(uint8_t pin)
{
    return digitalRead(pin) == AppConfig::ButtonPressedLevel;
}

} // namespace

void printStartup()
{
    Serial.println();
    Serial.println("ESP32 LOLIN32 OLED buttons LD2420 firmware");
    Serial.printf("Serial Monitor: %lu\n",
                  static_cast<unsigned long>(AppConfig::SerialBaud));
    Serial.println("Board: ESP32 LOLIN32 with LiPo charger");
    Serial.println("Power: OLED VCC=3V3 GND=GND; LD2420 VCC=3V3 GND=GND");
    Serial.printf("OLED I2C: SDA=%u SCL=%u\n",
                  AppConfig::OledSdaPin,
                  AppConfig::OledSclPin);
    Serial.printf("Buttons: B1=%u B2=%u B3=%u B4=%u active=%s\n",
                  AppConfig::Button1Pin,
                  AppConfig::Button2Pin,
                  AppConfig::Button3Pin,
                  AppConfig::Button4Pin,
                  AppConfig::ButtonPressedLevel == LOW ? "low" : "high");
    Serial.println("Buttons note: GPIO34/GPIO35 need external or module pull-ups");
    Serial.printf("LD2420 UART: module TX -> ESP32 RX2 GPIO%u, module RX -> ESP32 TX2 GPIO%u, %lu 8N1\n",
                  AppConfig::Ld2420RxPin,
                  AppConfig::Ld2420TxPin,
                  static_cast<unsigned long>(AppConfig::Ld2420UartBaud));
    Serial.printf("LD2420 OUT: GPIO%u, HIGH means presence detected\n",
                  AppConfig::Ld2420OutPin);
    Serial.println("Scheduler: FreeRTOS LD2420, display, and diagnostics tasks");
}

void printHeartbeat(const Ld2420Sensor &sensor)
{
    const Ld2420Sensor::Snapshot data = sensor.snapshot();
    char age[16];

    if (data.lastDataAtMs > 0) {
        snprintf(age, sizeof(age), "%lums", static_cast<unsigned long>(data.lastDataAgeMs));
    } else {
        snprintf(age, sizeof(age), "--");
    }

    Serial.printf("[diag] uptime=%lums presence=%s out=%s uart=%s bytes=%lu last_uart_age=%s b1=%u b2=%u b3=%u b4=%u\n",
                  static_cast<unsigned long>(millis()),
                  data.presenceDetected ? "yes" : "no",
                  data.outHigh ? "HIGH" : "LOW",
                  data.uartRecent ? "OK" : "--",
                  static_cast<unsigned long>(data.uartBytes),
                  age,
                  isButtonPressed(AppConfig::Button1Pin),
                  isButtonPressed(AppConfig::Button2Pin),
                  isButtonPressed(AppConfig::Button3Pin),
                  isButtonPressed(AppConfig::Button4Pin));
}

} // namespace DiagnosticsLogger
