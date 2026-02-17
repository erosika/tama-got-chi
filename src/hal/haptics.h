#pragma once
#include "config.h"

// ==========================================================================
// Haptics -- Vibration motor patterns
// Compiles to no-op when FEATURE_HAPTICS == 0
// ==========================================================================

namespace Haptics {

void init();
void tick();

void pulse();       // Short 100ms buzz
void doublePulse(); // Two short buzzes
void alert();       // Long 300ms buzz
void heartbeat();   // Thump-thump pattern

}  // namespace Haptics
