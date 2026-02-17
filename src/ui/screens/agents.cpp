#include "agents.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Agents screen -- Individual agent drill-down
// ==========================================================================

void Screens::agents(TFT_eSprite& fb, const CosmaniaStatus& status,
                     int selectedIndex) {
    fb.fillSprite(Theme::BG);

    if (!status.connected || status.agentCount == 0) {
        Theme::drawHeader(fb, "AGENT DETAIL");
        Theme::drawCenteredGLCD(fb, 100, "NO DATA", Theme::FG_MUTED);
        fb.setTextColor(Theme::FG_MUTED);
        fb.setTextDatum(BC_DATUM);
        fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
        fb.setTextDatum(TL_DATUM);
        return;
    }

    int idx = selectedIndex % status.agentCount;
    const AgentInfo& a = status.agents[idx];

    // Header with agent name
    char title[24];
    snprintf(title, sizeof(title), "%.15s", a.name);
    // Uppercase the title
    for (int i = 0; title[i]; i++) {
        if (title[i] >= 'a' && title[i] <= 'z') title[i] -= 32;
    }
    Theme::drawHeader(fb, title);

    int y = 32;
    int step = 18;

    fb.setTextFont(1);
    fb.setTextSize(1);

    auto row = [&](const char* label, const char* value,
                   uint16_t color = Theme::FG) {
        fb.setTextColor(Theme::FG_MUTED);
        fb.setCursor(8, y);
        fb.print(label);
        fb.setTextColor(color);
        fb.setTextDatum(TR_DATUM);
        fb.drawString(value, DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);
        y += step;
    };

    // Status
    row("STATUS", a.overdue ? "OVERDUE" : (a.todayRuns > 0 ? "OK" : "IDLE"),
        a.overdue ? Theme::RED : Theme::GREEN);

    // Runs today
    char runsStr[8];
    snprintf(runsStr, sizeof(runsStr), "%d", a.todayRuns);
    row("RUNS TODAY", runsStr);

    // Cost
    char costStr[12];
    snprintf(costStr, sizeof(costStr), "$%.4f", a.todayCostUsd);
    row("COST", costStr, a.overBudget ? Theme::RED : Theme::FG);

    // Last run
    if (a.minutesSince >= 0) {
        char lastStr[16];
        if (a.minutesSince < 60)
            snprintf(lastStr, sizeof(lastStr), "%dm AGO", a.minutesSince);
        else
            snprintf(lastStr, sizeof(lastStr), "%dh AGO", a.minutesSince / 60);
        row("LAST RUN", lastStr);
    } else {
        row("LAST RUN", "NEVER", Theme::FG_MUTED);
    }

    row("OVERDUE", a.overdue ? "YES" : "NO",
        a.overdue ? Theme::RED : Theme::FG);
    row("OVER BUDGET", a.overBudget ? "YES" : "NO",
        a.overBudget ? Theme::RED : Theme::FG);

    // Navigation hint
    Theme::drawRule(fb, DISPLAY_H - 24, Theme::BORDER);

    char navStr[32];
    snprintf(navStr, sizeof(navStr), "< %d/%d >  OK=BACK",
             idx + 1, status.agentCount);
    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString(navStr, DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
