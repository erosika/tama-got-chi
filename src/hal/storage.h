#pragma once
#include "types.h"

// ==========================================================================
// Storage HAL -- NVS persistence for pet state + settings
// ==========================================================================

namespace Storage {

void init();
void save(const PetState& pet, const Settings& settings);
void load(PetState& pet, Settings& settings);

}  // namespace Storage
