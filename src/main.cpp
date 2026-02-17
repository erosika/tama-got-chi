#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "hal/display.h"
#include "hal/buttons.h"
#include "hal/sound.h"
#include "hal/leds.h"
#include "hal/storage.h"
#include "hal/wifi_radio.h"
#include "hal/nfc.h"
#include "hal/gps.h"
#include "hal/haptics.h"
#include "hal/power.h"
#include "hal/ble.h"
#include "hal/wifi_promisc.h"
#include "net/wifi_manager.h"
#include "net/cosmania_client.h"
#include "state/pet_state.h"
#include "state/nfc_actions.h"
#include "state/location.h"
#include "state/threat_detect.h"
#include "ui/renderer.h"

// ==========================================================================
// TamaFi -- setup() + loop()
// Init/tick dispatch only. All logic lives in modules.
// ==========================================================================

// -- Global game state -----------------------------------------------------
static Screen       currentScreen   = SCREEN_BOOT;
static PetState     pet;
static WifiStats    wifiStats;
static CosmaniaStatus cosmania;
static Settings     settings;
static Activity     currentActivity = ACT_NONE;
static RestPhase    restPhase       = REST_NONE;
static LocationZone location        = LOC_HOME;
static RadioEnvironment radioEnv;
static int              lastThreatCount = 0;

// -- Rest animation state --------------------------------------------------
static int          restFrameIndex   = 0;
static unsigned long restDurationMs  = 0;
static bool         restStatsApplied = false;

// -- Hunger effect ---------------------------------------------------------
static bool         hungerEffectActive = false;
static int          hungerEffectFrame  = 0;

// -- Hatch -----------------------------------------------------------------
static bool         hatchTriggered = false;

// -- Menu state ------------------------------------------------------------
static int menuIndex     = 0;
static int settingsIndex = 0;
static int agentIndex    = 0;

// -- Timers ----------------------------------------------------------------
static unsigned long lastLogicTick = 0;
static unsigned long lastSaveTime  = 0;

// -- Pet logic context -----------------------------------------------------
static PetLogic::Context petCtx;

// -- Menu targets ----------------------------------------------------------
static const Screen MENU_TARGETS[] = {
    SCREEN_STATUS,      // 0: STATUS
    SCREEN_DASHBOARD,   // 1: AGENTS
    SCREEN_RADIO,       // 2: RADIO
    SCREEN_WIFI_SCAN,   // 3: WIFI SCAN
    SCREEN_SETTINGS,    // 4: SETTINGS
    SCREEN_SYSINFO,     // 5: SYSTEM INFO
    SCREEN_HOME,        // 6: BACK
};
static constexpr int MENU_ITEM_COUNT = sizeof(MENU_TARGETS) / sizeof(MENU_TARGETS[0]);
static constexpr int SETTINGS_ITEM_COUNT = 9;

// ==========================================================================
// Navigation
// ==========================================================================

static void switchScreen(Screen next) {
    currentScreen = next;
}

static void handleBootInput() {
    if (Buttons::upPressed() || Buttons::okPressed() || Buttons::downPressed()) {
        Sound::click();
        switchScreen(pet.hatched ? SCREEN_HOME : SCREEN_HATCH);
    }
}

static void handleHatchInput() {
    if (Buttons::okPressed() && !pet.hatched && !hatchTriggered) {
        Sound::click();
        Sound::hatch();
        hatchTriggered = true;
    }
}

static void handleHomeInput() {
    // Right-side quick access
    if (Buttons::r1Pressed()) {
        Sound::click();
        switchScreen(SCREEN_STATUS);
        return;
    }
    if (Buttons::r2Pressed()) {
        Sound::click();
        switchScreen(SCREEN_WIFI_SCAN);
        return;
    }
    if (Buttons::r3Pressed()) {
        Sound::click();
        switchScreen(SCREEN_SYSINFO);
        return;
    }
    // OK -> menu
    if (Buttons::okPressed()) {
        Sound::click();
        menuIndex = 0;
        switchScreen(SCREEN_MENU);
    }
}

static void handleMenuInput() {
    if (Buttons::upPressed()) {
        Sound::click();
        menuIndex = (menuIndex - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
    }
    if (Buttons::downPressed()) {
        Sound::click();
        menuIndex = (menuIndex + 1) % MENU_ITEM_COUNT;
    }
    if (Buttons::okPressed()) {
        Sound::click();
        switchScreen(MENU_TARGETS[menuIndex]);
    }
}

static void handleSettingsInput() {
    if (Buttons::upPressed()) {
        Sound::click();
        settingsIndex = (settingsIndex - 1 + SETTINGS_ITEM_COUNT) % SETTINGS_ITEM_COUNT;
    }
    if (Buttons::downPressed()) {
        Sound::click();
        settingsIndex = (settingsIndex + 1) % SETTINGS_ITEM_COUNT;
    }
    if (Buttons::okPressed()) {
        Sound::click();
        switch (settingsIndex) {
            case 0:
                settings.tftBrightness = (settings.tftBrightness + 1) % 3;
                Display::setBrightness(settings.tftBrightness);
                break;
            case 1:
                settings.ledBrightness = (settings.ledBrightness + 1) % 3;
                LEDs::setBrightness(settings.ledBrightness);
                break;
            case 2:
                settings.soundEnabled = !settings.soundEnabled;
                Sound::setEnabled(settings.soundEnabled);
                break;
            case 3:
                settings.neoPixelsEnabled = !settings.neoPixelsEnabled;
                LEDs::setEnabled(settings.neoPixelsEnabled);
                break;
            case 4:
                settings.autoSleep = !settings.autoSleep;
                break;
            case 5:
                if (settings.autoSaveMs == 15000) settings.autoSaveMs = 30000;
                else if (settings.autoSaveMs == 30000) settings.autoSaveMs = 60000;
                else settings.autoSaveMs = 15000;
                break;
            case 6:
                PetLogic::resetPet(petCtx, false);
                break;
            case 7:
                PetLogic::resetPet(petCtx, true);
                pet.stage   = STAGE_EGG;
                pet.hatched = false;
                Storage::save(pet, settings);
                hatchTriggered = false;
                switchScreen(SCREEN_HATCH);
                return;
            case 8:
                switchScreen(SCREEN_MENU);
                return;
        }
    }
}

static void handleGameoverInput() {
    if (Buttons::okPressed()) {
        Sound::click();
        PetLogic::resetPet(petCtx, true);
        pet.stage   = STAGE_EGG;
        pet.hatched = false;
        pet.alive   = true;
        Storage::save(pet, settings);
        hatchTriggered = false;
        switchScreen(SCREEN_HATCH);
    }
}

static void handleSimpleBackInput() {
    if (Buttons::okPressed()) {
        Sound::click();
        switchScreen(SCREEN_MENU);
    }
    // Right buttons also return to home from quick-access pages
    if (Buttons::r1Pressed() || Buttons::r2Pressed() || Buttons::r3Pressed()) {
        Sound::click();
        switchScreen(SCREEN_HOME);
    }
}

static void handleDashboardInput() {
    if (Buttons::okPressed()) {
        Sound::click();
        agentIndex = 0;
        switchScreen(SCREEN_AGENTS);
        return;
    }
    if (Buttons::downPressed() || Buttons::upPressed()) {
        Sound::click();
        switchScreen(SCREEN_MENU);
    }
}

static void handleAgentsInput() {
    if (Buttons::okPressed()) {
        Sound::click();
        switchScreen(SCREEN_DASHBOARD);
        return;
    }
    int count = cosmania.agentCount;
    if (count == 0) count = 1;
    if (Buttons::downPressed() || Buttons::r1Pressed()) {
        Sound::click();
        agentIndex = (agentIndex + 1) % count;
    }
    if (Buttons::upPressed() || Buttons::r3Pressed()) {
        Sound::click();
        agentIndex = (agentIndex - 1 + count) % count;
    }
}

static void handleInput() {
    switch (currentScreen) {
        case SCREEN_BOOT:       handleBootInput();       break;
        case SCREEN_HATCH:      handleHatchInput();      break;
        case SCREEN_HOME:       handleHomeInput();       break;
        case SCREEN_MENU:       handleMenuInput();       break;
        case SCREEN_SETTINGS:   handleSettingsInput();   break;
        case SCREEN_GAMEOVER:   handleGameoverInput();   break;
        case SCREEN_DASHBOARD:  handleDashboardInput();  break;
        case SCREEN_AGENTS:     handleAgentsInput();     break;
        case SCREEN_STATUS:
        case SCREEN_WIFI_SCAN:
        case SCREEN_SYSINFO:
        case SCREEN_LOCATION:
        case SCREEN_GLANCE:
        case SCREEN_REVIEW:
        case SCREEN_RADIO:      handleSimpleBackInput(); break;
        default: break;
    }
}

// ==========================================================================
// setup + loop
// ==========================================================================

void setup() {
    Serial.begin(115200);
    randomSeed(esp_random());

    Display::init();
    Buttons::init();
    Sound::init();
    LEDs::init();
    Storage::init();
    WifiRadio::init();
    NFC::init();
    GPS::init();
    Haptics::init();
    Power::init();
    BLE::init();
    WifiPromisc::init();
    ThreatDetect::init();
    Renderer::init();

    Storage::load(pet, settings);

    // WiFi STA + Cosmania (if configured)
    #if FEATURE_COSMANIA
    WifiManager::init(settings.wifiSsid, settings.wifiPass);
    CosmaniaClient::init(settings.cosmaniaUrl);
    #endif

    // Enable promiscuous mode for radio scanning
    #if FEATURE_SOVEREIGNTY
    WifiPromisc::enable();
    WifiPromisc::setChannelHopping(true);
    #endif

    // Apply loaded settings
    Display::setBrightness(settings.tftBrightness);
    LEDs::setBrightness(settings.ledBrightness);
    LEDs::setEnabled(settings.neoPixelsEnabled);
    Sound::setEnabled(settings.soundEnabled);

    // Build pet logic context
    petCtx.pet             = &pet;
    petCtx.wifi            = &wifiStats;
    petCtx.cosmania        = &cosmania;
    petCtx.settings        = &settings;
    petCtx.activity        = &currentActivity;
    petCtx.restPhase       = &restPhase;
    petCtx.restFrameIndex  = &restFrameIndex;
    petCtx.restDurationMs  = &restDurationMs;
    petCtx.restStatsApplied = &restStatsApplied;
    petCtx.radio            = &radioEnv;

    PetLogic::init(petCtx);
    Location::init(settings);

    currentScreen = SCREEN_BOOT;
    lastLogicTick = millis();
    lastSaveTime  = millis();

    Serial.println("[tamafi] boot");
}

void loop() {
    unsigned long now = millis();

    // HAL ticks (every loop iteration)
    Buttons::tick();
    Sound::tick();
    LEDs::tick();
    NFC::tick();
    GPS::tick();
    Haptics::tick();
    Power::tick();
    BLE::tick();
    WifiPromisc::tick();
    ThreatDetect::tick();
    radioEnv = ThreatDetect::environment();

    // Haptic alert on new threats
    int currentThreats = ThreatDetect::threatCount();
    if (currentThreats > lastThreatCount) {
        ThreatSeverity worst = radioEnv.worstThreat;
        if (worst == THREAT_CRITICAL) Haptics::alert();
        else                          Haptics::pulse();
        Sound::click();
    }
    lastThreatCount = currentThreats;

    // Network ticks
    #if FEATURE_COSMANIA
    WifiManager::tick();
    CosmaniaClient::tick();
    cosmania = CosmaniaClient::getStatus();
    #endif

    // NFC tap handling
    NFC::TapType tap = NFC::consumeTap();
    if (tap != NFC::TAP_NONE && currentScreen != SCREEN_BOOT &&
        currentScreen != SCREEN_HATCH && currentScreen != SCREEN_GAMEOVER) {
        NfcActions::Result nfcResult = NfcActions::process(
            tap, NFC::lastUID(), NFC::lastUIDLen(), settings);

        if (nfcResult.locationOverride) {
            Location::setOverride(nfcResult.overrideZone);
        }

        Sound::click();
        switchScreen(nfcResult.targetScreen);
    }

    // Input handling
    handleInput();

    // Logic tick (100ms, only when game is active)
    if (now - lastLogicTick >= LOGIC_TICK_MS) {
        lastLogicTick = now;

        if (currentScreen != SCREEN_BOOT &&
            currentScreen != SCREEN_HATCH &&
            currentScreen != SCREEN_GAMEOVER) {
            PetLogic::tick(petCtx);
            Location::tick(settings);
            location = Location::current();

            // Death transition
            if (!pet.alive && currentScreen != SCREEN_GAMEOVER) {
                switchScreen(SCREEN_GAMEOVER);
            }
        }
    }

    // Auto-save
    if (now - lastSaveTime >= settings.autoSaveMs) {
        lastSaveTime = now;
        Storage::save(pet, settings);
    }

    // Build draw context
    Renderer::DrawContext ctx = {};
    ctx.screen             = currentScreen;
    ctx.menuIndex          = menuIndex;
    ctx.settingsIndex      = settingsIndex;
    ctx.pet                = &pet;
    ctx.settings           = &settings;
    ctx.wifi               = &wifiStats;
    ctx.cosmania           = &cosmania;
    ctx.activity           = currentActivity;
    ctx.restPhase          = restPhase;
    ctx.restFrameIndex     = restFrameIndex;
    ctx.hungerEffectActive = hungerEffectActive;
    ctx.hungerEffectFrame  = hungerEffectFrame;
    ctx.hatchTriggered     = hatchTriggered;
    ctx.agentIndex         = agentIndex;
    ctx.location           = location;
    ctx.radio              = &radioEnv;
    ctx.threats            = ThreatDetect::threats();
    ctx.threatCount        = ThreatDetect::threatCount();

    Renderer::draw(ctx);

    // Hatch completion (checked after draw)
    if (currentScreen == SCREEN_HATCH && Renderer::wasHatchComplete()) {
        pet.hatched    = true;
        hatchTriggered = false;
        Storage::save(pet, settings);
        switchScreen(SCREEN_HOME);
    }
}
