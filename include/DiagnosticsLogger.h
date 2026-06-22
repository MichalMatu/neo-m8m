#pragma once

#include "Ld2420Sensor.h"

namespace DiagnosticsLogger {

void printStartup();
void printHeartbeat(const Ld2420Sensor &sensor);

} // namespace DiagnosticsLogger
