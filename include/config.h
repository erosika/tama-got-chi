#pragma once

// ==========================================================================
// TamaFi -- Hardware pin map & compile-time constants
// ==========================================================================

// -- Display ---------------------------------------------------------------
#define DISPLAY_W         240
#define DISPLAY_H         240
#define PET_SPRITE_W      115
#define PET_SPRITE_H      110
#define EFFECT_SPRITE_W   100
#define EFFECT_SPRITE_H   95

// -- Buttons (upstream PCB) ------------------------------------------------
#define PIN_BTN_UP        13
#define PIN_BTN_OK        12
#define PIN_BTN_DOWN      11
#define PIN_BTN_R1         8
#define PIN_BTN_R2         9
#define PIN_BTN_R3        10

// -- NeoPixels -------------------------------------------------------------
#define PIN_LED_DATA       1
#define LED_COUNT          4

// -- Buzzer ----------------------------------------------------------------
#define PIN_BUZZER         2
#define BUZZER_PWM_CH      5

// -- TFT backlight ---------------------------------------------------------
#define PIN_TFT_BL         7
#define TFT_BL_PWM_CH     0

// -- NFC (I2C) -- expansion ------------------------------------------------
#define PIN_NFC_SDA        4
#define PIN_NFC_SCL        5

// -- Haptic motor -- expansion ---------------------------------------------
#define PIN_HAPTIC         6

// -- GPS (UART) -- expansion -----------------------------------------------
#define PIN_GPS_TX        15
#define PIN_GPS_RX        16

// -- Feature flags (defaults; overridden by platformio.ini build_flags) ----
#ifndef FEATURE_NFC
#define FEATURE_NFC       0
#endif
#ifndef FEATURE_GPS
#define FEATURE_GPS       0
#endif
#ifndef FEATURE_HAPTICS
#define FEATURE_HAPTICS   0
#endif
#ifndef FEATURE_COSMANIA
#define FEATURE_COSMANIA  0
#endif
#ifndef FEATURE_SOVEREIGNTY
#define FEATURE_SOVEREIGNTY 0
#endif

// -- Stat decay timing (ms) ------------------------------------------------
#define HUNGER_DECAY_MS       5000
#define HAPPINESS_DECAY_MS    7000
#define HEALTH_DECAY_MS      10000
#define AGE_TICK_MS          60000
#define LOGIC_TICK_MS          100
#define DEFAULT_SAVE_MS      30000
#define COSMANIA_POLL_MS     30000

// -- Animation timing (ms) -------------------------------------------------
#define IDLE_BASE_DELAY       200
#define IDLE_FAST_DELAY       120
#define IDLE_SLOW_DELAY       280
#define EGG_IDLE_DELAY        350
#define HATCH_DELAY           300
#define HUNGER_EFFECT_DELAY   100
#define HUNGER_FRAME_COUNT      4
#define REST_ENTER_DELAY      400
#define REST_WAKE_DELAY       400
#define REST_BREATHE_MS       400
#define REST_MIN_DURATION    5000
#define REST_MAX_DURATION   15000
#define DEAD_DELAY            300
#define DEAD_FRAME_COUNT        3
#define MENU_ANIM_INTERVAL     16
#define MENU_ANIM_STEP          3

// -- WiFi activity decision -------------------------------------------------
#define DECISION_INTERVAL_MIN  8000
#define DECISION_INTERVAL_MAX 15000

// -- Sovereignty: radio scanning -------------------------------------------
#define MAX_BLE_DEVICES         64
#define MAX_PROBE_REQUESTS      32
#define MAX_THREATS             16
#define BLE_SCAN_INTERVAL_MS  5000
#define BLE_SCAN_DURATION_S      3
#define PROMISC_CHANNEL_HOP_MS 500
#define THREAT_EXPIRE_MS     300000  // 5 minutes
#define ROGUE_SCAN_THRESHOLD    10   // scans/min to flag
#define DEAUTH_THRESHOLD         5   // deauths in window
#define DEAUTH_WINDOW_MS     10000
#define MASS_ENUM_THRESHOLD     20   // BLE scans from single source
