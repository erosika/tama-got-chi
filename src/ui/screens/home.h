#pragma once
#include "types.h"

class TFT_eSprite;

namespace Screens {
struct HomeState {
    const PetState* pet;
    Activity activity;
    RestPhase restPhase;
    int restFrameIndex;
    bool hungerEffectActive;
    int hungerEffectFrame;
};

void home(TFT_eSprite& fb, TFT_eSprite& petSpr, TFT_eSprite& effectSpr,
          const HomeState& state);
}
