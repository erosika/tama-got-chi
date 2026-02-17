#pragma once
#include "types.h"

// ==========================================================================
// Renderer -- Screen dispatch + framebuffer management
// ==========================================================================

namespace Renderer {

void init();

struct DrawContext {
    Screen screen;
    int menuIndex;
    int settingsIndex;
    const PetState* pet;
    const Settings* settings;
    const WifiStats* wifi;
    const CosmaniaStatus* cosmania;
    Activity activity;
    RestPhase restPhase;
    int restFrameIndex;
    bool hungerEffectActive;
    int hungerEffectFrame;
    bool hatchTriggered;
    int agentIndex;
    LocationZone location;
    const RadioEnvironment* radio;
    const ThreatEntry* threats;
    int threatCount;
};

void draw(const DrawContext& ctx);

// Signals from last draw() call
bool wasHatchComplete();

}  // namespace Renderer
