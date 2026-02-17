#pragma once
#include "types.h"

// ==========================================================================
// Threat Detection -- Local pattern matching for radio anomalies
// No network required. Runs entirely on ESP32.
// ==========================================================================

namespace ThreatDetect {

void init();
void tick();   // Evaluates BLE + WiFi data against threat patterns

const RadioEnvironment& environment();
const ThreatEntry* threats();        // up to MAX_THREATS
int threatCount();

// Force re-evaluation (e.g., after location change)
void evaluate();

}  // namespace ThreatDetect
