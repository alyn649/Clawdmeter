#include "../../hal/power_hal.h"
#include "board.h"
#include <Arduino.h>

// No PMU and no battery measurement. The BOOT button (active-LOW GPIO0)
// plays the PWR role, with the same software edge synthesis as the
// LCD-1.54 port:
//   short    — fired on release if the hold was shorter than PWR_LONG_MS
//   long     — fired once when a hold crosses PWR_LONG_MS
//   release  — fired on every release edge
// so main.cpp's hold-to-pair gesture works unchanged. There is no
// software power-off — the board has no power latch.

#define PWR_POLL_MS      50
#define PWR_LONG_MS      1500

static bool     pwr_pressed_flag  = false;
static bool     pwr_long_flag     = false;
static bool     pwr_released_flag = false;
static bool     last_pwr_state    = false;
static uint32_t pwr_press_started_ms = 0;
static bool     pwr_long_fired    = false;
static uint32_t last_pwr_ms       = 0;

void power_hal_init(void) {
    pinMode(BTN_PWR_GPIO, INPUT_PULLUP);
}

void power_hal_tick(void) {
    uint32_t now = millis();
    if (now - last_pwr_ms < PWR_POLL_MS) return;
    last_pwr_ms = now;

    bool pwr_now = (digitalRead(BTN_PWR_GPIO) == LOW);
    if (pwr_now && !last_pwr_state) {            // press edge — hold begins
        pwr_press_started_ms = now;
        pwr_long_fired = false;
    } else if (pwr_now && last_pwr_state) {      // held
        if (!pwr_long_fired && (now - pwr_press_started_ms >= PWR_LONG_MS)) {
            pwr_long_flag  = true;
            pwr_long_fired = true;
        }
    } else if (!pwr_now && last_pwr_state) {     // release edge
        pwr_released_flag = true;
        if (!pwr_long_fired) pwr_pressed_flag = true;  // short press
    }
    last_pwr_state = pwr_now;
}

int  power_hal_battery_pct(void) { return -1; }
bool power_hal_is_charging(void) { return false; }
bool power_hal_is_vbus_in(void)  { return true; }

bool power_hal_pwr_pressed(void) {
    if (pwr_pressed_flag) { pwr_pressed_flag = false; return true; }
    return false;
}

bool power_hal_pwr_long_pressed(void) {
    if (pwr_long_flag) { pwr_long_flag = false; return true; }
    return false;
}

bool power_hal_pwr_released(void) {
    if (pwr_released_flag) { pwr_released_flag = false; return true; }
    return false;
}
