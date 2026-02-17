#include "wifi_scan.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// WiFi Scan screen -- Environment stats
// ==========================================================================

void Screens::wifiScan(TFT_eSprite& fb, const WifiStats& wifi) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "ENVIRONMENT");

    int y = 32;
    int step = 18;

    fb.setTextFont(1);
    fb.setTextSize(1);

    auto row = [&](const char* label, int value, uint16_t color = Theme::FG) {
        fb.setTextColor(Theme::FG_MUTED);
        fb.setCursor(8, y);
        fb.print(label);

        char str[16];
        snprintf(str, sizeof(str), "%d", value);
        fb.setTextColor(color);
        fb.setTextDatum(TR_DATUM);
        fb.drawString(str, DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);
        y += step;
    };

    row("NETWORKS", wifi.netCount, Theme::ACCENT);
    row("STRONG",   wifi.strongCount, Theme::GREEN);
    row("HIDDEN",   wifi.hiddenCount, Theme::PURPLE);
    row("OPEN",     wifi.openCount, Theme::ORANGE);
    row("WPA",      wifi.wpaCount);
    row("AVG RSSI", wifi.avgRSSI,
        wifi.avgRSSI > -60 ? Theme::GREEN :
        wifi.avgRSSI > -80 ? Theme::ORANGE : Theme::RED);

    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
