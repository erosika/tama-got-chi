#include "menu.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Menu screen -- > cursor, monospace, no highlight bars
// ==========================================================================

static const char* MENU_ITEMS[] = {
    "STATUS",
    "AGENTS",
    "RADIO",
    "WIFI SCAN",
    "SETTINGS",
    "SYSTEM INFO",
    "BACK",
};
static constexpr int MENU_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

void Screens::menu(TFT_eSprite& fb, int selectedIndex) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "MENU");

    int baseY = 40;
    int step  = 20;

    for (int i = 0; i < MENU_COUNT; i++) {
        Theme::drawMenuItem(fb, baseY + i * step, MENU_ITEMS[i],
                           i == selectedIndex);
    }

    // Footer hint
    fb.setTextFont(1);
    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString("UP/DOWN NAVIGATE  OK SELECT", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
