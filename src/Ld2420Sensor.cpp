#include "Ld2420Sensor.h"

#include "AppConfig.h"

void Ld2420Sensor::begin()
{
    pinMode(AppConfig::Ld2420OutPin, INPUT);
    Serial2.begin(AppConfig::Ld2420UartBaud,
                  SERIAL_8N1,
                  AppConfig::Ld2420RxPin,
                  AppConfig::Ld2420TxPin);
    update();
}

void Ld2420Sensor::update()
{
    const bool outHigh = digitalRead(AppConfig::Ld2420OutPin) == HIGH;
    const uint32_t now = millis();
    uint32_t bytesRead = 0;

    while (Serial2.available() > 0) {
        Serial2.read();
        ++bytesRead;
    }

    portENTER_CRITICAL(&mux_);
    outHigh_ = outHigh;
    presenceDetected_ = outHigh;
    if (bytesRead > 0) {
        uartBytes_ += bytesRead;
        lastDataAtMs_ = now;
    }
    portEXIT_CRITICAL(&mux_);
}

void Ld2420Sensor::resetStats()
{
    portENTER_CRITICAL(&mux_);
    uartBytes_ = 0;
    lastDataAtMs_ = 0;
    portEXIT_CRITICAL(&mux_);
}

Ld2420Sensor::Snapshot Ld2420Sensor::snapshot() const
{
    Snapshot data;
    const uint32_t now = millis();

    portENTER_CRITICAL(&mux_);
    data.presenceDetected = presenceDetected_;
    data.outHigh = outHigh_;
    data.uartBytes = uartBytes_;
    data.lastDataAtMs = lastDataAtMs_;
    portEXIT_CRITICAL(&mux_);

    if (data.lastDataAtMs > 0) {
        data.lastDataAgeMs = now - data.lastDataAtMs;
        data.uartRecent = data.lastDataAgeMs <= AppConfig::Ld2420UartRecentMs;
    }

    return data;
}
