#include "../../hal/sound_hal.h"

// The board has a small DAC-driven speaker amp, but it isn't wired up to the
// chime engine (which targets the ES8311 codec), so sound is a no-op.

void sound_hal_init(void) {}
void sound_hal_tick(void) {}
void sound_hal_play_reset(void) {}
