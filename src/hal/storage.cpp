#include "storage.h"
#include <Preferences.h>

// ==========================================================================
// Storage HAL -- NVS persistence (ported from upstream TamaFi.ino)
// ==========================================================================

static Preferences s_prefs;

void Storage::init() {
    s_prefs.begin("tamafi2", false);
}

void Storage::save(const PetState& pet, const Settings& settings) {
    s_prefs.putInt("hunger",   pet.hunger);
    s_prefs.putInt("happy",    pet.happiness);
    s_prefs.putInt("health",   pet.health);
    s_prefs.putULong("ageMin", pet.ageMinutes);
    s_prefs.putULong("ageHr",  pet.ageHours);
    s_prefs.putULong("ageDay", pet.ageDays);
    s_prefs.putUChar("stage",  static_cast<uint8_t>(pet.stage));
    s_prefs.putBool("hatched", pet.hatched);

    s_prefs.putBool("sound",   settings.soundEnabled);
    s_prefs.putBool("neo",     settings.neoPixelsEnabled);
    s_prefs.putUChar("tftBri", settings.tftBrightness);
    s_prefs.putUChar("ledBri", settings.ledBrightness);
    s_prefs.putBool("sleep",   settings.autoSleep);
    s_prefs.putUShort("saveMs", settings.autoSaveMs);
}

void Storage::load(PetState& pet, Settings& settings) {
    int h = s_prefs.getInt("hunger", -1);
    if (h == -1) {
        // First boot: defaults already set by struct initializers
        return;
    }

    pet.hunger     = s_prefs.getInt("hunger", 70);
    pet.happiness  = s_prefs.getInt("happy",  70);
    pet.health     = s_prefs.getInt("health", 70);
    pet.ageMinutes = s_prefs.getULong("ageMin", 0);
    pet.ageHours   = s_prefs.getULong("ageHr",  0);
    pet.ageDays    = s_prefs.getULong("ageDay", 0);
    pet.stage      = static_cast<EvolutionStage>(s_prefs.getUChar("stage", 0));
    pet.hatched    = s_prefs.getBool("hatched", false);

    settings.soundEnabled     = s_prefs.getBool("sound", true);
    settings.neoPixelsEnabled = s_prefs.getBool("neo",   true);
    settings.tftBrightness    = s_prefs.getUChar("tftBri", 1);
    settings.ledBrightness    = s_prefs.getUChar("ledBri", 1);
    settings.autoSleep        = s_prefs.getBool("sleep", true);
    settings.autoSaveMs       = s_prefs.getUShort("saveMs", 30000);
}
