#pragma once
#include "types.h"

class TFT_eSprite;

namespace Screens {
void glance(TFT_eSprite& fb, const PetState& pet,
            const CosmaniaStatus& cosmania);
}
