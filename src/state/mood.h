#pragma once
#include "types.h"

// ==========================================================================
// MoodLogic -- Determines pet mood from Cosmania status + local state
// ==========================================================================

namespace MoodLogic {
void update(PetState& pet, const WifiStats& wifi, const CosmaniaStatus& cosmania,
            const RadioEnvironment& radio, unsigned long lastScanTime);
}
