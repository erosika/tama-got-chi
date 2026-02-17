#pragma once
#include "types.h"

// ==========================================================================
// Location -- Zone state machine (HOME/WORK/TRAVEL/UNKNOWN)
// GPS geofences + NFC tag overrides. GPS data never leaves device.
// ==========================================================================

namespace Location {

void init(const Settings& settings);
void tick(const Settings& settings);

LocationZone current();

// NFC tag override (lasts 4 hours)
void setOverride(LocationZone zone);
void clearOverride();

// GPS geofence match
LocationZone matchGeofence(float lat, float lng, const Settings& settings);

const char* zoneName(LocationZone zone);

}  // namespace Location
