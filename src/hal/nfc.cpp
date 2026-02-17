#include "nfc.h"
#include <Arduino.h>

// ==========================================================================
// NFC -- PN532 I2C: multi-tap detection + long hold
// ==========================================================================

#if FEATURE_NFC

#include <Wire.h>
#include <Adafruit_PN532.h>

static Adafruit_PN532 s_nfc(PIN_NFC_SDA, PIN_NFC_SCL);
static bool s_initialized = false;

// Tap detection state
static NFC::TapType s_pendingTap   = NFC::TAP_NONE;
static uint8_t      s_tapCount     = 0;
static unsigned long s_firstTapMs  = 0;
static unsigned long s_holdStartMs = 0;
static bool          s_tagPresent  = false;
static bool          s_wasPresent  = false;

// Multi-tap window
static constexpr unsigned long MULTI_TAP_WINDOW_MS = 500;
// Long hold threshold
static constexpr unsigned long LONG_HOLD_MS = 3000;
// Debounce between reads
static constexpr unsigned long READ_INTERVAL_MS = 100;
static unsigned long s_lastReadMs = 0;

// Last tag UID
static uint8_t s_uid[7]  = {0};
static uint8_t s_uidLen  = 0;

void NFC::init() {
    s_nfc.begin();
    uint32_t fw = s_nfc.getFirmwareVersion();
    if (fw) {
        s_nfc.SAMConfig();
        s_initialized = true;
        Serial.println("[nfc] PN532 ready");
    } else {
        Serial.println("[nfc] PN532 not found");
    }
}

void NFC::tick() {
    if (!s_initialized) return;

    unsigned long now = millis();
    if (now - s_lastReadMs < READ_INTERVAL_MS) return;
    s_lastReadMs = now;

    // Try to read a tag (non-blocking: 50ms timeout)
    uint8_t uid[7];
    uint8_t uidLen = 0;
    bool found = s_nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 50);

    s_wasPresent = s_tagPresent;
    s_tagPresent = found;

    if (found) {
        memcpy(s_uid, uid, uidLen);
        s_uidLen = uidLen;

        if (!s_wasPresent) {
            // New tag arrival
            s_holdStartMs = now;
            s_tapCount++;
            if (s_tapCount == 1) s_firstTapMs = now;
        }

        // Check for long hold
        if ((now - s_holdStartMs) >= LONG_HOLD_MS && s_pendingTap != TAP_LONG_HOLD) {
            s_pendingTap = TAP_LONG_HOLD;
            s_tapCount = 0;
        }
    } else {
        // Tag removed -- if we had taps accumulating and window expired, resolve
        if (s_tapCount > 0 && (now - s_firstTapMs) > MULTI_TAP_WINDOW_MS) {
            if (s_pendingTap != TAP_LONG_HOLD) {
                switch (s_tapCount) {
                    case 1: s_pendingTap = TAP_SINGLE; break;
                    case 2: s_pendingTap = TAP_DOUBLE; break;
                    default: s_pendingTap = TAP_TRIPLE; break;
                }
            }
            s_tapCount = 0;
        }
    }
}

NFC::TapType NFC::consumeTap() {
    TapType t = s_pendingTap;
    s_pendingTap = TAP_NONE;
    return t;
}

bool NFC::isTagPresent() { return s_tagPresent; }
const uint8_t* NFC::lastUID() { return s_uid; }
uint8_t NFC::lastUIDLen() { return s_uidLen; }

#else

// -- Stubs when NFC disabled -----------------------------------------------
void NFC::init() {}
void NFC::tick() {}
NFC::TapType NFC::consumeTap() { return NFC::TAP_NONE; }
bool NFC::isTagPresent() { return false; }
const uint8_t* NFC::lastUID() { static uint8_t z[1] = {0}; return z; }
uint8_t NFC::lastUIDLen() { return 0; }

#endif
