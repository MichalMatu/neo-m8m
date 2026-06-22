# ESP Modules - LOLIN32 Battery OLED Buttons

PlatformIO/Arduino firmware branch for an ESP32 LOLIN32-style battery board
with an external 0.96" SSD1306 I2C OLED module and four integrated buttons.

This is the `module/oled-buttons-lolin32-battery` branch. It starts from the
`board/esp32-lolin32-battery` baseline and is reserved for the OLED/button
panel wiring and firmware.

Display/button pinout is intentionally left open until the actual wiring is
chosen.

## Board Reference

Target board is an ESP32 LOLIN32-style battery board. Use the seller page only
as the purchase reference; it has a weak spec table. Better references for this
board family are:

- Seller page: <https://probots.co.in/esp32-lolin32-wireless-development-board-wifi-bluetooth-battery-charger.html>
- WEMOS D32 documentation: <https://www.wemos.cc/en/latest/d32/d32.html>
- Zephyr LOLIN32 Lite board docs: <https://github.com/zephyrproject-rtos/zephyr/blob/main/boards/wemos/lolin32_lite/doc/index.rst>
- Arduino ESP32 `lolin32` pin variant: <https://github.com/espressif/arduino-esp32/blob/master/variants/lolin32/pins_arduino.h>

Board summary from the WEMOS/Zephyr/Arduino references:

- ESP32 LOLIN32 wireless development board
- Wi-Fi and Bluetooth
- 4 MB flash
- LiPo battery interface through PH-2 / 2-pin JST-style connector
- onboard USB battery charger; WEMOS D32 lists 500 mA max charging current
- 3.3 V GPIO logic
- 240 MHz max clock
- WEMOS D32 lists 57 mm x 25.4 mm board size
- Arduino `lolin32` variant maps `LED_BUILTIN` and default `SS` to `GPIO5`
- Arduino `lolin32` variant maps default I2C to `SDA=GPIO21`, `SCL=GPIO22`
- Arduino `lolin32` variant maps default SPI to `SCK=GPIO18`,
  `MISO=GPIO19`, `MOSI=GPIO23`

The product page has one inconsistent line that says `Microcontroller: ESP8266`,
but its title, description, and `Core Chipset` field identify the board as
ESP32. This branch treats the board as ESP32.

## OLED Button Module

Module reference:

- Product page: <https://mikrobot.pl/Modul-Wyswietlacz-OLED-096-bialy-I2C-SSD1306-plus-4-przyciski-do-Arduino>
- 0.96" white OLED
- 128 x 64 pixels
- SSD1306 controller
- I2C display interface
- 3.3 V supply
- four integrated programmable buttons
- listed dimensions: 27.5 mm x 44.6 mm
- listed weight: 11.5 g

Planned wiring:

| OLED/button module | LOLIN32 ESP32 |
| --- | --- |
| VCC / 3V3 | TBD |
| GND | TBD |
| SDA | TBD |
| SCL | TBD |
| Button 1 | TBD |
| Button 2 | TBD |
| Button 3 | TBD |
| Button 4 | TBD |

The Arduino `lolin32` variant uses `SDA=GPIO21` and `SCL=GPIO22` as default
I2C pins, but this branch will follow the actual wiring used on the build.

## Pins To Avoid

Avoid these ESP32 pins for add-on modules unless a module branch documents a
reason to use them:

- `GPIO1` / `GPIO3`: USB serial and upload
- `GPIO6` - `GPIO11`: ESP32 flash
- `GPIO5`: onboard LED / default SPI SS on the Arduino `lolin32` variant
- `GPIO0`, `GPIO2`, `GPIO12`, `GPIO15`: bootstrapping pins
- `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39`: input only

## Build And Upload

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

For now, this branch still uses only Serial Monitor at `115200` and one
periodic FreeRTOS diagnostics task. OLED/button firmware will be added after
the final pinout is known.

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
