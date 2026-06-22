#include "AppTasks.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "DiagnosticsLogger.h"
#include "DisplayRenderer.h"
#include "Ld2420Sensor.h"

namespace {

Ld2420Sensor ld2420Sensor;
DisplayRenderer displayRenderer(ld2420Sensor);

void ld2420Task(void *)
{
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        ld2420Sensor.update();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::Ld2420UpdateMs));
    }
}

void displayTask(void *)
{
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        displayRenderer.render();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::ScreenRefreshMs));
    }
}

void diagnosticsTask(void *)
{
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        DiagnosticsLogger::printHeartbeat(ld2420Sensor);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::DiagnosticLogMs));
    }
}

bool createPinnedTask(TaskFunction_t task,
                      const char *name,
                      uint32_t stackSize,
                      UBaseType_t priority,
                      BaseType_t core)
{
    return xTaskCreatePinnedToCore(
               task,
               name,
               stackSize,
               nullptr,
               priority,
               nullptr,
               core) == pdPASS;
}

} // namespace

bool startApplicationTasks()
{
    ld2420Sensor.begin();
    displayRenderer.begin();
    DiagnosticsLogger::printStartup();

    const bool sensorCreated = createPinnedTask(
        ld2420Task,
        "ld2420",
        AppConfig::Ld2420TaskStack,
        AppConfig::Ld2420TaskPriority,
        AppConfig::Ld2420TaskCore);

    const bool displayCreated = createPinnedTask(
        displayTask,
        "oled-render",
        AppConfig::DisplayTaskStack,
        AppConfig::DisplayTaskPriority,
        AppConfig::DisplayTaskCore);

    const bool diagnosticsCreated = createPinnedTask(
        diagnosticsTask,
        "serial-diag",
        AppConfig::DiagnosticsTaskStack,
        AppConfig::DiagnosticsTaskPriority,
        AppConfig::DiagnosticsTaskCore);

    if (!sensorCreated || !displayCreated || !diagnosticsCreated) {
        Serial.println("[fatal] FreeRTOS task creation failed");
        return false;
    }

    return true;
}
