#include "review.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Review screen -- Overdue + over-budget agents (NFC triple tap)
// Shows all agents that need attention.
// ==========================================================================

void Screens::review(TFT_eSprite& fb, const CosmaniaStatus& cosmania) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "REVIEW");

    if (!cosmania.connected) {
        Theme::drawCenteredGLCD(fb, 100, "NO CONNECTION", Theme::FG_MUTED);
        fb.setTextColor(Theme::FG_MUTED);
        fb.setTextDatum(BC_DATUM);
        fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
        fb.setTextDatum(TL_DATUM);
        return;
    }

    int y = 30;
    int items = 0;

    fb.setTextFont(1);
    fb.setTextSize(1);

    // List overdue agents
    for (int i = 0; i < cosmania.agentCount && y < 190; i++) {
        const AgentInfo& a = cosmania.agents[i];
        if (!a.overdue && !a.overBudget) continue;

        items++;

        // Status indicator
        uint16_t color = a.overdue ? Theme::RED : Theme::ORANGE;
        fb.fillCircle(12, y + 4, 3, color);

        // Agent name
        fb.setTextColor(Theme::FG);
        fb.setCursor(22, y);
        fb.print(a.name);

        // Issue label
        fb.setTextColor(color);
        fb.setTextDatum(TR_DATUM);
        if (a.overdue && a.overBudget)
            fb.drawString("OVERDUE+$", DISPLAY_W - 8, y);
        else if (a.overdue)
            fb.drawString("OVERDUE", DISPLAY_W - 8, y);
        else
            fb.drawString("OVER BUDGET", DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);

        y += 16;
    }

    // List recent errors if space remains
    if (cosmania.errorCount > 0 && y < 180) {
        y += 4;
        Theme::drawRule(fb, y, Theme::BORDER);
        y += 8;

        char errStr[24];
        snprintf(errStr, sizeof(errStr), "%d ERROR%s",
                 cosmania.errorCount, cosmania.errorCount > 1 ? "S" : "");
        fb.setTextColor(Theme::RED);
        fb.setCursor(8, y);
        fb.print(errStr);
        y += 16;
    }

    if (items == 0 && cosmania.errorCount == 0) {
        Theme::drawCenteredGLCD(fb, 100, "ALL CLEAR", Theme::GREEN);
    }

    // Navigation
    Theme::drawRule(fb, DISPLAY_H - 24, Theme::BORDER);
    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
