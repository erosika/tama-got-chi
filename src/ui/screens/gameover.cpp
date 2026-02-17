#include "gameover.h"
#include "../theme.h"
#include "config.h"
#include "../../sprites/sprites.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Game Over screen -- Dead pet animation
// ==========================================================================

static const uint16_t* DEAD_FRAMES[3] = { dead_1, dead_2, dead_3 };

static int s_frame = 0;
static unsigned long s_lastTime = 0;

static constexpr int PET_X = 62;
static constexpr int PET_Y = 50;

void Screens::gameover(TFT_eSprite& fb, TFT_eSprite& petSpr) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "SYSTEM DOWN");

    unsigned long now = millis();
    if (now - s_lastTime >= DEAD_DELAY) {
        s_lastTime = now;
        if (s_frame < 2) s_frame++;
    }

    petSpr.pushImage(0, 0, PET_SPRITE_W, PET_SPRITE_H,
                    DEAD_FRAMES[s_frame]);
    petSpr.pushToSprite(&fb, PET_X, PET_Y, TFT_WHITE);

    Theme::drawRule(fb, 180, Theme::RED);

    Theme::drawCenteredGLCD(fb, 195, "ALL AGENTS OFFLINE", Theme::RED);
    Theme::drawCenteredGLCD(fb, 215, "> OK TO RESTART", Theme::FG_MUTED);
}
