#pragma once

#include <Arduino.h>

#include "Max17048Monitor.h"

namespace BatteryMonitor {

struct Snapshot {
    max17048::Sample sample;
    uint32_t sampleCount;
};

void runTask(void *context);
Snapshot snapshot();
uint32_t sampleAgeMs(const Snapshot &snapshot);
uint32_t voltageMillivolts(const max17048::Sample &sample);
int32_t percentTenths(const max17048::Sample &sample);
int32_t chargeRateTenths(const max17048::Sample &sample);
void logStartup();
void logHeartbeat();

} // namespace BatteryMonitor
