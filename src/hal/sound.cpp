#include "sound.h"
#include "config.h"
#include <Arduino.h>

// ==========================================================================
// Sound HAL -- Non-blocking retro sequencer (ported from upstream TamaFi.ino)
// ==========================================================================

static bool s_enabled = true;

struct RetroSound {
    const int* freqs;
    const int* times;
    int length;
};

// -- Sound definitions -----------------------------------------------------
static const int CLICK_FREQS[]      = { 2100, 1600, 900 };
static const int CLICK_TIMES[]      = {   20,   20,  20 };
static const RetroSound SND_CLICK   = { CLICK_FREQS, CLICK_TIMES, 3 };

static const int GOOD_FREQS[]       = { 600, 900, 1200, 1500 };
static const int GOOD_TIMES[]       = {  40,  40,   40,   60 };
static const RetroSound SND_GOOD    = { GOOD_FREQS, GOOD_TIMES, 4 };

static const int BAD_FREQS[]        = { 900, 700, 500, 300 };
static const int BAD_TIMES[]        = {  50,  50,  60,  80 };
static const RetroSound SND_BAD     = { BAD_FREQS, BAD_TIMES, 4 };

static const int DISC_FREQS[]       = { 400, 650, 900, 1200, 1500 };
static const int DISC_TIMES[]       = {  40,  40,  40,   40,   60 };
static const RetroSound SND_DISC    = { DISC_FREQS, DISC_TIMES, 5 };

static const int REST_S_FREQS[]     = { 600, 400, 300 };
static const int REST_S_TIMES[]     = {  60,  70,  90 };
static const RetroSound SND_REST_S  = { REST_S_FREQS, REST_S_TIMES, 3 };

static const int REST_E_FREQS[]     = { 300, 500, 700 };
static const int REST_E_TIMES[]     = {  60,  60,  80 };
static const RetroSound SND_REST_E  = { REST_E_FREQS, REST_E_TIMES, 3 };

static const int HATCH_FREQS[]      = { 500, 800, 1200, 1600, 2000 };
static const int HATCH_TIMES[]      = {  60,  60,   60,   80,  100 };
static const RetroSound SND_HATCH   = { HATCH_FREQS, HATCH_TIMES, 5 };

static const RetroSound* ALL_SOUNDS[] = {
    &SND_CLICK, &SND_GOOD, &SND_BAD, &SND_DISC,
    &SND_REST_S, &SND_REST_E, &SND_HATCH,
};
static constexpr int SOUND_COUNT = sizeof(ALL_SOUNDS) / sizeof(ALL_SOUNDS[0]);

// -- Sequencer state -------------------------------------------------------
static int s_index = -1;   // -1 = idle
static int s_step  = 0;
static unsigned long s_next = 0;

void Sound::init() {
    ledcSetup(BUZZER_PWM_CH, 4000, 8);
    ledcAttachPin(PIN_BUZZER, BUZZER_PWM_CH);
    ledcWriteTone(BUZZER_PWM_CH, 0);
}

void Sound::tick() {
    if (!s_enabled) {
        if (s_index >= 0) {
            ledcWriteTone(BUZZER_PWM_CH, 0);
            s_index = -1;
            s_step  = 0;
        }
        return;
    }

    if (s_index < 0 || s_index >= SOUND_COUNT) return;

    unsigned long now = millis();
    if (now < s_next) return;

    const RetroSound* snd = ALL_SOUNDS[s_index];

    if (s_step >= snd->length) {
        ledcWriteTone(BUZZER_PWM_CH, 0);
        s_index = -1;
        s_step  = 0;
        return;
    }

    ledcWriteTone(BUZZER_PWM_CH, snd->freqs[s_step]);
    s_next = now + snd->times[s_step];
    s_step++;
}

static void play(int idx) {
    if (!s_enabled) return;
    s_index = idx;
    s_step  = 0;
    s_next  = 0;
}

void Sound::click()     { play(0); }
void Sound::goodFeed()  { play(1); }
void Sound::badFeed()   { play(2); }
void Sound::discover()  { play(3); }
void Sound::restStart() { play(4); }
void Sound::restEnd()   { play(5); }
void Sound::hatch()     { play(6); }

void Sound::setEnabled(bool on) {
    s_enabled = on;
    if (!on) {
        ledcWriteTone(BUZZER_PWM_CH, 0);
        s_index = -1;
        s_step  = 0;
    }
}

bool Sound::isEnabled() { return s_enabled; }
