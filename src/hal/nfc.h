#pragma once
#include "config.h"
#include <cstdint>

// ==========================================================================
// NFC -- PN532 I2C tap detect, multi-tap, long hold
// Compiles to no-op when FEATURE_NFC == 0
// ==========================================================================

namespace NFC {

enum TapType : uint8_t {
    TAP_NONE,
    TAP_SINGLE,     // Wake screen, show SCREEN_GLANCE
    TAP_DOUBLE,     // Quick approve top pending item
    TAP_TRIPLE,     // Full SCREEN_REVIEW mode
    TAP_LONG_HOLD,  // Emergency lockdown confirmation (3s)
};

void init();
void tick();

TapType consumeTap();       // Returns tap type and resets
bool isTagPresent();        // True while tag in field

// Tag UID from last read
const uint8_t* lastUID();
uint8_t lastUIDLen();

}  // namespace NFC
