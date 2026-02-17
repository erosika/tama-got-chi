#pragma once
#include "config.h"
#include "types.h"

// ==========================================================================
// WiFi Promiscuous -- Raw 802.11 frame capture for probe request
// monitoring and deauthentication detection.
// Compiles to no-op when FEATURE_SOVEREIGNTY == 0
// ==========================================================================

namespace WifiPromisc {

void init();
void tick();

void enable();
void disable();
bool isEnabled();

// Probe request data
int probeCount();
int uniqueProbers();
const ProbeRequest* probes();     // ring buffer, up to MAX_PROBE_REQUESTS

// Deauth detection
int deauthCount();
void resetDeauthCount();

// Channel hopping (only when not connected to AP)
void setChannelHopping(bool enabled);

}  // namespace WifiPromisc
