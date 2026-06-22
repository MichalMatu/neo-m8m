# ESP32 Battery OLED

PlatformIO/Arduino base firmware for an ESP32 18650 board with an onboard
0.96" 128x64 I2C OLED.

The `main` branch is intentionally module-free. It provides the shared board
baseline: serial startup logs, OLED initialization, a simple status screen, and
FreeRTOS task wiring. Sensor and peripheral variants should live on dedicated
branches.

Current module branches:

- `module/gps`: u-blox NEO-M8M GPS over UART2 with TinyGPSPlus parsing and OLED diagnostics.

## Board Reference

Target board: TTGO WiFi & Bluetooth Battery ESP32 0.96 Inch OLED Development Tool.

Product photos and board details below are from the
[HiTechChain product page](https://hitechchain.se/iot/ttgo/esp32-pico-kit-utvecklingsbord).

![TTGO ESP32 OLED 18650 board front](docs/images/ttgo-battery-oled-front.jpeg)

![TTGO ESP32 OLED 18650 board dimensions](docs/images/ttgo-battery-oled-dimensions.jpeg)

![TTGO ESP32 OLED 18650 board battery side](docs/images/ttgo-battery-oled-back.jpeg)

![TTGO ESP32 OLED board with OLED running](docs/images/ttgo-battery-oled-screen.jpeg)

![TTGO ESP32 OLED 18650 board pinout](docs/images/ttgo-battery-oled-pinout.jpeg)

![TTGO ESP32 OLED board alternate front view](docs/images/ttgo-battery-oled-front-screen.jpeg)

Short board specification:

- ESP32 development board with Wi-Fi and Bluetooth.
- ESP32-WROOM-32 module on the board revision shown in the product photos.
- Integrated 0.96" OLED.
- 18650 cell holder and onboard charging system.
- USB, 5 V, or 18650 power input paths.
- Charging and running at the same time are supported by the board.
- Charge indicator LED: red while charging, green when full.
- Power switch on the board.
- Extra programmable LED connected to `GPIO16` / `D0`.
- Listed charging current: 0.5 A.
- Listed output current: 1 A.
- Over-charge and over-discharge protection.
- Full ESP32 pin breakout.
- Product image marks approximate board dimensions as 114 mm x 28.25 mm x 17.6 mm.

## Hardware

Target board:

- ESP32 OLED 18650 development board
- ESP32-WROOM-32
- Onboard OLED 0.96" 128x64 I2C
- USB, 5 V, or 18650 power
- 3.3 V GPIO logic

Onboard OLED:

| OLED | ESP32 |
| --- | --- |
| SDA | GPIO5 |
| SCL | GPIO4 |

Pins to avoid for add-on modules unless a branch documents otherwise:

- `GPIO1` / `GPIO3`: USB serial and upload
- `GPIO6` - `GPIO11`: ESP32 flash
- `GPIO16`: onboard programmable LED on this board
- `GPIO0`, `GPIO2`, `GPIO12`, `GPIO15`: bootstrapping pins
- `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39`: input only

## Build And Upload

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

The base firmware uses:

- OLED I2C: `SDA=GPIO5`, `SCL=GPIO4`
- Serial Monitor: `115200`
- FreeRTOS tasks for OLED rendering and periodic serial diagnostics

## Project Layout

```text
include/
  AppConfig.h          board pins, timings, task stack sizes and priorities
  AppTasks.h           FreeRTOS task bootstrap
  DiagnosticsLogger.h  Serial Monitor diagnostics API
  DisplayRenderer.h    OLED rendering API
src/
  AppTasks.cpp         task creation and task loops
  DiagnosticsLogger.cpp
  DisplayRenderer.cpp
  main.cpp             Arduino setup/loop entrypoint
lib/
  U8g2/                local vendored OLED library
```

## FreeRTOS Tasks

| Task | Core | Priority | Period | Responsibility |
| --- | ---: | ---: | ---: | --- |
| `oled-render` | 1 | 2 | 500 ms | Render boot and base status screens |
| `serial-diag` | 0 | 1 | 5000 ms | Print periodic heartbeat diagnostics |

The Arduino `loop()` is intentionally idle and only calls `vTaskDelay()`.

## Local Libraries

U8g2 is stored in `lib/`, so the project builds from a local copy instead of
downloading the library into `.pio/libdeps`.

To refresh U8g2 manually, temporarily add it back to `lib_deps`, run `pio run`,
then copy the resolved package from `.pio/libdeps/esp32dev/` into `lib/U8g2`.

Module branches may add their own local libraries when needed.

## OLED Screens

At boot, the OLED shows the repository baseline name, OLED pins, and Serial
Monitor speed.

After the boot screen, the OLED shows:

- base firmware ready status
- uptime
- OLED I2C pins
- reminder that module variants live on branches

All OLED screens use only the first five text rows to avoid the damaged bottom
line on this board.

## Branch Workflow

Use `main` as the starting point for a new hardware configuration:

```sh
git switch main
git switch -c module/<name>
```

Keep module-specific code, libraries, wiring notes, and troubleshooting in that
module branch. When replacing an internal API or config shape inside a branch,
migrate its current callers and remove the old path in the same change.

## Troubleshooting

OLED does not display:

- This project starts with `U8G2_SSD1306_128X64_NONAME_F_HW_I2C`.
- If your board uses a SH1106 or another controller, switch the U8g2 constructor in `src/DisplayRenderer.cpp` to the matching `U8G2_SH1106_128X64_*_HW_I2C` variant.
- Confirm OLED pins `SDA=GPIO5` and `SCL=GPIO4` for your exact board revision.
- This board variant renders correctly with `U8G2_R2`, which rotates the OLED by 180 degrees. Use `U8G2_R0` if your display is mounted the other way.
