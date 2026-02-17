#include "boot.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Boot screen -- dark, stark, eri's aesthetic
// ==========================================================================

void Screens::boot(TFT_eSprite& fb) {
    fb.fillSprite(Theme::BG);

    // Top accent rule
    Theme::drawRule(fb, 24, Theme::ACCENT);

    // Title block
    Theme::drawCenteredGLCD(fb, 50, "TAMAFI", Theme::FG_BRIGHT);
    Theme::drawCenteredFont2(fb, 72, "COSMANIA COMPANION", Theme::ACCENT);
    Theme::drawCenteredGLCD(fb, 100, "V0.1", Theme::FG_MUTED);

    // Divider
    Theme::drawRule(fb, 180, Theme::BORDER);

    // Prompt
    Theme::drawCenteredGLCD(fb, 200, "> PRESS ANY BUTTON", Theme::FG);
}
