#pragma once
#include "types.h"

// ==========================================================================
// WiFi Radio HAL -- Scan abstraction (no STA connection)
// ==========================================================================

namespace WifiRadio {

void init();
void startScan();
bool isScanDone();          // True once results are ready
WifiStats getResults();     // Consumes scan results

}  // namespace WifiRadio
