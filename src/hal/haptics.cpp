#include "haptics.h"
#include <Arduino.h>

// ==========================================================================
// Haptics -- Non-blocking vibration motor patterns
// ==========================================================================

#if FEATURE_HAPTICS

struct HapticPattern {
    const uint16_t* durations;  // on/off/on/off... (ms)
    uint8_t length;
};

// Pattern definitions (on/off pairs in ms)
static const uint16_t PAT_PULSE[]       = {100};
static const uint16_t PAT_DOUBLE[]      = {80, 80, 80};
static const uint16_t PAT_ALERT[]       = {300};
static const uint16_t PAT_HEARTBEAT[]   = {60, 100, 80, 400};

static const HapticPattern PATTERNS[] = {
    {nullptr, 0},                   // NONE
    {PAT_PULSE, 1},                 // PULSE
    {PAT_DOUBLE, 3},                // DOUBLE
    {PAT_ALERT, 1},                 // ALERT
    {PAT_HEARTBEAT, 4},             // HEARTBEAT
};

static uint8_t s_currentPattern = 0;
static uint8_t s_step = 0;
static unsigned long s_stepStart = 0;
static bool s_motorOn = false;

static void startPattern(uint8_t idx) {
    s_currentPattern = idx;
    s_step = 0;
    s_stepStart = millis();
    s_motorOn = true;
    digitalWrite(PIN_HAPTIC, HIGH);
}

void Haptics::init() {
    pinMode(PIN_HAPTIC, OUTPUT);
    digitalWrite(PIN_HAPTIC, LOW);
}

void Haptics::tick() {
    if (s_currentPattern == 0) return;

    const HapticPattern& pat = PATTERNS[s_currentPattern];
    unsigned long now = millis();

    if (now - s_stepStart >= pat.durations[s_step]) {
        s_step++;
        s_stepStart = now;

        if (s_step >= pat.length) {
            // Pattern complete
            digitalWrite(PIN_HAPTIC, LOW);
            s_currentPattern = 0;
            s_motorOn = false;
            return;
        }

        s_motorOn = !s_motorOn;
        digitalWrite(PIN_HAPTIC, s_motorOn ? HIGH : LOW);
    }
}

void Haptics::pulse()       { startPattern(1); }
void Haptics::doublePulse() { startPattern(2); }
void Haptics::alert()       { startPattern(3); }
void Haptics::heartbeat()   { startPattern(4); }

#else

// -- Stubs when haptics disabled -------------------------------------------
void Haptics::init() {}
void Haptics::tick() {}
void Haptics::pulse() {}
void Haptics::doublePulse() {}
void Haptics::alert() {}
void Haptics::heartbeat() {}

#endif
