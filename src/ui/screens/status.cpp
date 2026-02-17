#include "status.h"
#include "../theme.h"
#include "config.h"
#include "../../state/evolution.h"
#include <TFT_eSPI.h>

// ==========================================================================
// Pet Status screen -- detailed stats view
// ==========================================================================

static const char* moodName(Mood m) {
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

void Screens::status(TFT_eSprite& fb, const PetState& pet) {
    fb.fillSprite(Theme::BG);
    Theme::drawHeader(fb, "STATUS");

    int y = 32;
    int step = 16;

    fb.setTextFont(1);
    fb.setTextSize(1);

    auto row = [&](const char* label, const char* value,
                   uint16_t valColor = Theme::FG) {
        fb.setTextColor(Theme::FG_MUTED);
        fb.setCursor(8, y);
        fb.print(label);
        fb.setTextColor(valColor);
        fb.setTextDatum(TR_DATUM);
        fb.drawString(value, DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);
        y += step;
    };

    row("STAGE", Evolution::stageName(pet.stage), Theme::ACCENT);

    char ageStr[24];
    snprintf(ageStr, sizeof(ageStr), "%lud %luh %lum",
             (unsigned long)pet.ageDays,
             (unsigned long)pet.ageHours,
             (unsigned long)pet.ageMinutes);
    row("AGE", ageStr);

    Theme::drawRule(fb, y + 2, Theme::BORDER);
    y += 10;

    // Stat bars
    auto statRow = [&](const char* label, int value, uint16_t color) {
        fb.setTextColor(Theme::FG_MUTED);
        fb.setCursor(8, y);
        fb.print(label);
        Theme::drawBar(fb, 65, y - 1, 100, 6, value, color);

        char pct[8];
        snprintf(pct, sizeof(pct), "%d%%", value);
        fb.setTextColor(Theme::FG);
        fb.setTextDatum(TR_DATUM);
        fb.drawString(pct, DISPLAY_W - 8, y);
        fb.setTextDatum(TL_DATUM);
        y += step;
    };

    statRow("HUNGER", pet.hunger, Theme::ORANGE);
    statRow("HAPPY",  pet.happiness, Theme::ACCENT);
    statRow("HEALTH", pet.health, Theme::GREEN);

    Theme::drawRule(fb, y + 2, Theme::BORDER);
    y += 10;

    row("MOOD", moodName(pet.mood),
        pet.mood <= MOOD_ANXIOUS ? Theme::RED : Theme::FG);
    row("ALIVE", pet.alive ? "YES" : "NO",
        pet.alive ? Theme::GREEN : Theme::RED);

    fb.setTextColor(Theme::FG_MUTED);
    fb.setTextDatum(BC_DATUM);
    fb.drawString("OK = BACK", DISPLAY_W / 2, DISPLAY_H - 8);
    fb.setTextDatum(TL_DATUM);
}
