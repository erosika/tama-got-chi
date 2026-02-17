#include "radio.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Radio screen -- Radio environment + threat dashboard
// ==========================================================================

static uint16_t safetyColor(uint8_t score) {
    if (score >= 80) return Theme::GREEN;
    if (score >= 40) return Theme::ORANGE;
    return Theme::RED;
}

static const char* threatTypeLabel(ThreatType t) {
    switch (t) {
        case THREAT_ROGUE_SCANNER: return "ROGUE SCANNER";
        case THREAT_EVIL_TWIN:     return "EVIL TWIN AP";
        case THREAT_DEAUTH:        return "DEAUTH FLOOD";
        case THREAT_MASS_ENUM:     return "MASS ENUM";
        case THREAT_NFC_SKIMMER:   return "NFC SKIMMER";
        case THREAT_IMSI_PATTERN:  return "IMSI PATTERN";
        default:                   return "UNKNOWN";
    }
}

static uint16_t severityColor(ThreatSeverity s) {
    switch (s) {
        case THREAT_CRITICAL: return Theme::RED;
        case THREAT_WARNING:  return Theme::ORANGE;
        case THREAT_INFO:     return Theme::FG_MUTED;
    }
    return Theme::FG;
}

void Screens::radio(TFT_eSprite& fb, const RadioEnvironment& env,
                    const ThreatEntry* threats, int threatCount) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "RADIO");

    int y = 28;

    // Safety score -- large, centered
    fb.setTextFont(2);
    fb.setTextColor(safetyColor(env.safetyScore));
    fb.setTextDatum(TC_DATUM);

    char scoreStr[8];
    snprintf(scoreStr, sizeof(scoreStr), "%d", env.safetyScore);
    fb.drawString(scoreStr, DISPLAY_W / 2 - 20, y);

    fb.setTextFont(1);
    fb.setTextSize(1);
    fb.setTextColor(Theme::FG_MUTED);
    fb.drawString("/ 100", DISPLAY_W / 2 + 20, y + 6);
    fb.setTextDatum(TL_DATUM);

    y += 28;
    Theme::drawRule(fb, y, Theme::BORDER);
    y += 8;

    // Stats rows
    auto row = [&](const char* label, int value, uint16_t color = Theme::FG) {
        fb.setTextColor(Theme::FG_MUTED);
        fb.setCursor(8, y);
        fb.print(label);
        char val[8];
        snprintf(val, sizeof(val), "%d", value);
        fb.setTextColor(color);
        fb.setTextDatum(TR_DATUM);
        fb.drawString(val, DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);
        y += 14;
    };

    row("BLE DEVICES", env.bleDeviceCount);
    row("BLE SCANNERS", env.bleScannerCount,
        env.bleScannerCount > 0 ? Theme::ORANGE : Theme::FG);
    row("PROBES", env.probeCount);
    row("PROBERS", env.uniqueProbers);
    row("DEAUTHS", env.deauthCount,
        env.deauthCount > 0 ? Theme::RED : Theme::FG);
    row("THREATS", env.threatCount,
        env.threatCount > 0 ? Theme::RED : Theme::GREEN);

    // Show most recent threat if any
    if (threatCount > 0) {
        y += 4;
        Theme::drawRule(fb, y, Theme::BORDER);
        y += 6;

        const ThreatEntry& t = threats[threatCount - 1];
        fb.setTextColor(severityColor(t.severity));
        fb.setCursor(8, y);
        fb.print(threatTypeLabel(t.type));
        y += 12;

        if (t.detail[0]) {
            fb.setTextColor(Theme::FG_MUTED);
            fb.setCursor(8, y);
            // Truncate detail to fit screen
            char truncated[30];
            strncpy(truncated, t.detail, 29);
            truncated[29] = '\0';
            fb.print(truncated);
        }
    }

    // Navigation
    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
