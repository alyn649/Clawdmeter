#include "../../hal/touch_hal.h"
#include "board.h"
#include <Arduino.h>

// Minimal bit-banged XPT2046 reader (vendored to keep the dependency tree
// copyleft-free). The touch controller sits on its own GPIOs, so no SPI
// peripheral is needed — a full sample is ~100 µs of GPIO toggling.
// 12-bit differential conversions:
//   0xB1 / 0xC1 — Z1 / Z2 pressure, 0x91 — Y, 0xD1 — X (last cmd 0xD0 powers
//   the ADC down so PENIRQ stays armed between reads).
// PENIRQ gates the sampling: nothing is clocked while nobody touches.

#define TOUCH_SAMPLES 3

static uint16_t last_x = 0, last_y = 0;

static uint16_t xfer12(uint8_t cmd) {
    for (int i = 7; i >= 0; i--) {
        digitalWrite(TP_MOSI, (cmd >> i) & 1);
        digitalWrite(TP_SCLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(TP_SCLK, LOW);
        delayMicroseconds(1);
    }
    digitalWrite(TP_MOSI, LOW);
    uint16_t v = 0;
    for (int i = 0; i < 16; i++) {
        digitalWrite(TP_SCLK, HIGH);
        delayMicroseconds(1);
        v = (v << 1) | digitalRead(TP_MISO);
        digitalWrite(TP_SCLK, LOW);
        delayMicroseconds(1);
    }
    return v >> 3;
}

static bool sample(uint16_t* rx, uint16_t* ry, int* z) {
    digitalWrite(TP_CS, LOW);

    int z1 = xfer12(0xB1);
    int z2 = xfer12(0xC1);
    *z = z1 + 4095 - z2;

    uint32_t sx = 0, sy = 0;
    xfer12(0x91);   // discard first Y — settling
    for (int i = 0; i < TOUCH_SAMPLES; i++) {
        sy += xfer12(0x91);
        sx += xfer12(0xD1);
    }
    xfer12(0xD0);   // power down, PENIRQ enabled

    digitalWrite(TP_CS, HIGH);

    *rx = sx / TOUCH_SAMPLES;
    *ry = sy / TOUCH_SAMPLES;
    return z1 > 0 && *z >= TP_Z_THRESHOLD;
}

static int map_axis(int v, int lo, int hi, int out_max, bool invert) {
    int p = (int)((long)(v - lo) * out_max / (hi - lo));
    if (p < 0) p = 0;
    if (p > out_max - 1) p = out_max - 1;
    return invert ? out_max - 1 - p : p;
}

void touch_hal_init(void) {
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH);
    pinMode(TP_SCLK, OUTPUT);
    digitalWrite(TP_SCLK, LOW);
    pinMode(TP_MOSI, OUTPUT);
    pinMode(TP_MISO, INPUT);
    pinMode(TP_IRQ, INPUT);
    uint16_t rx, ry;
    int z;
    sample(&rx, &ry, &z);   // one dummy conversion leaves the ADC in PENIRQ mode
    Serial.println("Touch XPT2046 (bit-banged)");
}

void touch_hal_read(uint16_t* x, uint16_t* y, bool* pressed) {
    *pressed = false;
    if (digitalRead(TP_IRQ) == LOW) {
        uint16_t rx, ry;
        int z;
        bool down = sample(&rx, &ry, &z);
#if TP_SWAP_XY
        uint16_t t = rx; rx = ry; ry = t;
#endif
        if (down) {
            last_x = map_axis(rx, TP_RAW_X_MIN, TP_RAW_X_MAX, LCD_WIDTH,  TP_INVERT_X);
            last_y = map_axis(ry, TP_RAW_Y_MIN, TP_RAW_Y_MAX, LCD_HEIGHT, TP_INVERT_Y);
            *pressed = true;
        }
#ifdef TOUCH_DEBUG
        static uint32_t last_log = 0;
        if (millis() - last_log > 200) {
            last_log = millis();
            Serial.printf("touch raw=%u,%u z=%d down=%d -> %u,%u\n",
                          rx, ry, z, down, last_x, last_y);
        }
#endif
    }
    *x = last_x;
    *y = last_y;
}
