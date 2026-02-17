#include "nfc_actions.h"
#include <cstring>

// ==========================================================================
// NFC Actions -- Tap -> behavior mapping
//
//   Single tap    -> SCREEN_GLANCE (wake + priority view)
//   Double tap    -> SCREEN_REVIEW (quick approve)
//   Triple tap    -> SCREEN_REVIEW (full review mode)
//   Long hold 3s  -> SCREEN_SETTINGS (emergency/lockdown)
//
// If the tag UID matches a registered location tag, override location.
// ==========================================================================

static bool uidMatch(const uint8_t* a, uint8_t aLen,
                     const uint8_t* b, uint8_t bLen) {
    if (aLen != bLen || aLen == 0) return false;
    return memcmp(a, b, aLen) == 0;
}

NfcActions::Result NfcActions::process(NFC::TapType tap,
                                       const uint8_t* uid, uint8_t uidLen,
                                       const Settings& settings) {
    Result r;

    // Check for location tag match
    for (int i = 0; i < 8; i++) {
        const NfcTag& tag = settings.nfcTags[i];
        if (tag.uidLen > 0 && uidMatch(uid, uidLen, tag.uid, tag.uidLen)) {
            r.locationOverride = true;
            r.overrideZone = tag.zone;
            break;
        }
    }

    switch (tap) {
        case NFC::TAP_SINGLE:
            r.targetScreen = SCREEN_GLANCE;
            break;
        case NFC::TAP_DOUBLE:
        case NFC::TAP_TRIPLE:
            r.targetScreen = SCREEN_REVIEW;
            break;
        case NFC::TAP_LONG_HOLD:
            r.targetScreen = SCREEN_SETTINGS;
            break;
        default:
            r.targetScreen = SCREEN_HOME;
            break;
    }

    return r;
}
