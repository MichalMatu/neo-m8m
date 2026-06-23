#include "BatteryMonitor.h"

#include <cmath>

#include "AppConfig.h"
#include "AppLog.h"
#include "HardwareProfile.h"

namespace BatteryMonitor {

namespace {

constexpr const char *LogTag = "battery";
TwoWire batteryWire(1);
max17048::Monitor monitor(batteryWire);
portMUX_TYPE sampleMux = portMUX_INITIALIZER_UNLOCKED;

Snapshot latest = {
    {0,
     max17048::Status::DeviceNotReady,
     {HardwareProfile::Max17048I2c, MAX17048_I2CADDR_DEFAULT, 0, 0},
     NAN,
     NAN,
     NAN,
     false,
     false,
     0},
    0,
};

int32_t scaledTenths(float value)
{
    if (std::isnan(value)) {
        return INT32_MIN;
    }

    return static_cast<int32_t>(std::lround(value * 10.0f));
}

void publish(const max17048::Sample &sample)
{
    portENTER_CRITICAL(&sampleMux);
    latest.sample = sample;
    latest.sampleCount++;
    portEXIT_CRITICAL(&sampleMux);
}

void publishStatus(max17048::Status status)
{
    max17048::Sample sample;
    sample.timestampMs = millis();
    sample.status = status;
    sample.device = monitor.deviceInfo();
    sample.voltage = NAN;
    sample.percent = NAN;
    sample.chargeRate = NAN;
    sample.hibernating = false;
    sample.activeAlert = false;
    sample.alerts = 0;
    publish(sample);
}

void waitForDevice()
{
    for (;;) {
        const max17048::Status status = monitor.begin(HardwareProfile::Max17048I2c);
        publishStatus(status);

        if (status == max17048::Status::Ok) {
            APP_LOGI(LogTag, "MAX17048 ready chip=0x%02X version=0x%04X",
                     monitor.deviceInfo().chipId,
                     monitor.deviceInfo().icVersion);
            vTaskDelay(pdMS_TO_TICKS(AppConfig::BatteryStartupDelayMs));
            return;
        }

        APP_LOGW(LogTag, "MAX17048 init status=%s", max17048::statusToString(status));
        vTaskDelay(pdMS_TO_TICKS(AppConfig::BatteryRetryIntervalMs));
    }
}

} // namespace

void runTask(void *)
{
    waitForDevice();

    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        max17048::Sample sample;
        const max17048::Status status = monitor.read(sample);
        publish(sample);

        if (status != max17048::Status::Ok) {
            APP_LOGW(LogTag, "read status=%s", max17048::statusToString(status));
            waitForDevice();
            lastWake = xTaskGetTickCount();
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::BatteryReadIntervalMs));
    }
}

Snapshot snapshot()
{
    portENTER_CRITICAL(&sampleMux);
    const Snapshot copy = latest;
    portEXIT_CRITICAL(&sampleMux);
    return copy;
}

uint32_t sampleAgeMs(const Snapshot &snapshot)
{
    if (snapshot.sample.timestampMs == 0) {
        return UINT32_MAX;
    }

    return millis() - snapshot.sample.timestampMs;
}

uint32_t voltageMillivolts(const max17048::Sample &sample)
{
    if (std::isnan(sample.voltage)) {
        return 0;
    }

    return static_cast<uint32_t>(std::lround(sample.voltage * 1000.0f));
}

int32_t percentTenths(const max17048::Sample &sample)
{
    return scaledTenths(sample.percent);
}

int32_t chargeRateTenths(const max17048::Sample &sample)
{
    return scaledTenths(sample.chargeRate);
}

void logStartup()
{
    APP_LOGI(LogTag, "MAX17048 I2C SDA=%u SCL=%u clock=%lu",
             HardwareProfile::Max17048I2c.sdaPin,
             HardwareProfile::Max17048I2c.sclPin,
             static_cast<unsigned long>(HardwareProfile::Max17048I2c.clockHz));
}

void logHeartbeat()
{
    char alertText[96];
    const Snapshot current = snapshot();
    const max17048::Sample &sample = current.sample;
    const int32_t percent = percentTenths(sample);
    const int32_t rate = chargeRateTenths(sample);

    APP_LOGI(LogTag, "samples=%lu age=%lums status=%s voltage=%lumV soc=%ld.%ld%% rate=%ld.%ld%%/h alerts=0x%02X:%s",
             static_cast<unsigned long>(current.sampleCount),
             static_cast<unsigned long>(sampleAgeMs(current)),
             max17048::statusToString(sample.status),
             static_cast<unsigned long>(voltageMillivolts(sample)),
             static_cast<long>(percent == INT32_MIN ? 0 : percent / 10),
             static_cast<long>(percent == INT32_MIN ? 0 : std::abs(percent % 10)),
             static_cast<long>(rate == INT32_MIN ? 0 : rate / 10),
             static_cast<long>(rate == INT32_MIN ? 0 : std::abs(rate % 10)),
             sample.alerts,
             max17048::alertFlagsToString(sample.alerts, alertText, sizeof(alertText)));
}

} // namespace BatteryMonitor
