#include "sysinfo.h"
#include "../theme.h"
#include "config.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

// ==========================================================================
// System Info screen -- heap, uptime, firmware version
// ==========================================================================

void Screens::sysinfo(TFT_eSprite& fb) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "SYSTEM");

    int y = 32;
    int step = 18;

    fb.setTextFont(1);
    fb.setTextSize(1);

    auto row = [&](const char* label, const char* value) {
        fb.setTextColor(Theme::FG_MUTED);
        fb.setCursor(8, y);
        fb.print(label);

        fb.setTextColor(Theme::FG);
        fb.setTextDatum(TR_DATUM);
        fb.drawString(value, DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);

        y += step;
    };

    row("FIRMWARE", "0.1");
    row("MCU", "ESP32-S3");

    char heapStr[16];
    snprintf(heapStr, sizeof(heapStr), "%lu KB", ESP.getFreeHeap() / 1024);
    row("HEAP FREE", heapStr);

    unsigned long s = millis() / 1000;
    unsigned long m = s / 60;
    unsigned long h = m / 60;
    s %= 60; m %= 60;
    char uptimeStr[16];
    snprintf(uptimeStr, sizeof(uptimeStr), "%02lu:%02lu:%02lu", h, m, s);
    row("UPTIME", uptimeStr);

    // Divider
    Theme::drawRule(fb, y + 4, Theme::BORDER);
    y += 14;

    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
