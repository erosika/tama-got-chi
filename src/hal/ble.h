#pragma once
#include "config.h"
#include "types.h"

// ==========================================================================
// BLE -- Passive BLE scanning for device discovery + threat detection
// Compiles to no-op when FEATURE_SOVEREIGNTY == 0
// ==========================================================================

namespace BLE {

void init();
void tick();

int deviceCount();
int scannerCount();               // devices actively scanning
const BleDevice* devices();       // ring buffer, up to MAX_BLE_DEVICES
const BleDevice* device(int idx);

void clearDevices();

}  // namespace BLE
