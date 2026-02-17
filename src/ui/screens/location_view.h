#pragma once
#include "types.h"

class TFT_eSprite;

namespace Screens {
void locationView(TFT_eSprite& fb, LocationZone zone,
                  bool gpsFix, int satellites);
}
