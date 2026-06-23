#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

namespace HardwareProfile {

inline constexpr const char *ProjectName = "MAX17048 MONITOR";
inline constexpr const char *BoardProfileName = "TTGO ESP32 OLED 18650";

struct OledProfile {
    uint8_t sdaPin;
    uint8_t sclPin;
    const u8g2_cb_t *rotation;
};

struct I2cProfile {
    uint8_t sdaPin;
    uint8_t sclPin;
    uint32_t clockHz;
    uint16_t timeoutMs;
};

inline constexpr OledProfile Oled = {
    5,
    4,
    U8G2_R2,
};

inline constexpr I2cProfile Max17048I2c = {
    19,
    23,
    100000,
    50,
};

} // namespace HardwareProfile
