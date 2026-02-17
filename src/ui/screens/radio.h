#pragma once
#include "types.h"

class TFT_eSprite;

namespace Screens {
void radio(TFT_eSprite& fb, const RadioEnvironment& env,
           const ThreatEntry* threats, int threatCount);
}
