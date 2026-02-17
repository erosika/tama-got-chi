#pragma once
#include "config.h"
#include <cstdint>

// ==========================================================================
// GPS -- NEO-6M UART: NMEA parse, geofence
// Compiles to no-op when FEATURE_GPS == 0
// ==========================================================================

namespace GPS {

void init();
void tick();

bool hasFix();
float lat();
float lng();
int satellites();

// Sleep/wake for power management
void sleep();
void wake();

}  // namespace GPS
