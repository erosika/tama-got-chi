#include "dashboard.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Dashboard screen -- All agents in compact rows
// ==========================================================================

static uint16_t tierColor(BudgetTier tier) {
    switch (tier) {
        case TIER_GREEN:  return Theme::GREEN;
        case TIER_YELLOW: return Theme::ORANGE;
        case TIER_RED:    return Theme::RED;
        case TIER_BLACK:  return Theme::RED;
        default:          return Theme::FG_MUTED;
    }
}

static const char* tierName(BudgetTier tier) {
    switch (tier) {
        case TIER_GREEN:  return "GREEN";
        case TIER_YELLOW: return "YELLOW";
        case TIER_RED:    return "RED";
        case TIER_BLACK:  return "BLACK";
        default:          return "---";
    }
}

void Screens::dashboard(TFT_eSprite& fb, const CosmaniaStatus& status) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "AGENTS");

    if (!status.connected) {
        Theme::drawCenteredGLCD(fb, 100, "NO CONNECTION", Theme::FG_MUTED);
        Theme::drawCenteredGLCD(fb, 120, "CONFIGURE WIFI + URL", Theme::FG_MUTED);
        fb.setTextColor(Theme::FG_MUTED);
        fb.setTextDatum(BC_DATUM);
        fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
        fb.setTextDatum(TL_DATUM);
        return;
    }

    // Budget tier badge
    fb.setTextFont(1);
    fb.setTextColor(tierColor(status.budgetTier));
    fb.setCursor(8, 26);
    fb.print("TIER ");
    fb.print(tierName(status.budgetTier));

    // Spend summary
    char spendStr[24];
    snprintf(spendStr, sizeof(spendStr), "$%.2f/$%.2f",
             status.totalDailySpend, status.totalDailyBudget);
    fb.setTextColor(Theme::FG);
    fb.setTextDatum(TR_DATUM);
    fb.drawString(spendStr, DISPLAY_W - 8, 26);
    fb.setTextDatum(TL_DATUM);

    Theme::drawRule(fb, 38, Theme::BORDER);

    // Agent rows
    int y = 44;
    int step = 22;

    for (int i = 0; i < status.agentCount && i < 7; i++) {
        const AgentInfo& a = status.agents[i];

        // Status dot
        uint16_t dotColor = Theme::FG_MUTED;
        if (a.todayRuns > 0 && !a.overdue) dotColor = Theme::GREEN;
        else if (a.overdue)                 dotColor = Theme::RED;
        else if (a.overBudget)              dotColor = Theme::ORANGE;
        fb.fillRect(8, y + 2, 4, 4, dotColor);

        // Name
        fb.setTextFont(1);
        fb.setTextColor(Theme::FG);
        fb.setCursor(18, y);
        fb.print(a.name);

        // Runs + cost on right
        char info[16];
        snprintf(info, sizeof(info), "%dx $%.3f", a.todayRuns, a.todayCostUsd);
        fb.setTextColor(Theme::FG_MUTED);
        fb.setTextDatum(TR_DATUM);
        fb.drawString(info, DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);

        y += step;
    }

    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
