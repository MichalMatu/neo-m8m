#pragma once

#include <Adafruit_MAX1704X.h>
#include <Arduino.h>
#include <Wire.h>

#include "HardwareProfile.h"

namespace max17048 {

struct DeviceInfo {
    HardwareProfile::I2cProfile i2c;
    uint8_t address;
    uint8_t chipId;
    uint16_t icVersion;
};

enum class Status : uint8_t {
    Ok,
    BusError,
    I2cNotDetected,
    DeviceNotReady,
    ReadError,
};

struct Sample {
    uint32_t timestampMs;
    Status status;
    DeviceInfo device;
    float voltage;
    float percent;
    float chargeRate;
    bool hibernating;
    bool activeAlert;
    uint8_t alerts;
};

class Monitor {
public:
    explicit Monitor(TwoWire &wire);

    Status begin(const HardwareProfile::I2cProfile &config);
    Status read(Sample &sample);
    bool ready() const;
    const DeviceInfo &deviceInfo() const;

private:
    bool addressPresent();
    void resetSample(Sample &sample, Status status) const;

    TwoWire &wire_;
    Adafruit_MAX17048 fuelGauge_;
    DeviceInfo info_;
    bool ready_;
};

const char *statusToString(Status status);
const char *alertFlagsToString(uint8_t flags, char *buffer, size_t bufferSize);

} // namespace max17048
