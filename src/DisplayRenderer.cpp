#include "DisplayRenderer.h"

#include "AppConfig.h"
#include "I2cBus.h"

DisplayRenderer::DisplayRenderer()
    : display_(U8G2_R2,
               U8X8_PIN_NONE,
               AppConfig::OledSclPin,
               AppConfig::OledSdaPin)
{
}

void DisplayRenderer::begin()
{
    I2cBus::begin();
    I2cBus::lock();
    display_.begin();
    I2cBus::unlock();
    startedAtMs_ = millis();
    renderBootScreen();
}

void DisplayRenderer::drawLine(uint8_t row, const char *text)
{
    display_.drawStr(0, 8 + row * 10, text);
}

void DisplayRenderer::renderBootScreen()
{
    I2cBus::lock();
    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "ESP32 MAX30100");
    drawLine(1, "BPM + SpO2");
    drawLine(2, "I2C addr 0x57");
    drawLine(3, "OLED SDA5 SCL4");
    drawLine(4, "Place finger");
    display_.sendBuffer();
    I2cBus::unlock();
}

void DisplayRenderer::renderSensorMissingScreen(const Max30100Snapshot &snapshot)
{
    char line[24];

    I2cBus::lock();
    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "MAX30100 FAIL");
    drawLine(1, "Check I2C/power");
    snprintf(line, sizeof(line), "Addr 0x%02X",
             AppConfig::Max30100Address);
    drawLine(2, line);
    snprintf(line, sizeof(line), "Last try %lus",
             static_cast<unsigned long>(snapshot.lastInitAttemptAtMs / 1000));
    drawLine(3, line);
    drawLine(4, "Retrying...");
    display_.sendBuffer();
    I2cBus::unlock();
}

void DisplayRenderer::renderWaitingFingerScreen(const Max30100Snapshot &snapshot)
{
    char line[24];

    I2cBus::lock();
    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "PLACE FINGER");
    drawLine(1, "BPM --.-");
    drawLine(2, "SpO2 --%");
    snprintf(line, sizeof(line), "Beats %lu",
             static_cast<unsigned long>(snapshot.beatCount));
    drawLine(3, line);
    snprintf(line, sizeof(line), "LED bias %u", snapshot.redLedCurrentBias);
    drawLine(4, line);
    display_.sendBuffer();
    I2cBus::unlock();
}

void DisplayRenderer::renderMeasurementScreen(const Max30100Snapshot &snapshot)
{
    char bpmValue[8];
    char spo2Value[8];
    char statusLine[32];

    if (snapshot.heartRateValid) {
        const uint16_t bpm = static_cast<uint16_t>(snapshot.heartRateBpm + 0.5f);
        snprintf(bpmValue, sizeof(bpmValue), "%u", bpm);
    } else {
        snprintf(bpmValue, sizeof(bpmValue), "--");
    }

    if (snapshot.spo2Valid) {
        snprintf(spo2Value, sizeof(spo2Value), "%u", snapshot.spo2);
    } else {
        snprintf(spo2Value, sizeof(spo2Value), "--");
    }

    I2cBus::lock();
    display_.clearBuffer();

    display_.setFont(u8g2_font_5x8_tf);
    display_.drawStr(0, 8, "bpm:");
    display_.drawStr(72, 8, "spo2:");
    display_.drawVLine(63, 0, 46);

    display_.setFont(u8g2_font_logisoso22_tf);
    display_.drawStr(0, 40, bpmValue);
    display_.drawStr(72, 40, spo2Value);

    display_.setFont(u8g2_font_5x8_tf);
    display_.drawHLine(0, 51, 128);
    snprintf(statusLine, sizeof(statusLine), "beat:%s cnt:%lu led:%u",
             snapshot.freshBeat ? "*" : "-",
             static_cast<unsigned long>(snapshot.beatCount),
             snapshot.redLedCurrentBias);
    display_.drawStr(0, 63, statusLine);
    display_.sendBuffer();
    I2cBus::unlock();
}

void DisplayRenderer::render(const Max30100Snapshot &snapshot)
{
    if (millis() - startedAtMs_ < AppConfig::BootScreenMs) {
        renderBootScreen();
        return;
    }

    if (!snapshot.sensorFound) {
        renderSensorMissingScreen(snapshot);
        return;
    }

    if (!snapshot.heartRateValid && !snapshot.spo2Valid) {
        renderWaitingFingerScreen(snapshot);
        return;
    }

    renderMeasurementScreen(snapshot);
}
