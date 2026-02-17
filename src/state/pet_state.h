#pragma once
#include "types.h"

// ==========================================================================
// Pet State -- Stat decay, WiFi feeding, autonomous decisions
// ==========================================================================

namespace PetLogic {

struct Context {
    PetState* pet;
    WifiStats* wifi;
    CosmaniaStatus* cosmania;
    Settings* settings;
    Activity* activity;
    RestPhase* restPhase;
    int* restFrameIndex;
    unsigned long* restDurationMs;
    bool* restStatsApplied;
    const RadioEnvironment* radio;
};

void init(Context& ctx);
void tick(Context& ctx);       // Call at LOGIC_TICK_MS intervals

// WiFi feeding results
void resolveHunt(Context& ctx);
void resolveDiscover(Context& ctx);

// Reset
void resetPet(Context& ctx, bool fullReset);

}  // namespace PetLogic
