#include "pet_state.h"
#include "config.h"
#include "mood.h"
#include "evolution.h"
#include "../hal/sound.h"
#include "../hal/leds.h"
#include "../hal/wifi_radio.h"
#include <Arduino.h>

// ==========================================================================
// Pet State -- Stat decay, WiFi feeding, autonomous decisions
// (ported from upstream TamaFi.ino with new type system)
// ==========================================================================

// -- Timers ----------------------------------------------------------------
static unsigned long s_hungerTimer    = 0;
static unsigned long s_happyTimer     = 0;
static unsigned long s_healthTimer    = 0;
static unsigned long s_ageTimer       = 0;
static unsigned long s_decisionTimer  = 0;
static uint32_t      s_decisionInterval = 10000;
static unsigned long s_lastScanTime   = 0;

// -- Rest state ------------------------------------------------------------
static unsigned long s_restPhaseStart = 0;
static unsigned long s_restAnimTime   = 0;

// -- Hunger effect ---------------------------------------------------------
static bool          s_hungerEffect    = false;
static int           s_hungerFrame     = 0;
static unsigned long s_hungerFrameTime = 0;

// -- Personality traits (randomized on first boot) -------------------------
static uint8_t s_curiosity = 70;
static uint8_t s_activity  = 60;
static uint8_t s_stress    = 40;

void PetLogic::init(Context& ctx) {
    unsigned long now = millis();
    s_hungerTimer    = now;
    s_happyTimer     = now;
    s_healthTimer    = now;
    s_ageTimer       = now;
    s_decisionTimer  = now;
    s_restAnimTime   = now;
    s_hungerFrameTime = now;
    s_lastScanTime   = 0;

    // Randomize traits on first boot (stage == EGG means fresh)
    if (ctx.pet->stage == STAGE_EGG && !ctx.pet->hatched) {
        s_curiosity = random(40, 90);
        s_activity  = random(30, 90);
        s_stress    = random(20, 80);
    }
}

// -- Stat decay ------------------------------------------------------------
static void decayStats(PetLogic::Context& ctx) {
    unsigned long now = millis();
    PetState& p = *ctx.pet;

    if (now - s_hungerTimer >= HUNGER_DECAY_MS) {
        p.hunger = max(0, p.hunger - 2);
        s_hungerTimer = now;
    }

    if (now - s_happyTimer >= HAPPINESS_DECAY_MS) {
        // Cosmania-driven happiness trends
        if (ctx.cosmania && ctx.cosmania->connected) {
            BudgetTier tier = ctx.cosmania->budgetTier;
            if (tier == TIER_GREEN && ctx.cosmania->errorCount == 0) {
                // GREEN + no errors: trend toward 90
                if (p.happiness < 90) p.happiness = min(100, p.happiness + 1);
                else p.happiness = max(0, p.happiness - 1);
            } else if (tier == TIER_YELLOW || ctx.cosmania->overdueCount > 0) {
                p.happiness = max(0, p.happiness - 2);
            } else if (tier == TIER_RED || tier == TIER_BLACK) {
                p.happiness = max(0, p.happiness - 3);
            } else {
                p.happiness = max(0, p.happiness - 1);
            }
        } else if (ctx.wifi->netCount == 0 && s_lastScanTime > 0 &&
                   (now - s_lastScanTime) > 30000) {
            // Disconnected: faster decay
            p.happiness = max(0, p.happiness - 3);
        } else {
            p.happiness = max(0, p.happiness - 1);
        }
        s_happyTimer = now;
    }

    if (now - s_healthTimer >= HEALTH_DECAY_MS) {
        // Cosmania-driven health: budgetTier sets target
        if (ctx.cosmania && ctx.cosmania->connected) {
            int target = 70;
            switch (ctx.cosmania->budgetTier) {
                case TIER_GREEN:  target = 90; break;
                case TIER_YELLOW: target = 65; break;
                case TIER_RED:    target = 30; break;
                case TIER_BLACK:  target = 10; break;
                default:          target = 70; break;
            }
            if (p.health > target) p.health = max(0, p.health - 2);
            else if (p.health < target) p.health = min(100, p.health + 1);
        } else if (p.hunger < 20 || p.happiness < 20) {
            p.health = max(0, p.health - 2);
        } else {
            p.health = max(0, p.health - 1);
        }
        s_healthTimer = now;
    }

    // Age
    if (now - s_ageTimer >= AGE_TICK_MS) {
        p.ageMinutes++;
        if (p.ageMinutes >= 60) {
            p.ageMinutes -= 60;
            p.ageHours++;
        }
        if (p.ageHours >= 24) {
            p.ageHours -= 24;
            p.ageDays++;
        }
        s_ageTimer = now;
    }
}

// -- WiFi feeding ----------------------------------------------------------
void PetLogic::resolveHunt(Context& ctx) {
    PetState& p = *ctx.pet;
    WifiStats& w = *ctx.wifi;

    if (w.netCount == 0) {
        p.hunger    = max(0, p.hunger - 15);
        p.happiness = max(0, p.happiness - 10);
        p.health    = max(0, p.health - 5);
        Sound::badFeed();
        LEDs::sad();
    } else {
        int hungerDelta = min(35, w.netCount * 2 + w.strongCount * 3);
        int varietyScore = w.hiddenCount * 2 + w.openCount;
        int happyDelta  = min(30, varietyScore * 3 + (w.avgRSSI + 100) / 3);
        int healthDelta = 0;
        if (w.avgRSSI > -75) healthDelta += 5;
        if (w.avgRSSI > -65) healthDelta += 5;
        if (w.strongCount > 5) healthDelta += 3;

        p.hunger    = min(100, p.hunger + hungerDelta);
        p.happiness = min(100, p.happiness + happyDelta);
        p.health    = min(100, p.health + healthDelta);
        Sound::goodFeed();
        LEDs::happy();
    }

    s_hungerEffect    = true;
    s_hungerFrame     = 0;
    s_hungerFrameTime = millis();
}

void PetLogic::resolveDiscover(Context& ctx) {
    PetState& p = *ctx.pet;
    WifiStats& w = *ctx.wifi;

    if (w.netCount == 0) {
        p.happiness = max(0, p.happiness - 5);
        p.hunger    = max(0, p.hunger - 3);
        Sound::badFeed();
    } else {
        int curiosity = w.hiddenCount * 4 + w.openCount * 3 + w.netCount;
        int happyDelta = min(35, curiosity / 2);
        p.happiness = min(100, p.happiness + happyDelta);
        p.hunger    = max(0, p.hunger - 5);
        Sound::discover();
    }
}

// -- Rest state machine ----------------------------------------------------
static void stepRest(PetLogic::Context& ctx) {
    if (*ctx.activity != ACT_REST || *ctx.restPhase == REST_NONE) return;

    unsigned long now = millis();
    PetState& p = *ctx.pet;

    switch (*ctx.restPhase) {
        case REST_ENTER:
            if (now - s_restAnimTime >= REST_ENTER_DELAY) {
                s_restAnimTime = now;
                if (*ctx.restFrameIndex > 0) {
                    (*ctx.restFrameIndex)--;
                } else {
                    *ctx.restFrameIndex = 0;
                    *ctx.restPhase = REST_DEEP;
                    s_restPhaseStart = now;
                    *ctx.restStatsApplied = false;
                }
            }
            break;

        case REST_DEEP:
            LEDs::breathe(now - s_restPhaseStart);

            if (!*ctx.restStatsApplied &&
                (now - s_restPhaseStart > *ctx.restDurationMs / 2)) {
                p.hunger    = max(0, p.hunger - 3);
                p.happiness = min(100, p.happiness + 10);
                p.health    = min(100, p.health + 15);
                *ctx.restStatsApplied = true;
            }

            if (now - s_restPhaseStart >= *ctx.restDurationMs) {
                *ctx.restPhase = REST_WAKE;
                s_restPhaseStart = now;
                s_restAnimTime   = now;
                *ctx.restFrameIndex = 0;
                Sound::restEnd();
                LEDs::off();
            }
            break;

        case REST_WAKE:
            if (now - s_restAnimTime >= REST_WAKE_DELAY) {
                s_restAnimTime = now;
                if (*ctx.restFrameIndex < 4) {
                    (*ctx.restFrameIndex)++;
                } else {
                    *ctx.restFrameIndex = 4;
                    *ctx.restPhase = REST_NONE;
                    *ctx.activity  = ACT_NONE;
                }
            }
            break;

        default:
            break;
    }
}

// -- Autonomous decisions --------------------------------------------------
static void decideActivity(PetLogic::Context& ctx) {
    if (*ctx.activity != ACT_NONE || *ctx.restPhase != REST_NONE) return;

    unsigned long now = millis();
    if (now - s_decisionTimer < s_decisionInterval) return;

    s_decisionTimer = now;
    s_decisionInterval = random(DECISION_INTERVAL_MIN, DECISION_INTERVAL_MAX);

    PetState& p = *ctx.pet;
    WifiStats& w = *ctx.wifi;

    int desireHunt = (100 - p.hunger) + s_curiosity / 2;
    int desireDisc = s_curiosity + w.hiddenCount * 10 + w.openCount * 6 +
                     w.netCount * 2 + random(0, 20);
    int desireRest = (100 - p.health) + s_stress / 2;
    int desireIdle = 10;

    if (w.netCount == 0) { desireHunt /= 2; desireDisc /= 2; }
    if (p.hunger < 20)   desireRest -= 10;

    // Mood modifiers
    Mood m = p.mood;
    if (m == MOOD_HUNGRY)  { desireHunt += 20; desireRest -= 10; }
    if (m == MOOD_ANXIOUS) { desireDisc += 15; }
    if (m == MOOD_SICK)    { desireRest += 20; desireDisc -= 10; }
    if (m == MOOD_HAPPY)   { desireDisc += 10; desireHunt += 5; }
    if (m == MOOD_SLEEPY)  { desireDisc += 10; desireHunt += 5; }

    desireHunt = max(desireHunt, 0);
    desireDisc = max(desireDisc, 0);
    desireRest = max(desireRest, 0);
    desireIdle = max(desireIdle, 0);

    int best = desireIdle;
    Activity chosen = ACT_NONE;
    if (desireHunt > best) { best = desireHunt; chosen = ACT_HUNT; }
    if (desireDisc > best) { best = desireDisc; chosen = ACT_DISCOVER; }
    if (desireRest > best) { best = desireRest; chosen = ACT_REST; }

    if (chosen == ACT_NONE) return;

    if (chosen == ACT_HUNT || chosen == ACT_DISCOVER) {
        *ctx.activity = chosen;
        LEDs::wifi();
        WifiRadio::startScan();
    } else if (chosen == ACT_REST) {
        *ctx.activity         = ACT_REST;
        *ctx.restPhase        = REST_ENTER;
        *ctx.restFrameIndex   = 4;
        *ctx.restDurationMs   = random(REST_MIN_DURATION, REST_MAX_DURATION);
        *ctx.restStatsApplied = false;
        s_restAnimTime        = millis();
        s_restPhaseStart      = millis();
        Sound::restStart();
        LEDs::rest();
    }
}

// -- Main tick -------------------------------------------------------------
void PetLogic::tick(Context& ctx) {
    decayStats(ctx);

    // Hunger effect animation
    unsigned long now = millis();
    if (s_hungerEffect && now - s_hungerFrameTime >= HUNGER_EFFECT_DELAY) {
        s_hungerFrameTime = now;
        s_hungerFrame++;
        if (s_hungerFrame >= HUNGER_FRAME_COUNT) {
            s_hungerEffect = false;
            LEDs::off();
        }
    }

    // WiFi activity resolution
    if (*ctx.activity == ACT_HUNT || *ctx.activity == ACT_DISCOVER) {
        if (WifiRadio::isScanDone()) {
            *ctx.wifi = WifiRadio::getResults();
            s_lastScanTime = millis();
            if (*ctx.activity == ACT_HUNT)      resolveHunt(ctx);
            else if (*ctx.activity == ACT_DISCOVER) resolveDiscover(ctx);
            *ctx.activity = ACT_NONE;
            LEDs::off();
        }
    }

    // Rest
    stepRest(ctx);

    // Mood + evolution (Cosmania-driven when connected, radio-aware)
    const CosmaniaStatus& cs = ctx.cosmania ? *ctx.cosmania : CosmaniaStatus();
    const RadioEnvironment& re = ctx.radio ? *ctx.radio : RadioEnvironment();
    MoodLogic::update(*ctx.pet, *ctx.wifi, cs, re, s_lastScanTime);
    Evolution::update(*ctx.pet, cs);

    // Death check
    PetState& p = *ctx.pet;
    if (p.hunger <= 0 && p.happiness <= 0 && p.health <= 0) {
        p.alive = false;
        LEDs::sad();
    }

    // Autonomous decisions (only when on home screen, idle)
    decideActivity(ctx);
}

void PetLogic::resetPet(Context& ctx, bool fullReset) {
    ctx.pet->hunger    = 70;
    ctx.pet->happiness = 70;
    ctx.pet->health    = 70;
    if (fullReset) {
        ctx.pet->ageMinutes = 0;
        ctx.pet->ageHours   = 0;
        ctx.pet->ageDays    = 0;
    }

    *ctx.wifi     = WifiStats();
    *ctx.activity = ACT_NONE;
    *ctx.restPhase = REST_NONE;
    s_hungerEffect = false;
    s_lastScanTime = 0;
    LEDs::off();

    unsigned long now = millis();
    s_hungerTimer   = now;
    s_happyTimer    = now;
    s_healthTimer   = now;
    s_ageTimer      = now;
    s_decisionTimer = now;
}
