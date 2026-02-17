#include "mood.h"
#include <Arduino.h>

// ==========================================================================
// Mood -- Priority-based mood selection (Cosmania-driven)
// Priority: SICK > ANGRY > ANXIOUS > WORKING > HUNGRY > SLEEPY > HAPPY > CONTENT
// Falls back to WiFi/local state when Cosmania not connected.
// ==========================================================================

static bool anyAgentRecentlyActive(const CosmaniaStatus& c) {
    for (int i = 0; i < c.agentCount; i++) {
        if (c.agents[i].minutesSince >= 0 && c.agents[i].minutesSince < 5)
            return true;
    }
    return false;
}

void MoodLogic::update(PetState& pet, const WifiStats& wifi,
                       const CosmaniaStatus& cosmania,
                       const RadioEnvironment& radio,
                       unsigned long lastScanTime) {
    unsigned long now = millis();

    // -- Radio-driven moods (highest priority -- physical safety) ----------
    if (radio.safetyScore < 40) {
        // Critical radio environment overrides everything
        pet.mood = MOOD_SICK;
        return;
    }

    // -- Cosmania-driven moods (when connected) ----------------------------
    if (cosmania.connected) {
        // SICK: budgetTier RED or BLACK
        if (cosmania.budgetTier == TIER_RED || cosmania.budgetTier == TIER_BLACK) {
            pet.mood = MOOD_SICK;
            return;
        }

        // ANGRY: errorCount >= 3
        if (cosmania.errorCount >= 3) {
            pet.mood = MOOD_ANGRY;
            return;
        }

        // ANXIOUS: budgetTier YELLOW, overdueCount >= 2, or radio below 80
        if (cosmania.budgetTier == TIER_YELLOW || cosmania.overdueCount >= 2 ||
            radio.safetyScore < 80) {
            pet.mood = MOOD_ANXIOUS;
            return;
        }

        // WORKING: any agent ran within last 5 minutes
        if (anyAgentRecentlyActive(cosmania)) {
            pet.mood = MOOD_WORKING;
            return;
        }

        // HUNGRY: low hunger (WiFi mechanic still drives this)
        if (pet.hunger < 25) {
            pet.mood = MOOD_HUNGRY;
            return;
        }

        // SLEEPY: no agents active today
        if (cosmania.activeCount == 0) {
            pet.mood = MOOD_SLEEPY;
            return;
        }

        // HAPPY: GREEN tier, no overdue, good stats
        if (cosmania.budgetTier == TIER_GREEN &&
            cosmania.overdueCount == 0 && pet.happiness > 60) {
            pet.mood = MOOD_HAPPY;
            return;
        }

        // CONTENT: GREEN tier, no overdue
        if (cosmania.budgetTier == TIER_GREEN && cosmania.overdueCount == 0) {
            pet.mood = MOOD_CONTENT;
            return;
        }

        pet.mood = MOOD_CONTENT;
        return;
    }

    // -- Fallback: WiFi/local state only (no Cosmania connection) ----------

    // SICK: low health or no WiFi for extended time
    if (pet.health < 25 ||
        (wifi.netCount == 0 && lastScanTime > 0 &&
         (now - lastScanTime) > 60000)) {
        pet.mood = MOOD_SICK;
        return;
    }

    // HUNGRY
    if (pet.hunger < 25) {
        pet.mood = MOOD_HUNGRY;
        return;
    }

    // HAPPY: good stats + WiFi-rich environment
    if (pet.happiness > 80 && wifi.netCount > 8) {
        pet.mood = MOOD_HAPPY;
        return;
    }

    // SLEEPY: no WiFi for a while
    if (wifi.netCount == 0 && lastScanTime > 0 &&
        (now - lastScanTime) > 30000) {
        pet.mood = MOOD_SLEEPY;
        return;
    }

    pet.mood = MOOD_CONTENT;
}
