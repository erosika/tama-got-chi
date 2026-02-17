#pragma once
#include "types.h"
#include "../hal/nfc.h"

// ==========================================================================
// NFC Actions -- Map NFC taps to pet/screen behavior
// ==========================================================================

namespace NfcActions {

struct Result {
    Screen targetScreen = SCREEN_HOME;
    bool locationOverride = false;
    LocationZone overrideZone = LOC_UNKNOWN;
};

// Process a tap event, optionally matching tag UIDs for location
Result process(NFC::TapType tap, const uint8_t* uid, uint8_t uidLen,
               const Settings& settings);

}  // namespace NfcActions
