#pragma once
#include <cstdint>

// ==========================================================================
// LEDs HAL -- NeoPixel patterns + brightness
// ==========================================================================

namespace LEDs {

void init();
void tick();    // Call every loop iteration

void off();
void happy();       // soft violet
void sad();         // dim red
void wifi();        // blue
void rest();        // calm deep-blue
void breathe(unsigned long phase);  // breathing cycle for deep sleep

void setBrightness(uint8_t level);  // 0=low, 1=mid, 2=high
void setEnabled(bool on);
bool isEnabled();

}  // namespace LEDs
