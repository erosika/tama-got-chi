#include "glance.h"
#include "../theme.h"
#include "config.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Glance screen -- NFC wake: priority items + mood (2-second read)
// Shows the most important info at a glance: mood, tier, worst agent.
// ==========================================================================

static const char* moodLabel(Mood m) {
    switch (m) {
        case MOOD_SICK:    return "SICK";
        case MOOD_ANGRY:   return "ANGRY";
        case MOOD_ANXIOUS: return "ANXIOUS";
        case MOOD_WORKING: return "WORKING";
        case MOOD_HUNGRY:  return "HUNGRY";
        case MOOD_SLEEPY:  return "SLEEPY";
        case MOOD_HAPPY:   return "HAPPY";
        case MOOD_CONTENT: return "CONTENT";
    }
    return "?";
}

static uint16_t moodColor(Mood m) {
    switch (m) {
        case MOOD_SICK:    return Theme::RED;
        case MOOD_ANGRY:   return Theme::RED;
        case MOOD_ANXIOUS: return Theme::ORANGE;
        case MOOD_WORKING: return Theme::ACCENT;
        case MOOD_HUNGRY:  return Theme::ORANGE;
        case MOOD_SLEEPY:  return Theme::FG_MUTED;
        case MOOD_HAPPY:   return Theme::GREEN;
        case MOOD_CONTENT: return Theme::FG;
    }
    return Theme::FG;
}

static const char* tierLabel(BudgetTier t) {
    switch (t) {
        case TIER_GREEN:   return "GREEN";
        case TIER_YELLOW:  return "YELLOW";
        case TIER_RED:     return "RED";
        case TIER_BLACK:   return "BLACK";
        case TIER_UNKNOWN: return "---";
    }
    return "?";
}

static uint16_t tierColor(BudgetTier t) {
    switch (t) {
        case TIER_GREEN:  return Theme::GREEN;
        case TIER_YELLOW: return Theme::ORANGE;
        case TIER_RED:    return Theme::RED;
        case TIER_BLACK:  return Theme::RED;
        default:          return Theme::FG_MUTED;
    }
}

void Screens::glance(TFT_eSprite& fb, const PetState& pet,
                     const CosmaniaStatus& cosmania) {
    fb.fillSprite(Theme::BG);

    int y = 30;

    // Large mood display
    fb.setTextFont(1);
    fb.setTextSize(1);
    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(TC_DATUM);
    fb.drawString("MOOD", DISPLAY_W / 2, y);
    y += 16;

    fb.setTextFont(2);
    fb.setTextColor(moodColor(pet.mood));
    fb.drawString(moodLabel(pet.mood), DISPLAY_W / 2, y);
    y += 28;

    Theme::drawRule(fb, y, Theme::BORDER);
    y += 12;

    // Budget tier
    fb.setTextFont(1);
    fb.setTextSize(1);
    fb.setTextColor(Theme::FG_MUTED);
    fb.drawString("BUDGET", DISPLAY_W / 2, y);
    y += 16;

    fb.setTextFont(2);
    fb.setTextColor(tierColor(cosmania.budgetTier));
    fb.drawString(tierLabel(cosmania.budgetTier), DISPLAY_W / 2, y);
    y += 28;

    Theme::drawRule(fb, y, Theme::BORDER);
    y += 12;

    // Worst agent (first overdue, or highest cost)
    if (cosmania.connected && cosmania.agentCount > 0) {
        int worstIdx = -1;
        for (int i = 0; i < cosmania.agentCount; i++) {
            if (cosmania.agents[i].overdue) { worstIdx = i; break; }
        }
        if (worstIdx < 0) {
            // Find highest cost agent
            float maxCost = 0;
            for (int i = 0; i < cosmania.agentCount; i++) {
                if (cosmania.agents[i].todayCostUsd > maxCost) {
                    maxCost = cosmania.agents[i].todayCostUsd;
                    worstIdx = i;
                }
            }
        }

        if (worstIdx >= 0) {
            const AgentInfo& a = cosmania.agents[worstIdx];
            fb.setTextFont(1);
            fb.setTextColor(Theme::FG_MUTED);
            fb.drawString(a.overdue ? "OVERDUE" : "TOP COST", DISPLAY_W / 2, y);
            y += 16;

            char buf[24];
            snprintf(buf, sizeof(buf), "%.15s", a.name);
            fb.setTextFont(2);
            fb.setTextColor(a.overdue ? Theme::RED : Theme::FG);
            fb.drawString(buf, DISPLAY_W / 2, y);
        }
    } else {
        fb.setTextFont(1);
        fb.setTextColor(Theme::FG_MUTED);
        fb.drawString("NO CONNECTION", DISPLAY_W / 2, y);
    }

    fb.setTextDatum(TL_DATUM);
}
