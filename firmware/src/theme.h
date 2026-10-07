#pragma once
#include <lvgl.h>

// Design tokens — single source of truth for UI colors. Anthropic-inspired
// dark palette, AMOLED-friendly (true black bg).
//
// THEME_TFT_BRIGHT (set per env in platformio.ini) swaps in a lifted variant
// for cheap TN/TFT panels, where the AMOLED palette's near-black cards and
// muted greens disappear: lighter cards and tracks, full-white text, and
// saturated status colours with a distinct amber.
#ifdef THEME_TFT_BRIGHT
#define THEME_BG       lv_color_hex(0x000000)
#define THEME_PANEL    lv_color_hex(0x2e2d2a)
#define THEME_TEXT     lv_color_hex(0xffffff)
#define THEME_DIM      lv_color_hex(0xdedcd2)
#define THEME_ACCENT   lv_color_hex(0xff8a5c)
#define THEME_GREEN    lv_color_hex(0x8fd14f)
#define THEME_AMBER    lv_color_hex(0xffb020)
#define THEME_RED      lv_color_hex(0xff4a3d)
#define THEME_BAR_BG   lv_color_hex(0x4a4944)
#else
#define THEME_BG       lv_color_hex(0x000000)   // screen background
#define THEME_PANEL    lv_color_hex(0x1f1f1e)   // card/zone fill
#define THEME_TEXT     lv_color_hex(0xfaf9f5)   // primary text
#define THEME_DIM      lv_color_hex(0xb0aea5)   // secondary text
#define THEME_ACCENT   lv_color_hex(0xd97757)   // brand terra-cotta
#define THEME_GREEN    lv_color_hex(0x788c5d)
#define THEME_AMBER    lv_color_hex(0xd97757)
#define THEME_RED      lv_color_hex(0xc0392b)
#define THEME_BAR_BG   lv_color_hex(0x2a2a28)   // unfilled bar track
#endif
