#include "display.h"
#include "config.h"

// ==========================================================================
// Display HAL -- TFT_eSPI init, sprite creation, backlight PWM
// ==========================================================================

static TFT_eSPI    s_tft;
static TFT_eSprite s_fb(&s_tft);
static TFT_eSprite s_pet(&s_tft);
static TFT_eSprite s_effect(&s_tft);

void Display::init() {
    s_tft.init();
    s_tft.setRotation(0);
    s_tft.setSwapBytes(true);

    s_fb.setColorDepth(16);
    s_fb.createSprite(DISPLAY_W, DISPLAY_H);
    s_fb.setSwapBytes(true);

    s_pet.setColorDepth(16);
    s_pet.createSprite(PET_SPRITE_W, PET_SPRITE_H);

    s_effect.setColorDepth(16);
    s_effect.createSprite(EFFECT_SPRITE_W, EFFECT_SPRITE_H);

    // Backlight PWM
    ledcSetup(TFT_BL_PWM_CH, 12000, 8);
    ledcAttachPin(PIN_TFT_BL, TFT_BL_PWM_CH);
    setBrightness(1);   // default mid
}

void Display::push() {
    s_fb.pushSprite(0, 0);
}

void Display::setBrightness(uint8_t level) {
    uint8_t val = (level == 0) ? 60 :
                  (level == 1) ? 150 : 255;
    ledcWrite(TFT_BL_PWM_CH, val);
}

TFT_eSPI&    Display::tft()          { return s_tft; }
TFT_eSprite& Display::fb()           { return s_fb; }
TFT_eSprite& Display::petSprite()    { return s_pet; }
TFT_eSprite& Display::effectSprite() { return s_effect; }
