#include "hatch.h"
#include "../theme.h"
#include "config.h"
#include "../../sprites/sprites.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Hatch screen -- Egg idle -> triggered -> hatch sequence
// ==========================================================================

static const uint16_t* EGG_IDLE[4] = {
    egg_hatch_11, egg_hatch_21, egg_hatch_31, egg_hatch_41
};
static const uint16_t* EGG_HATCH[5] = {
    egg_hatch_1, egg_hatch_2, egg_hatch_3, egg_hatch_4, egg_hatch_5
};

static int s_idleFrame  = 0;
static int s_hatchFrame = 0;
static unsigned long s_lastTime = 0;
static bool s_sequenceStarted = false;

static constexpr int EGG_X = 62;   // centered
static constexpr int EGG_Y = 50;

bool Screens::hatch(TFT_eSprite& fb, TFT_eSprite& petSpr,
                    bool hatched, bool triggered) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "HATCHING");

    unsigned long now = millis();

    // Already hatched -- shouldn't be here, signal transition
    if (hatched) return true;

    // Idle egg (waiting for OK press)
    if (!triggered) {
        if (now - s_lastTime >= EGG_IDLE_DELAY) {
            s_lastTime = now;
            s_idleFrame = (s_idleFrame + 1) % 4;
        }

        petSpr.pushImage(0, 0, PET_SPRITE_W, PET_SPRITE_H,
                        EGG_IDLE[s_idleFrame]);
        petSpr.pushToSprite(&fb, EGG_X, EGG_Y, TFT_WHITE);

        Theme::drawCenteredGLCD(fb, 200, "> PRESS OK TO HATCH", Theme::FG_MUTED);
        return false;
    }

    // Hatch sequence
    if (!s_sequenceStarted) {
        s_sequenceStarted = true;
        s_hatchFrame = 0;
        s_lastTime = now;
    }

    if (now - s_lastTime >= HATCH_DELAY) {
        s_lastTime = now;
        if (s_hatchFrame < 4) {
            s_hatchFrame++;
        } else {
            // Animation complete
            s_sequenceStarted = false;
            s_hatchFrame = 0;
            s_idleFrame  = 0;
            return true;    // Signal: hatch complete
        }
    }

    petSpr.pushImage(0, 0, PET_SPRITE_W, PET_SPRITE_H,
                    EGG_HATCH[s_hatchFrame]);
    petSpr.pushToSprite(&fb, EGG_X, EGG_Y, TFT_WHITE);

    return false;
}
