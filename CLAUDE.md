# STM32F746-DISCO USB HID Gamepad with LVGL Touchscreen UI

## Project Overview

A **USB HID gamepad controller** built on the **STM32F746 Discovery board**. The 4.3" touchscreen (480x272, LTDC) displays game-specific dashboards via LVGL, and each on-screen button maps to a USB gamepad button. The goal is to use it as a physical button box for sim racing games (ETS2, Assetto Corsa, BeamNG Drive).

**Working directory:** `/home/luka/embedded-prjs/ac-workspace/stmf7-folder`

## Hardware

- **Board:** STM32F746NGHx Discovery (STM32F746G-DISCO)
- **MCU:** STM32F746 @ 216 MHz (Cortex-M7)
- **Display:** 4.3" TFT 480x272 RGB565 via LTDC, framebuffers in external SDRAM
- **Touch:** FT5336 capacitive touch controller on I2C3 (PH7=SCL, PH8=SDA, addr 0x70)
- **SDRAM:** IS42S32400F, 8MB at 0xC0000000 via FMC
- **USB:** USB_OTG_FS in Device mode (CN13 port) — Custom HID gamepad class

## Toolchain & Build

- **IDE:** VSCode with STM32CubeIDE extension
- **Code gen:** STM32CubeMX (`.ioc` file: `stmf7-folder.ioc`)
- **Build:** CMake + arm-none-eabi-gcc
- **Build commands:** `cmake --preset Debug` then `cmake --build build/Debug`
- **Flash:** VSCode command palette → `STM32: Flash STM32`

## Key Architecture

### File Structure (user-created files)

```
Core/Src/sdram.c        / Core/Inc/sdram.h         — SDRAM init sequence (CubeMX doesn't generate this)
Core/Src/lv_port_disp.c / Core/Inc/lv_port_disp.h  — LVGL display driver (LTDC double-buffered)
Core/Src/lv_port_indev.c/ Core/Inc/lv_port_indev.h — LVGL touch driver (FT5336 via I2C3)
Core/Src/gamepad.c      / Core/Inc/gamepad.h       — USB HID gamepad state + report sending
Core/Src/ui_screens.c   / Core/Inc/ui_screens.h    — All LVGL UI screens
Core/Inc/lv_conf.h                                  — LVGL configuration (copied from template)
```

### CMakeLists.txt (user sources section)

User source files are added in the root `CMakeLists.txt` under `target_sources`. LVGL is added via `add_subdirectory(Middlewares/Third_Party/lvgl)` and linked with `target_link_libraries`. LVGL needs extra include dirs for FreeRTOS/HAL headers — these are set via `target_include_directories(lvgl SYSTEM PUBLIC ...)`.

## Critical: CubeMX Regeneration Checklist

**Every time you regenerate code from CubeMX, you MUST manually fix these:**

### 1. Linker Script (`STM32F746NGHx_FLASH.ld`)

Add SDRAM memory region:
```
MEMORY
{
  RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 320K
  FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 1024K
  SDRAM (rw)      : ORIGIN = 0xC0000000, LENGTH = 8M     ← ADD THIS
}
```

Add `.sdram` section before `/DISCARD/`:
```
  .sdram (NOLOAD) :
  {
    . = ALIGN(4);
    *(.sdram)
    *(.sdram*)
    . = ALIGN(4);
  } >SDRAM
```

Without this, the two 510KB framebuffers land in RAM (320KB) → linker error "RAM 159%".

### 2. USB Descriptor Size (`USB_DEVICE/Target/usbd_conf.h`)

Change: `#define USBD_CUSTOM_HID_REPORT_DESC_SIZE  2U` → `41U`

### 3. USB Device Strings (`USB_DEVICE/App/usbd_desc.c`)

```c
#define USBD_VID     0x0483
#define USBD_MANUFACTURER_STRING     "STM32 Gamepad"
#define USBD_PID_FS     0x5750
#define USBD_PRODUCT_STRING_FS     "STM32 Game Controller"
#define USBD_CONFIGURATION_STRING_FS     "Gamepad Config"
#define USBD_INTERFACE_STRING_FS     "Gamepad Interface"
```

## LVGL Setup Details

- **Version:** v9.2.2 (git submodule at `Middlewares/Third_Party/lvgl`, checked out to tag `v9.2.2`)
- **Color depth:** 16-bit RGB565 (matches LTDC)
- **OS mode:** `LV_USE_OS = LV_OS_NONE` (CubeMX FreeRTOS is too old for LVGL's FreeRTOS OSAL — missing `atomic.h`)
- **Memory:** `LV_MEM_SIZE = 48KB` (LVGL internal heap)
- **Perf monitor:** `LV_USE_SYSMON = 1`, `LV_USE_PERF_MONITOR = 1` (FPS/CPU% in bottom-right)
- **Rendering:** Software renderer (`LV_USE_DRAW_SW`), no DMA2D (not available in LVGL 9.2.2)
- **Tick source:** `lv_tick_set_cb(HAL_GetTick)`
- **Config location:** `Core/Inc/lv_conf.h` (passed to LVGL CMake via `LV_CONF_PATH`)

### Display Driver (`lv_port_disp.c`)

- Two framebuffers in SDRAM: `fb1[480*272]` and `fb2[480*272]` with `__attribute__((section(".sdram")))`
- Direct render mode: `LV_DISPLAY_RENDER_MODE_DIRECT`
- Flush callback: `HAL_LTDC_SetAddress(&hltdc, (uint32_t)px_map, 0)` — just swaps LTDC buffer pointer

### Touch Driver (`lv_port_indev.c`)

- FT5336 at I2C address 0x70 on I2C3
- **Axis mapping:** FT5336 X/Y are swapped relative to display:
  - `display_x = raw_y`
  - `display_y = raw_x`
- Raw coordinate ranges: X: 0-278, Y: 0-474 (close to 1:1 with 272x480 display)
- Exposes `touch_raw_x` / `touch_raw_y` globals for debug display

### SDRAM Init (`sdram.c`)

CubeMX only generates FMC register config — it does NOT send the SDRAM chip init sequence. `SDRAM_Init()` sends: Clock Enable → PALL → Auto-Refresh (x8) → Load Mode Register → Set Refresh Rate (1542). Must be called before LVGL uses framebuffers.

## USB HID Gamepad

### Report Descriptor (41 bytes, in `usbd_custom_hid_if.c`)

- **Report ID:** 1
- **16 buttons** (2 bytes, each bit = one button)
- **2 axes** (X, Y) — signed 8-bit, range -127 to +127, currently always 0 (reserved for future joystick)
- **Total report size:** 5 bytes `[ReportID] [buttons_lo] [buttons_hi] [X] [Y]`

### Gamepad Logic (`gamepad.c`)

- `gamepad_set_button(bit_index, active)` — sets/clears a bit in `gamepad_buttons` (uint16_t)
- `gamepad_send_report()` — sends 5-byte HID report via `USBD_CUSTOM_HID_SendReport_FS()`
- Send function is in `usbd_custom_hid_if.c` (uncommented from CubeMX template)

### Button Mapping (ETS2)

| Button ID | Gamepad Bit | Function  | Linux Event |
|-----------|-------------|-----------|-------------|
| 0         | bit 0       | Hazard    | BTN_SOUTH   |
| 1         | bit 1       | Trailer   | BTN_EAST    |
| 2         | bit 2       | Lights    | BTN_C       |
| 3         | bit 3       | Wipers    | BTN_NORTH   |
| 4-15      | bits 4-15   | Available | —           |

## UI Screens (`ui_screens.c`)

### Home Screen (`scr_home`)
- Dark blue background (0x1A1A2E)
- Title: "Select Game"
- Three game buttons: ETS2 (blue), Assetto Corsa (red), BeamNG (orange)
- Buttons spaced at x = -150, 0, +150 from center

### ETS2 Dashboard (`scr_ets2`)
- Dark navy background (0x0D1B2A)
- Title: "Euro Truck Simulator 2"
- 2x2 grid of **momentary** buttons (grey=idle, green=pressed)
  - Hazard (0), Trailer (1), Lights (2), Wipers (3)
  - Each sends HID press on `LV_EVENT_PRESSED`, release on `LV_EVENT_RELEASED`
- Back button (top-left, arrow icon)

### Assetto Corsa & BeamNG
- Placeholder screens with "Coming soon..." text and back button

### Helper Functions
- `create_momentary_btn(parent, text, w, h, button_id)` — creates a button that sends gamepad press/release
- `create_placeholder_screen(title)` — creates a simple "coming soon" screen with back button
- `ui_create_screens()` — builds all screens, called once from `StartDefaultTask`
- `ui_get_home_screen()` — returns home screen pointer for initial load

## FreeRTOS

- CMSIS-RTOS v1 API (CubeMX default)
- Single task: `defaultTask` with 4096 words (16KB) stack
- Task does: SDRAM init → USB Device init → LVGL init → create screens → `lv_timer_handler()` loop (5ms delay)
- LVGL is only called from this one task, so `LV_USE_OS = LV_OS_NONE` is safe

## Resource Usage (last build)

| Resource | Used | Total | Free |
|----------|------|-------|------|
| FLASH | 449 KB | 1024 KB | 575 KB |
| RAM | 91 KB | 320 KB | 229 KB |
| SDRAM | 510 KB | 8 MB | 7.5 MB |

## Testing

- **Linux:** `sudo evtest` — select STM32 device, see button press/release events
- **Linux:** `lsusb | grep STM` — device shows as `0483:5750`
- **Windows:** "Set up USB game controllers" in Control Panel — shows 16 buttons + 2 axes

## Pending Tasks

- [ ] Fill out **Assetto Corsa** dashboard screen with game-specific buttons
- [ ] Fill out **BeamNG Drive** dashboard screen with game-specific buttons
- [ ] Test and bind buttons in actual games on Windows
- [ ] Consider adding **SPI device** support (SPI2 is configured but unused)
- [ ] Consider MPU configuration to mark SDRAM framebuffer as non-cacheable (prevents potential display artifacts if D-Cache is enabled)
- [ ] Consider upgrading FreeRTOS to v10.4+ to enable `LV_USE_OS = LV_OS_FREERTOS` (if multi-task LVGL access is ever needed)
- [ ] LTDC vsync-based flush (call `lv_display_flush_ready()` from LTDC line interrupt instead of immediately — for tear-free rendering)
