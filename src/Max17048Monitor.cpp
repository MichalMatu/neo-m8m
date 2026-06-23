#include "Max17048Monitor.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace max17048 {

namespace {

constexpr uint8_t I2cAddress = MAX17048_I2CADDR_DEFAULT;

void appendFlag(char *buffer, size_t bufferSize, bool &hasValue, const char *name)
{
    if (bufferSize == 0) {
        return;
    }

    const size_t used = strlen(buffer);
    if (used >= bufferSize - 1) {
        return;
    }

    if (hasValue) {
        strncat(buffer, "|", bufferSize - strlen(buffer) - 1);
    }
    strncat(buffer, name, bufferSize - strlen(buffer) - 1);
    hasValue = true;
}

} // namespace

Monitor::Monitor(TwoWire &wire)
    : wire_(wire),
      info_{{0, 0, 0, 0}, I2cAddress, 0, 0},
      ready_(false)
{
}

Status Monitor::begin(const HardwareProfile::I2cProfile &config)
{
    ready_ = false;
    info_ = {config, I2cAddress, 0, 0};

    wire_.end();
    vTaskDelay(pdMS_TO_TICKS(20));

    if (!wire_.begin(config.sdaPin, config.sclPin, config.clockHz)) {
        return Status::BusError;
    }

    wire_.setTimeOut(config.timeoutMs);
    if (!wire_.setClock(config.clockHz)) {
        return Status::BusError;
    }

    if (!addressPresent()) {
        return Status::I2cNotDetected;
    }

    if (!fuelGauge_.begin(&wire_)) {
        info_.chipId = fuelGauge_.getChipID();
        info_.icVersion = fuelGauge_.getICversion();
        return Status::DeviceNotReady;
    }

    info_.chipId = fuelGauge_.getChipID();
    info_.icVersion = fuelGauge_.getICversion();
    ready_ = true;
    return Status::Ok;
}

Status Monitor::read(Sample &sample)
{
    resetSample(sample, ready_ ? Status::Ok : Status::DeviceNotReady);

    if (!ready_) {
        return sample.status;
    }

    if (!fuelGauge_.isDeviceReady()) {
        ready_ = false;
        sample.status = Status::DeviceNotReady;
        return sample.status;
    }

    const float voltage = fuelGauge_.cellVoltage();
    const float percent = fuelGauge_.cellPercent();
    const float chargeRate = fuelGauge_.chargeRate();

    if (std::isnan(voltage) || std::isnan(percent) || std::isnan(chargeRate)) {
        sample.status = Status::ReadError;
        return sample.status;
    }

    sample.voltage = voltage;
    sample.percent = percent;
    sample.chargeRate = chargeRate;
    sample.hibernating = fuelGauge_.isHibernating();
    sample.activeAlert = fuelGauge_.isActiveAlert();
    sample.alerts = sample.activeAlert ? fuelGauge_.getAlertStatus() : 0;
    return sample.status;
}

bool Monitor::ready() const
{
    return ready_;
}

const DeviceInfo &Monitor::deviceInfo() const
{
    return info_;
}

bool Monitor::addressPresent()
{
    wire_.beginTransmission(I2cAddress);
    return wire_.endTransmission() == 0;
}

void Monitor::resetSample(Sample &sample, Status status) const
{
    sample.timestampMs = millis();
    sample.status = status;
    sample.device = info_;
    sample.voltage = NAN;
    sample.percent = NAN;
    sample.chargeRate = NAN;
    sample.hibernating = false;
    sample.activeAlert = false;
    sample.alerts = 0;
}

const char *statusToString(Status status)
{
    switch (status) {
    case Status::Ok:
        return "ok";
    case Status::BusError:
        return "bus_error";
    case Status::I2cNotDetected:
        return "i2c_not_detected";
    case Status::DeviceNotReady:
        return "device_not_ready";
    case Status::ReadError:
        return "read_error";
    }

    return "unknown";
}

const char *alertFlagsToString(uint8_t flags, char *buffer, size_t bufferSize)
{
    if (bufferSize == 0) {
        return "";
    }

    buffer[0] = '\0';
    bool hasValue = false;

    if (flags & MAX1704X_ALERTFLAG_SOC_CHANGE) {
        appendFlag(buffer, bufferSize, hasValue, "soc_change");
    }
    if (flags & MAX1704X_ALERTFLAG_SOC_LOW) {
        appendFlag(buffer, bufferSize, hasValue, "soc_low");
    }
    if (flags & MAX1704X_ALERTFLAG_VOLTAGE_RESET) {
        appendFlag(buffer, bufferSize, hasValue, "voltage_reset");
    }
    if (flags & MAX1704X_ALERTFLAG_VOLTAGE_LOW) {
        appendFlag(buffer, bufferSize, hasValue, "voltage_low");
    }
    if (flags & MAX1704X_ALERTFLAG_VOLTAGE_HIGH) {
        appendFlag(buffer, bufferSize, hasValue, "voltage_high");
    }
    if (flags & MAX1704X_ALERTFLAG_RESET_INDICATOR) {
        appendFlag(buffer, bufferSize, hasValue, "reset_indicator");
    }

    if (!hasValue) {
        strncat(buffer, "none", bufferSize - strlen(buffer) - 1);
    }

    return buffer;
}

} // namespace max17048
