#include "../../hal/display_hal.h"
#include "board.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <math.h>

// ILI9341 over 4-wire SPI. Brightness is a LEDC PWM duty on the backlight
// (the TFT has no brightness command), which keeps the idle fade working.
// Pixels pass through a per-channel gamma LUT (LCD_GAMMA in board.h) to lift
// the AMOLED-tuned dark palette on this TN panel.

static Arduino_DataBus* bus = nullptr;
static Arduino_ILI9341* gfx = nullptr;

static uint16_t lut_r[32], lut_g[64], lut_b[32];   // pre-shifted into place

static void build_gamma_lut(void) {
    for (int i = 0; i < 32; i++) {
        uint16_t v = (uint16_t)lroundf(powf(i / 31.0f, LCD_GAMMA) * 31.0f);
        lut_r[i] = v << 11;
        lut_b[i] = v;
    }
    for (int i = 0; i < 64; i++) {
        lut_g[i] = (uint16_t)lroundf(powf(i / 63.0f, LCD_GAMMA) * 63.0f) << 5;
    }
}

void display_hal_init(void) {
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, LCD_MISO, HSPI);
    gfx = new Arduino_ILI9341(bus, LCD_RST, LCD_ROTATION, false /* ips */);
}

void display_hal_begin(void) {
    build_gamma_lut();
    gfx->begin(LCD_SPI_HZ);
    gfx->fillScreen(0x0000);
    ledcAttach(LCD_BL, 5000 /* Hz */, 8 /* bits */);
    ledcWrite(LCD_BL, 200);
}

void display_hal_set_brightness(uint8_t level) {
    ledcWrite(LCD_BL, level);
}

void display_hal_fill_screen(uint16_t color) {
    if (gfx) gfx->fillScreen(color);
}

void display_hal_draw_bitmap(int32_t x, int32_t y, int32_t w, int32_t h,
                             const uint16_t* pixels) {
    if (!gfx) return;
    // Remap in place: callers (LVGL flush strips, splash band buffer) fully
    // re-render their buffer before every draw, so it's safe to overwrite.
    uint16_t* p = (uint16_t*)pixels;
    for (int32_t i = 0, n = w * h; i < n; i++) {
        uint16_t c = p[i];
        p[i] = lut_r[c >> 11] | lut_g[(c >> 5) & 0x3F] | lut_b[c & 0x1F];
    }
    gfx->draw16bitRGBBitmap(x, y, p, w, h);
}

void display_hal_tick(void) {}

void display_hal_round_area(int32_t* x1, int32_t* y1, int32_t* x2, int32_t* y2) {
    (void)x1; (void)y1; (void)x2; (void)y2;
}
