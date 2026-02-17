#include "theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Theme -- Drawing primitives for eri's dark-first aesthetic
// ==========================================================================

void Theme::drawRule(TFT_eSprite& fb, int y, uint16_t color) {
    fb.drawFastHLine(0, y, DISPLAY_W, color);
}

void Theme::drawHeader(TFT_eSprite& fb, const char* title) {
    fb.fillRect(0, 0, DISPLAY_W, 20, BG_ELEVATED);
    drawRule(fb, 20, ACCENT);

    fb.setTextFont(1);          // GLCD 8px monospace
    fb.setTextSize(1);
    fb.setTextColor(FG_BRIGHT);
    fb.setTextDatum(ML_DATUM);  // middle-left
    fb.drawString(title, 8, 10);
    fb.setTextDatum(TL_DATUM);  // reset
}

void Theme::drawBar(TFT_eSprite& fb, int x, int y, int w, int h,
                    int value, uint16_t color) {
    fb.drawRect(x, y, w, h, BORDER);
    int fill = ((w - 2) * value) / 100;
    if (fill > 0) {
        fb.fillRect(x + 1, y + 1, fill, h - 2, color);
    }
}

void Theme::drawCenteredGLCD(TFT_eSprite& fb, int y, const char* text,
                             uint16_t color) {
    fb.setTextFont(1);
    fb.setTextSize(1);
    fb.setTextColor(color);
    fb.setTextDatum(TC_DATUM);  // top-center
    fb.drawString(text, DISPLAY_W / 2, y);
    fb.setTextDatum(TL_DATUM);
}

void Theme::drawCenteredFont2(TFT_eSprite& fb, int y, const char* text,
                              uint16_t color) {
    fb.setTextFont(2);
    fb.setTextSize(1);
    fb.setTextColor(color);
    fb.setTextDatum(TC_DATUM);
    fb.drawString(text, DISPLAY_W / 2, y);
    fb.setTextDatum(TL_DATUM);
}

void Theme::drawMenuItem(TFT_eSprite& fb, int y, const char* label,
                         bool selected, uint16_t color) {
    fb.setTextFont(1);
    fb.setTextSize(1);

    if (selected) {
        fb.setTextColor(ACCENT);
        fb.setCursor(8, y);
        fb.print("> ");
        fb.print(label);
    } else {
        fb.setTextColor(color);
        fb.setCursor(20, y);
        fb.print(label);
    }
}
