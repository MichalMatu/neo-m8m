#include "DisplayRenderer.h"

#include <Wire.h>

#include "AppConfig.h"
#include "AppLog.h"
#include "BatteryMonitor.h"

DisplayRenderer::DisplayRenderer()
    : display_(HardwareProfile::Oled.rotation,
               U8X8_PIN_NONE,
               HardwareProfile::Oled.sclPin,
               HardwareProfile::Oled.sdaPin)
{
}

void DisplayRenderer::begin()
{
    Wire.begin(HardwareProfile::Oled.sdaPin, HardwareProfile::Oled.sclPin);
    display_.begin();
    startedAtMs_ = millis();
    renderBootScreen();
    APP_LOGI("display", "OLED initialized on SDA=%u SCL=%u",
             HardwareProfile::Oled.sdaPin,
             HardwareProfile::Oled.sclPin);
}

void DisplayRenderer::drawLine(uint8_t row, const char *text)
{
    display_.drawStr(0, 8 + row * 10, text);
}

void DisplayRenderer::renderBootScreen()
{
    char line[24];

    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "MAX17048 MONITOR");
    drawLine(1, "Battery fuel gauge");
    snprintf(line, sizeof(line), "SDA%u SCL%u",
             HardwareProfile::Max17048I2c.sdaPin,
             HardwareProfile::Max17048I2c.sclPin);
    drawLine(2, line);
    snprintf(line, sizeof(line), "OLED %u/%u",
             HardwareProfile::Oled.sdaPin,
             HardwareProfile::Oled.sclPin);
    drawLine(3, line);
    snprintf(line, sizeof(line), "%lu baud",
             static_cast<unsigned long>(AppConfig::SerialBaud));
    drawLine(4, line);
    display_.sendBuffer();
}

void DisplayRenderer::renderStatusScreen()
{
    renderBatteryScreen();
}

void DisplayRenderer::renderBatteryScreen()
{
    char line[24];
    const BatteryMonitor::Snapshot battery = BatteryMonitor::snapshot();
    const max17048::Sample &sample = battery.sample;
    const int32_t percent = BatteryMonitor::percentTenths(sample);
    const int32_t rate = BatteryMonitor::chargeRateTenths(sample);

    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);

    if (sample.status == max17048::Status::Ok) {
        snprintf(line, sizeof(line), "BAT %ld.%ld%%",
                 static_cast<long>(percent / 10),
                 static_cast<long>(abs(percent % 10)));
    } else {
        snprintf(line, sizeof(line), "BAT %s", max17048::statusToString(sample.status));
    }
    drawLine(0, line);

    snprintf(line, sizeof(line), "V %lumV",
             static_cast<unsigned long>(BatteryMonitor::voltageMillivolts(sample)));
    drawLine(1, line);

    snprintf(line, sizeof(line), "Rate %ld.%ld%%/h",
             static_cast<long>(rate == INT32_MIN ? 0 : rate / 10),
             static_cast<long>(rate == INT32_MIN ? 0 : abs(rate % 10)));
    drawLine(2, line);

    snprintf(line, sizeof(line), "Alert 0x%02X", sample.alerts);
    drawLine(3, line);

    if (battery.sampleCount == 0) {
        snprintf(line, sizeof(line), "Age --");
    } else {
        snprintf(line, sizeof(line), "Age %lus",
                 static_cast<unsigned long>(BatteryMonitor::sampleAgeMs(battery) / 1000));
    }
    drawLine(4, line);
    display_.sendBuffer();
}

void DisplayRenderer::render()
{
    if (millis() - startedAtMs_ < AppConfig::BootScreenMs) {
        renderBootScreen();
        return;
    }

    renderStatusScreen();
}
