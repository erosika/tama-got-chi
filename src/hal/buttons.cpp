#include "buttons.h"
#include "config.h"
#include <Arduino.h>

// ==========================================================================
// Buttons HAL -- Edge detection (HIGH->LOW = press on active-low buttons)
// ==========================================================================

struct ButtonState {
    int pin;
    bool last;      // previous read (HIGH = released)
    bool pressed;   // true for one tick on falling edge
};

static ButtonState s_buttons[] = {
    { PIN_BTN_UP,   HIGH, false },
    { PIN_BTN_OK,   HIGH, false },
    { PIN_BTN_DOWN, HIGH, false },
    { PIN_BTN_R1,   HIGH, false },
    { PIN_BTN_R2,   HIGH, false },
    { PIN_BTN_R3,   HIGH, false },
};

static constexpr int BTN_COUNT = sizeof(s_buttons) / sizeof(s_buttons[0]);

void Buttons::init() {
    for (int i = 0; i < BTN_COUNT; i++) {
        pinMode(s_buttons[i].pin, INPUT_PULLUP);
        s_buttons[i].last    = HIGH;
        s_buttons[i].pressed = false;
    }
}

void Buttons::tick() {
    for (int i = 0; i < BTN_COUNT; i++) {
        bool current = digitalRead(s_buttons[i].pin);
        s_buttons[i].pressed = (s_buttons[i].last == HIGH && current == LOW);
        s_buttons[i].last = current;
    }
}

bool Buttons::upPressed()   { return s_buttons[0].pressed; }
bool Buttons::okPressed()   { return s_buttons[1].pressed; }
bool Buttons::downPressed() { return s_buttons[2].pressed; }
bool Buttons::r1Pressed()   { return s_buttons[3].pressed; }
bool Buttons::r2Pressed()   { return s_buttons[4].pressed; }
bool Buttons::r3Pressed()   { return s_buttons[5].pressed; }
