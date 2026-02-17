#pragma once
#include <cstdint>

class TFT_eSprite;

namespace Screens {

// Returns true when hatch animation completes (transition to home)
bool hatch(TFT_eSprite& fb, TFT_eSprite& petSpr,
           bool hatched, bool triggered);
}
