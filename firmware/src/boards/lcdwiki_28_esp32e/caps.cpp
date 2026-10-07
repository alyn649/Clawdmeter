#include "../../hal/board_caps.h"
#include "board.h"

static const BoardCaps caps = {
    .name = BOARD_NAME,
    .width = LCD_WIDTH,
    .height = LCD_HEIGHT,
    // BOOT is used as the PWR-role button (power.cpp), so no HID buttons.
    .button_count = 0,
    .has_rotation = (bool)BOARD_HAS_ROTATION,
    .has_battery  = (bool)BOARD_HAS_BATTERY,
    .has_imu      = (bool)BOARD_HAS_IMU,
};

const BoardCaps& board_caps(void) { return caps; }
