#include "DisplayRenderer.h"

#include <Wire.h>

#include "AppConfig.h"

DisplayRenderer::DisplayRenderer()
    : display_(U8G2_R2,
               U8X8_PIN_NONE,
               AppConfig::OledSclPin,
               AppConfig::OledSdaPin)
{
}

void DisplayRenderer::begin()
{
    Wire.begin(AppConfig::OledSdaPin, AppConfig::OledSclPin);
    display_.begin();
    startedAtMs_ = millis();
    renderBootScreen();
}

void DisplayRenderer::drawLine(uint8_t row, const char *text)
{
    display_.drawStr(0, 8 + row * 10, text);
}

void DisplayRenderer::renderBootScreen()
{
    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "ESP32 BATT OLED");
    drawLine(1, "Base firmware");
    drawLine(2, "No module code");
    drawLine(3, "OLED SDA5 SCL4");
    drawLine(4, "Serial 115200");
    display_.sendBuffer();
}

void DisplayRenderer::renderStatusScreen()
{
    char line[24];

    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "BASE READY");
    snprintf(line, sizeof(line), "Uptime %lus",
             static_cast<unsigned long>(millis() / 1000));
    drawLine(1, line);
    snprintf(line, sizeof(line), "OLED SDA%u SCL%u",
             AppConfig::OledSdaPin,
             AppConfig::OledSclPin);
    drawLine(2, line);
    drawLine(3, "Module variants");
    drawLine(4, "use branches");
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
