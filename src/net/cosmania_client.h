#pragma once
#include "types.h"

// ==========================================================================
// Cosmania Client -- HTTP poll /status endpoint, parse JSON
// ==========================================================================

namespace CosmaniaClient {

void init(const char* baseUrl);
void tick();    // Non-blocking: polls at configured interval

bool isConnected();
const CosmaniaStatus& getStatus();

// Force immediate poll (e.g., after location change)
void pollNow();

}  // namespace CosmaniaClient
