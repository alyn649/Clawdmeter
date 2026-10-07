#pragma once

// LCDWiki 2.8" ESP32-32E display module (E32R28T) — the "Cheap Yellow
// Display"-style board with an ESP32-32E module (classic ESP32-D0WD-V3,
// 4 MB flash, NO PSRAM), a 240x320 ILI9341 TFT over 4-wire SPI and an
// XPT2046 resistive touch controller on its own GPIOs. USB is a
// CH340 UART bridge. Run in landscape (320x240) so the UI lands on the
// 240-tall "small" layout breakpoint.

#define BOARD_NAME           "LCDWiki 2.8 ESP32-32E"

// ---- Display geometry (post-rotation, landscape) ----
#define LCD_WIDTH            320
#define LCD_HEIGHT           240
#define LCD_ROTATION         3     // Arduino_GFX rotation: 3 = landscape, USB on the left

// ---- LCD SPI bus (HSPI) ----
#define LCD_SCLK             14
#define LCD_MOSI             13
#define LCD_MISO             12
#define LCD_CS               15
#define LCD_DC               2
#define LCD_RST              GFX_NOT_DEFINED   // tied to EN
#define LCD_BL               21    // backlight, LEDC PWM, active HIGH
#define LCD_SPI_HZ           40000000
// Software gamma applied to every pixel before it reaches the panel. The
// shared palette is tuned for AMOLED (near-black cards, muted greens), which
// reads murky on this TN LCD; < 1.0 lifts mid-tones and dark greys while
// keeping true black black. 1.0 = passthrough. Mild, since the env also
// selects THEME_TFT_BRIGHT — mostly here for the splash art's palettes.
#define LCD_GAMMA            0.8f

// ---- Touch (XPT2046 resistive, its own pins — bit-banged SPI) ----
#define TP_SCLK              25
#define TP_MOSI              32
#define TP_MISO              39    // input-only pin
#define TP_CS                33
#define TP_IRQ               36    // PENIRQ, active LOW (input-only pin)
// Raw 12-bit ADC range of the panel edges, and the axis mapping into
// landscape pixels (rotation 3). Calibrated on hardware from corner taps
// (build with -DTOUCH_DEBUG to log raw samples): after the swap, raw X runs
// ~3550 at the left edge to ~500 at the right, raw Y ~3550 top to ~600 bottom.
#define TP_RAW_X_MIN         400
#define TP_RAW_X_MAX         3650
#define TP_RAW_Y_MIN         500
#define TP_RAW_Y_MAX         3650
#define TP_SWAP_XY           1
#define TP_INVERT_X          1
#define TP_INVERT_Y          1
#define TP_Z_THRESHOLD       400   // minimum pressure to count as a touch

// ---- RGB status LED (common anode, active LOW) — parked off ----
#define LED_R                22
#define LED_G                16
#define LED_B                17

// ---- Buttons ----
// Only BOOT (GPIO0) is user-accessible (the other button is RESET). It takes
// the PWR role — short press cycles animations / brightness, hold 3s +
// release enters pairing — since that's what the board needs most.
#define BTN_PWR_GPIO         0

// ---- Capability flags ----
#define BOARD_HAS_SECONDARY_BUTTON 0
#define BOARD_HAS_ROTATION         0
#define BOARD_HAS_IMU              0
#define BOARD_HAS_BATTERY          0
#define BOARD_HAS_IO_EXPANDER      0
#define BOARD_HAS_SOUND            0
