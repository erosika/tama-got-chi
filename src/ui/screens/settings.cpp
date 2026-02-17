#include "settings.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Settings screen -- > cursor, value display, sharp layout
// ==========================================================================

void Screens::settings(TFT_eSprite& fb, int selectedIndex, const Settings& s) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "SETTINGS");

    int baseY = 36;
    int step  = 22;

    // Labels and values
    struct Row {
        const char* label;
        const char* value;
    };

    const char* brStr[] = { "LOW", "MID", "HIGH" };

    char saveStr[8];
    snprintf(saveStr, sizeof(saveStr), "%ds", s.autoSaveMs / 1000);

    Row rows[] = {
        { "DISPLAY",    brStr[s.tftBrightness % 3] },
        { "LED",        brStr[s.ledBrightness % 3] },
        { "SOUND",      s.soundEnabled ? "ON" : "OFF" },
        { "NEOPIXELS",  s.neoPixelsEnabled ? "ON" : "OFF" },
        { "AUTO SLEEP", s.autoSleep ? "ON" : "OFF" },
        { "AUTO SAVE",  saveStr },
        { "RESET PET",  "" },
        { "RESET ALL",  "" },
        { "BACK",       "" },
    };
    int rowCount = sizeof(rows) / sizeof(rows[0]);

    for (int i = 0; i < rowCount; i++) {
        int y = baseY + i * step;

        Theme::drawMenuItem(fb, y, rows[i].label, i == selectedIndex);

        // Value on the right side
        if (rows[i].value[0] != '\0') {
            fb.setTextFont(1);
            fb.setTextColor(Theme::ACCENT);
            fb.setTextDatum(TR_DATUM);
            fb.drawString(rows[i].value, DISPLAY_W - 12, y);
            fb.setTextDatum(TL_DATUM);
        }
    }
}
