#pragma once
#include <cstdint>

// ==========================================================================
// Buttons HAL -- Edge detection for 6-button layout
// ==========================================================================

namespace Buttons {

void init();
void tick();    // Call every loop iteration (reads pins, detects edges)

// True for one tick on falling edge (press detected)
bool upPressed();
bool okPressed();
bool downPressed();
bool r1Pressed();
bool r2Pressed();
bool r3Pressed();

}  // namespace Buttons
