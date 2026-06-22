#include "DisplayRenderer.h"

#include <Wire.h>

#include "AppConfig.h"

namespace {

bool isButtonPressed(uint8_t pin)
{
    return digitalRead(pin) == AppConfig::ButtonPressedLevel;
}

bool pressedEdge(bool pressed, bool lastPressed)
{
    return pressed && !lastPressed;
}

void setupButtonPin(uint8_t pin)
{
    if (pin == 34 || pin == 35 || pin == 36 || pin == 39) {
        pinMode(pin, INPUT);
        return;
    }

    pinMode(pin, INPUT_PULLUP);
}

} // namespace

DisplayRenderer::DisplayRenderer(Ld2420Sensor &sensor)
    : sensor_(sensor),
      display_(U8G2_R0,
               U8X8_PIN_NONE,
               AppConfig::OledSclPin,
               AppConfig::OledSdaPin)
{
}

void DisplayRenderer::begin()
{
    setupButtonPin(AppConfig::Button1Pin);
    setupButtonPin(AppConfig::Button2Pin);
    setupButtonPin(AppConfig::Button3Pin);
    setupButtonPin(AppConfig::Button4Pin);

    Wire.begin(AppConfig::OledSdaPin, AppConfig::OledSclPin);
    display_.begin();
    startedAtMs_ = millis();
    renderBootScreen();
}

void DisplayRenderer::drawLine(uint8_t row, const char *text)
{
    display_.drawStr(0, 8 + row * 10, text);
}

void DisplayRenderer::handleButtons()
{
    const bool button1Pressed = isButtonPressed(AppConfig::Button1Pin);
    const bool button2Pressed = isButtonPressed(AppConfig::Button2Pin);
    const bool button3Pressed = isButtonPressed(AppConfig::Button3Pin);
    const bool button4Pressed = isButtonPressed(AppConfig::Button4Pin);

    if (pressedEdge(button1Pressed, lastButton1Pressed_)) {
        selectPreviousScreen();
    }
    if (pressedEdge(button2Pressed, lastButton2Pressed_)) {
        selectNextScreen();
    }
    if (pressedEdge(button3Pressed, lastButton3Pressed_)) {
        sensor_.resetStats();
    }
    if (pressedEdge(button4Pressed, lastButton4Pressed_)) {
        screen_ = screen_ == Screen::Main ? Screen::Details : Screen::Main;
    }

    lastButton1Pressed_ = button1Pressed;
    lastButton2Pressed_ = button2Pressed;
    lastButton3Pressed_ = button3Pressed;
    lastButton4Pressed_ = button4Pressed;
}

void DisplayRenderer::selectPreviousScreen()
{
    switch (screen_) {
    case Screen::Main:
        screen_ = Screen::Details;
        break;
    case Screen::Wiring:
        screen_ = Screen::Main;
        break;
    case Screen::Details:
        screen_ = Screen::Wiring;
        break;
    }
}

void DisplayRenderer::selectNextScreen()
{
    switch (screen_) {
    case Screen::Main:
        screen_ = Screen::Wiring;
        break;
    case Screen::Wiring:
        screen_ = Screen::Details;
        break;
    case Screen::Details:
        screen_ = Screen::Main;
        break;
    }
}

void DisplayRenderer::renderBootScreen()
{
    char line[24];

    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "LD2420 PRESENCE");
    snprintf(line, sizeof(line), "OUT GPIO%u", AppConfig::Ld2420OutPin);
    drawLine(1, line);
    snprintf(line, sizeof(line), "RX2 %u TX2 %u", AppConfig::Ld2420RxPin, AppConfig::Ld2420TxPin);
    drawLine(2, line);
    snprintf(line, sizeof(line), "UART %lu 8N1", static_cast<unsigned long>(AppConfig::Ld2420UartBaud));
    drawLine(3, line);
    snprintf(line, sizeof(line), "OLED SDA%u SCL%u", AppConfig::OledSdaPin, AppConfig::OledSclPin);
    drawLine(4, line);
    display_.sendBuffer();
}

void DisplayRenderer::renderMainScreen()
{
    char line[32];
    const Ld2420Sensor::Snapshot data = sensor_.snapshot();

    display_.clearBuffer();
    display_.setFont(u8g2_font_helvB14_tr);
    display_.drawStr(0, 17, data.presenceDetected ? "PRESENCE" : "CLEAR");

    display_.setFont(u8g2_font_5x8_tf);
    snprintf(line, sizeof(line), "OUT: %s", data.outHigh ? "HIGH" : "LOW");
    drawLine(2, line);
    snprintf(line, sizeof(line), "UART: %s", data.uartRecent ? "OK" : "--");
    drawLine(3, line);
    snprintf(line, sizeof(line), "BYTES: %lu", static_cast<unsigned long>(data.uartBytes));
    drawLine(4, line);
    if (data.lastDataAtMs > 0) {
        snprintf(line, sizeof(line), "LAST: %lus", static_cast<unsigned long>(data.lastDataAgeMs / 1000));
    } else {
        snprintf(line, sizeof(line), "LAST: --");
    }
    drawLine(5, line);
    display_.sendBuffer();
}

void DisplayRenderer::renderWiringScreen()
{
    char line[32];

    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "WIRING");
    snprintf(line, sizeof(line), "LD VCC 3V3 GND GND");
    drawLine(1, line);
    snprintf(line, sizeof(line), "LD TX -> RX2 GPIO%u", AppConfig::Ld2420RxPin);
    drawLine(2, line);
    snprintf(line, sizeof(line), "LD RX -> TX2 GPIO%u", AppConfig::Ld2420TxPin);
    drawLine(3, line);
    snprintf(line, sizeof(line), "LD OUT -> GPIO%u", AppConfig::Ld2420OutPin);
    drawLine(4, line);
    snprintf(line, sizeof(line), "OLED %u/%u", AppConfig::OledSdaPin, AppConfig::OledSclPin);
    drawLine(5, line);
    display_.sendBuffer();
}

void DisplayRenderer::renderDetailsScreen()
{
    char line[32];
    const Ld2420Sensor::Snapshot data = sensor_.snapshot();

    display_.clearBuffer();
    display_.setFont(u8g2_font_5x8_tf);
    drawLine(0, "LD2420 DETAILS");
    snprintf(line, sizeof(line), "Presence: %s", data.presenceDetected ? "yes" : "no");
    drawLine(1, line);
    snprintf(line, sizeof(line), "OUT level: %s", data.outHigh ? "HIGH" : "LOW");
    drawLine(2, line);
    snprintf(line, sizeof(line), "UART bytes: %lu", static_cast<unsigned long>(data.uartBytes));
    drawLine(3, line);
    if (data.lastDataAtMs > 0) {
        snprintf(line, sizeof(line), "UART age: %lums", static_cast<unsigned long>(data.lastDataAgeMs));
    } else {
        snprintf(line, sizeof(line), "UART age: --");
    }
    drawLine(4, line);
    drawLine(5, "B3 resets stats");
    display_.sendBuffer();
}

void DisplayRenderer::render()
{
    if (millis() - startedAtMs_ < AppConfig::BootScreenMs) {
        renderBootScreen();
        return;
    }

    handleButtons();

    switch (screen_) {
    case Screen::Main:
        renderMainScreen();
        break;
    case Screen::Wiring:
        renderWiringScreen();
        break;
    case Screen::Details:
        renderDetailsScreen();
        break;
    }
}
