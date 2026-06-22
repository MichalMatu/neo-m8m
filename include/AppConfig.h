#pragma once

#include <Arduino.h>

namespace AppConfig {

inline constexpr uint32_t SerialBaud = 115200;

inline constexpr uint32_t DiagnosticLogMs = 5000;
inline constexpr uint32_t IdleTaskDelayMs = 1000;

inline constexpr uint32_t DiagnosticsTaskStack = 4096;

inline constexpr UBaseType_t DiagnosticsTaskPriority = 1;

inline constexpr BaseType_t DiagnosticsTaskCore = 0;

} // namespace AppConfig
