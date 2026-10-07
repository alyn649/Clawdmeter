#include "../../hal/input_hal.h"
#include "board.h"

// BOOT (GPIO0) is the only user button and it's owned by power.cpp as the
// PWR-role button, so there are no HID buttons on this board.

void input_hal_init(void) {}

bool input_hal_is_held(InputButton btn) {
    (void)btn;
    return false;
}
