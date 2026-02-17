#include "home.h"
#include "../theme.h"
#include "config.h"
#include "../../sprites/sprites.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Home screen -- Pet sprite + stats + mood + activity
// Dark background, no forest -- eri's aesthetic
// ==========================================================================

// -- Sprite frame tables ---------------------------------------------------
static const uint16_t* IDLE_FRAMES[4] = { idle_1, idle_2, idle_3, idle_4 };
static const uint16_t* EGG_FRAMES[5]  = {
    egg_hatch_1, egg_hatch_2, egg_hatch_3, egg_hatch_4, egg_hatch_5
};
static const uint16_t* ATTACK_FRAMES[3] = { attack_0, attack_1, attack_2 };
static const uint16_t* HUNGER_FRAMES[4] = { hunger1, hunger2, hunger3, hunger4 };

// -- Animation state -------------------------------------------------------
static int s_idleFrame = 0;
static unsigned long s_lastIdleTime = 0;

static int s_huntFrame = 0;
static unsigned long s_lastHuntTime = 0;

// -- Pet position ----------------------------------------------------------
static constexpr int PET_X = 62;   // centered: (240 - 115) / 2
static constexpr int PET_Y = 28;   // below header

// -- Mood text -------------------------------------------------------------
static const char* moodText(Mood m) {
    switch (m) {
        case MOOD_SICK:    return "SICK";
        case MOOD_ANGRY:   return "ANGRY";
        case MOOD_ANXIOUS: return "ANXIOUS";
        case MOOD_WORKING: return "WORKING";
        case MOOD_HUNGRY:  return "HUNGRY";
        case MOOD_SLEEPY:  return "SLEEPY";
        case MOOD_HAPPY:   return "HAPPY";
        case MOOD_CONTENT: return "CONTENT";
    }
    return "?";
}

// -- Activity text ---------------------------------------------------------
static const char* activityText(Activity a) {
    switch (a) {
        case ACT_HUNT:     return "HUNTING WIFI";
        case ACT_DISCOVER: return "DISCOVERING";
        case ACT_REST:     return "RESTING";
        default:           return "IDLE";
    }
}

// -- Stage text (short) ----------------------------------------------------
static const char* stageTextShort(EvolutionStage s) {
    switch (s) {
        case STAGE_EGG:      return "EGG";
        case STAGE_LARVA:    return "LARVA";
        case STAGE_NYMPH:    return "NYMPH";
        case STAGE_JUVENILE: return "JUV";
        case STAGE_ADULT:    return "ADULT";
        case STAGE_ELDER:    return "ELDER";
    }
    return "?";
}

// -- Stats block -----------------------------------------------------------
static void drawStats(TFT_eSprite& fb, const PetState& pet) {
    int x = 8;
    int y = 148;
    int barW = 100;
    int barH = 6;
    int step = 18;

    // Hunger
    fb.setTextFont(1);
    fb.setTextColor(Theme::FG_MUTED);
    fb.setCursor(x, y);
    fb.print("HNG");
    Theme::drawBar(fb, x + 30, y - 1, barW, barH, pet.hunger, Theme::ORANGE);

    // Happiness
    fb.setCursor(x, y + step);
    fb.print("HPY");
    Theme::drawBar(fb, x + 30, y + step - 1, barW, barH,
                   pet.happiness, Theme::ACCENT);

    // Health
    fb.setCursor(x, y + step * 2);
    fb.print("HP ");
    Theme::drawBar(fb, x + 30, y + step * 2 - 1, barW, barH,
                   pet.health, Theme::GREEN);

    // Mood + stage on the right
    fb.setTextColor(Theme::FG);
    fb.setTextDatum(TR_DATUM);
    fb.drawString(moodText(pet.mood), DISPLAY_W - 8, y);
    fb.setTextColor(Theme::FG_MUTED);
    fb.drawString(stageTextShort(pet.stage), DISPLAY_W - 8, y + step);
    fb.setTextDatum(TL_DATUM);
}

void Screens::home(TFT_eSprite& fb, TFT_eSprite& petSpr,
                   TFT_eSprite& effectSpr, const HomeState& state) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, activityText(state.activity));

    unsigned long now = millis();
    const PetState& pet = *state.pet;

    // -- REST ANIMATION ---------------------------------------------------
    if (state.activity == ACT_REST && state.restPhase != REST_NONE) {
        int frameIdx = 0;
        if (state.restPhase == REST_ENTER)
            frameIdx = 4 - constrain(state.restFrameIndex, 0, 4);
        else if (state.restPhase == REST_DEEP)
            frameIdx = 0;
        else if (state.restPhase == REST_WAKE)
            frameIdx = constrain(state.restFrameIndex, 0, 4);

        petSpr.pushImage(0, 0, PET_SPRITE_W, PET_SPRITE_H,
                        EGG_FRAMES[frameIdx]);
        petSpr.pushToSprite(&fb, PET_X, PET_Y, TFT_WHITE);
        drawStats(fb, pet);
        return;
    }

    // -- HUNTING ANIMATION ------------------------------------------------
    if (state.activity == ACT_HUNT) {
        if (now - s_lastHuntTime >= 300) {
            s_lastHuntTime = now;
            s_huntFrame = (s_huntFrame + 1) % 3;
        }
        petSpr.pushImage(0, 0, PET_SPRITE_W, PET_SPRITE_H,
                        ATTACK_FRAMES[s_huntFrame]);
        petSpr.pushToSprite(&fb, PET_X, PET_Y, TFT_WHITE);
        drawStats(fb, pet);
        return;
    }

    // -- IDLE ANIMATION ---------------------------------------------------
    int speed = IDLE_BASE_DELAY;
    if (pet.mood == MOOD_HAPPY)  speed = IDLE_FAST_DELAY;
    if (pet.mood == MOOD_SLEEPY || pet.mood == MOOD_SICK) speed = IDLE_SLOW_DELAY;

    if (now - s_lastIdleTime >= (unsigned long)speed) {
        s_lastIdleTime = now;
        s_idleFrame = (s_idleFrame + 1) % 4;
    }

    petSpr.pushImage(0, 0, PET_SPRITE_W, PET_SPRITE_H,
                    IDLE_FRAMES[s_idleFrame]);
    petSpr.pushToSprite(&fb, PET_X, PET_Y, TFT_WHITE);

    // -- HUNGER EFFECT OVERLAY --------------------------------------------
    if (state.hungerEffectActive && state.hungerEffectFrame < 4) {
        effectSpr.pushImage(0, 0, EFFECT_SPRITE_W, EFFECT_SPRITE_H,
                           HUNGER_FRAMES[state.hungerEffectFrame]);
        effectSpr.pushToSprite(&fb, 120, 60, TFT_WHITE);
    }

    drawStats(fb, pet);
}
