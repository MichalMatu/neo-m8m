# ESP Modules - LOLIN32 Battery Board

PlatformIO/Arduino base firmware for an ESP32 LOLIN32-style development board
with Wi-Fi, Bluetooth, LiPo battery input, and onboard USB charging.

This is the `board/esp32-lolin32-battery` branch. It is intentionally
module-free and display-free. Sensor and peripheral variants should branch from
this board baseline.

Planned module branch:

- `module/ld2420-lolin32-battery`: HLK-LD2420 24 GHz presence radar over UART.

## Board Reference

Target board:

- Product page: <https://probots.co.in/esp32-lolin32-wireless-development-board-wifi-bluetooth-battery-charger.html>
- ESP32 LOLIN32 wireless development board
- Wi-Fi and Bluetooth
- 4 MB flash
- LiPo battery interface through 2-pin JST
- onboard USB battery charger
- listed maximum charge current: 500 mA
- 3.3 V GPIO logic
- 26 digital I/O pins
- 12 analog I/O pins
- listed clock speed: 240 MHz
- listed PCB size: 58 mm x 25 mm

The product page has one inconsistent line that says `Microcontroller: ESP8266`,
but its title, description, and `Core Chipset` field identify the board as
ESP32. This branch treats the board as ESP32.

## Pins To Avoid

Avoid these ESP32 pins for add-on modules unless a module branch documents a
reason to use them:

- `GPIO1` / `GPIO3`: USB serial and upload
- `GPIO6` - `GPIO11`: ESP32 flash
- `GPIO0`, `GPIO2`, `GPIO12`, `GPIO15`: bootstrapping pins
- `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39`: input only

## Build And Upload

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

The base firmware uses only Serial Monitor at `115200` and one periodic
FreeRTOS diagnostics task.

## Project Layout

```text
include/
  AppConfig.h          board timings and task config
  AppTasks.h           FreeRTOS task bootstrap
  DiagnosticsLogger.h  Serial Monitor diagnostics API
src/
  AppTasks.cpp         task creation and task loops
  DiagnosticsLogger.cpp
  main.cpp             Arduino setup/loop entrypoint
```

## FreeRTOS Tasks

| Task | Core | Priority | Period | Responsibility |
| --- | ---: | ---: | ---: | --- |
| `serial-diag` | 0 | 1 | 5000 ms | Print periodic heartbeat diagnostics |

The Arduino `loop()` is intentionally idle and only calls `vTaskDelay()`.

## Branch Workflow

Use this branch as the starting point for modules built on the LOLIN32 battery
board:

```sh
git switch board/esp32-lolin32-battery
git switch -c module/<name>-lolin32-battery
```

Keep module-specific code, libraries, wiring notes, and troubleshooting in that
module branch.
