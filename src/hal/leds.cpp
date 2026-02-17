#include "leds.h"
#include "config.h"
#include <Adafruit_NeoPixel.h>

// ==========================================================================
// LEDs HAL -- NeoPixel patterns (ported from upstream TamaFi.ino)
// ==========================================================================

static Adafruit_NeoPixel s_strip(LED_COUNT, PIN_LED_DATA, NEO_GRB + NEO_KHZ800);
static bool s_enabled = true;

void LEDs::init() {
    pinMode(PIN_LED_DATA, OUTPUT);
    digitalWrite(PIN_LED_DATA, LOW);

    s_strip.begin();
    s_strip.setBrightness(40);
    s_strip.clear();
    s_strip.show();
}

void LEDs::tick() {
    // Periodic patterns handled by callers via explicit calls
}

void LEDs::off() {
    for (int i = 0; i < LED_COUNT; i++)
        s_strip.setPixelColor(i, 0);
    s_strip.show();
}

static void setAll(uint8_t r, uint8_t g, uint8_t b) {
    if (!s_enabled) return;
    uint32_t c = s_strip.Color(r, g, b);
    for (int i = 0; i < LED_COUNT; i++)
        s_strip.setPixelColor(i, c);
    s_strip.show();
}

void LEDs::happy()   { setAll(120, 40, 200); }
void LEDs::sad()     { setAll(200,  0,   0); }
void LEDs::wifi()    { setAll(  0, 90, 255); }
void LEDs::rest()    { setAll(  0, 25,  90); }

void LEDs::breathe(unsigned long phase) {
    if (!s_enabled) return;
    float t = phase / (float)REST_BREATHE_MS;
    int val = (int)(sin(t) * 40.0f + 60.0f);
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    for (int i = 0; i < LED_COUNT; i++)
        s_strip.setPixelColor(i, s_strip.Color(0, 0, val));
    s_strip.show();
}

void LEDs::setBrightness(uint8_t level) {
    uint8_t bri = (level == 0) ? 20 :
                  (level == 1) ? 90 : 180;
    s_strip.setBrightness(bri);
    if (!s_enabled) {
        off();
        return;
    }
    s_strip.show();
}

void LEDs::setEnabled(bool on) {
    s_enabled = on;
    if (!on) off();
}

bool LEDs::isEnabled() { return s_enabled; }
