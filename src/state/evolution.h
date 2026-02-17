#pragma once
#include "types.h"

// ==========================================================================
// Evolution -- Stage engine (milestone-based, never regresses)
// ==========================================================================

namespace Evolution {
void update(PetState& pet, const CosmaniaStatus& cosmania);

const char* stageName(EvolutionStage stage);
}
