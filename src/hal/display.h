#pragma once
#include <TFT_eSPI.h>

// ==========================================================================
// Display HAL -- TFT + sprite management + backlight
// ==========================================================================

namespace Display {

void init();
void push();    // Push framebuffer to TFT

void setBrightness(uint8_t level);  // 0=low, 1=mid, 2=high

TFT_eSPI&    tft();
TFT_eSprite& fb();             // 240x240 framebuffer
TFT_eSprite& petSprite();      // 115x110 pet overlay
TFT_eSprite& effectSprite();   // 100x95 effect overlay

}  // namespace Display
