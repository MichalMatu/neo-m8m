#pragma once

#include <Arduino.h>

class Ld2420Sensor {
public:
    struct Snapshot {
        bool presenceDetected = false;
        bool outHigh = false;
        bool uartRecent = false;
        uint32_t uartBytes = 0;
        uint32_t lastDataAtMs = 0;
        uint32_t lastDataAgeMs = 0;
    };

    void begin();
    void update();
    void resetStats();
    Snapshot snapshot() const;

private:
    mutable portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
    bool presenceDetected_ = false;
    bool outHigh_ = false;
    uint32_t uartBytes_ = 0;
    uint32_t lastDataAtMs_ = 0;
};
