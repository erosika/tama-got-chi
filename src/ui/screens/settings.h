#pragma once
#include "types.h"

class TFT_eSprite;

namespace Screens {
void settings(TFT_eSprite& fb, int selectedIndex, const Settings& s);
}
