#include "board.h"
#include <Arduino.h>

// No I2C devices on this board (touch is bit-banged SPI), so no
// Wire.begin(). Park the RGB LED off and deselect the touch controller.
extern "C" void board_init(void) {
    pinMode(LED_R, OUTPUT); digitalWrite(LED_R, HIGH);
    pinMode(LED_G, OUTPUT); digitalWrite(LED_G, HIGH);
    pinMode(LED_B, OUTPUT); digitalWrite(LED_B, HIGH);

    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH);
}
