#include "evolution.h"
#include "../hal/sound.h"
#include <cstring>

// ==========================================================================
// Evolution -- Cosmania milestone-based stage transitions (never regress)
//
//   EGG      -> LARVA     : sentinel todayRuns > 0
//   LARVA    -> NYMPH     : dreamer AND coder todayRuns > 0
//   NYMPH    -> JUVENILE  : activeAgentCount >= 7, ageDays >= 7
//   JUVENILE -> ADULT     : budgetTier GREEN for 7 consecutive days
//   ADULT    -> ELDER     : ageDays >= 180, all agents uptimePct > 80%
//
// Falls back to time-based when Cosmania not connected (pet survives
// on WiFi feeding but never evolves past LARVA without Cosmania).
// ==========================================================================

static bool agentHasRuns(const CosmaniaStatus& c, const char* name) {
    for (int i = 0; i < c.agentCount; i++) {
        if (strncmp(c.agents[i].name, name, 15) == 0 && c.agents[i].todayRuns > 0)
            return true;
    }
    return false;
}

void Evolution::update(PetState& pet, const CosmaniaStatus& cosmania) {
    uint32_t ageDays = pet.ageDays;

    // Never regress. Check from highest to lowest.
    if (cosmania.connected) {
        // ADULT -> ELDER: ageDays >= 180 (uptimePct check deferred to Phase 8)
        if (ageDays >= 180 && pet.stage < STAGE_ELDER) {
            pet.stage = STAGE_ELDER;
            Sound::discover();
            return;
        }

        // JUVENILE -> ADULT: budgetTier GREEN for 7 consecutive days
        if (cosmania.greenDaysStreak >= 7 && pet.stage < STAGE_ADULT) {
            pet.stage = STAGE_ADULT;
            Sound::discover();
            return;
        }

        // NYMPH -> JUVENILE: activeAgentCount >= 7, ageDays >= 7
        if (cosmania.activeCount >= 7 && ageDays >= 7 && pet.stage < STAGE_JUVENILE) {
            pet.stage = STAGE_JUVENILE;
            Sound::discover();
            return;
        }

        // LARVA -> NYMPH: dreamer AND coder have todayRuns > 0
        if (agentHasRuns(cosmania, "dreamer") &&
            agentHasRuns(cosmania, "coder") &&
            pet.stage < STAGE_NYMPH) {
            pet.stage = STAGE_NYMPH;
            Sound::discover();
            return;
        }

        // EGG -> LARVA: sentinel todayRuns > 0
        if (agentHasRuns(cosmania, "sentinel") && pet.stage < STAGE_LARVA) {
            pet.stage = STAGE_LARVA;
            Sound::discover();
            return;
        }
    } else {
        // Fallback: time-based for survival without Cosmania
        // Can only reach LARVA without Cosmania data
        int avg = (pet.hunger + pet.happiness + pet.health) / 3;
        if (avg > 40 && ageDays >= 1 && pet.stage < STAGE_LARVA) {
            pet.stage = STAGE_LARVA;
            Sound::discover();
        }
    }
}

const char* Evolution::stageName(EvolutionStage stage) {
    switch (stage) {
        case STAGE_EGG:      return "EGG";
        case STAGE_LARVA:    return "LARVA";
        case STAGE_NYMPH:    return "NYMPH";
        case STAGE_JUVENILE: return "JUVENILE";
        case STAGE_ADULT:    return "ADULT";
        case STAGE_ELDER:    return "ELDER";
    }
    return "?";
}
