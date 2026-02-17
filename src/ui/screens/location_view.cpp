#include "location_view.h"
#include "../theme.h"
#include "config.h"
#include "../../state/location.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Location screen -- Current zone, GPS status, override info
// ==========================================================================

static uint16_t zoneColor(LocationZone z) {
    switch (z) {
        case LOC_HOME:    return Theme::GREEN;
        case LOC_WORK:    return Theme::ACCENT;
        case LOC_TRAVEL:  return Theme::ORANGE;
        case LOC_UNKNOWN: return Theme::FG_MUTED;
    }
    return Theme::FG;
}

void Screens::locationView(TFT_eSprite& fb, LocationZone zone,
                           bool gpsFix, int satellites) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "LOCATION");

    int y = 40;

    // Large zone display
    fb.setTextFont(2);
    fb.setTextColor(zoneColor(zone));
    fb.setTextDatum(TC_DATUM);
    fb.drawString(Location::zoneName(zone), DISPLAY_W / 2, y);
    y += 32;

    Theme::drawRule(fb, y, Theme::BORDER);
    y += 16;

    fb.setTextFont(1);
    fb.setTextSize(1);

    // GPS status
    fb.setTextColor(Theme::FG_MUTED);
    fb.setCursor(8, y);
    fb.print("GPS FIX");
    fb.setTextColor(gpsFix ? Theme::GREEN : Theme::RED);
    fb.setTextDatum(TR_DATUM);
    fb.drawString(gpsFix ? "YES" : "NO", DISPLAY_W - 8, y);
    fb.setTextDatum(TL_DATUM);
    y += 18;

    // Satellites
    fb.setTextColor(Theme::FG_MUTED);
    fb.setCursor(8, y);
    fb.print("SATELLITES");
    char satStr[8];
    snprintf(satStr, sizeof(satStr), "%d", satellites);
    fb.setTextColor(satellites > 0 ? Theme::FG : Theme::FG_MUTED);
    fb.setTextDatum(TR_DATUM);
    fb.drawString(satStr, DISPLAY_W - 8, y);
    fb.setTextDatum(TL_DATUM);
    y += 18;

    // Zone behavior hint
    y += 8;
    Theme::drawRule(fb, y, Theme::BORDER);
    y += 12;

    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(TC_DATUM);
    switch (zone) {
        case LOC_HOME:
            fb.drawString("FULL DISPLAY  30s POLL", DISPLAY_W / 2, y);
            break;
        case LOC_WORK:
            fb.drawString("SIMPLE VIEW  60s POLL", DISPLAY_W / 2, y);
            break;
        case LOC_TRAVEL:
            fb.drawString("ALERTS ON  10s POLL", DISPLAY_W / 2, y);
            break;
        case LOC_UNKNOWN:
            fb.drawString("LOCKDOWN AVAILABLE", DISPLAY_W / 2, y);
            break;
    }

    // Navigation
    fb.setTextDatum(BC_DATUM);
    fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
