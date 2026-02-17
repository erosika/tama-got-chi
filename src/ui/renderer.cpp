#include "renderer.h"
#include "screens/boot.h"
#include "screens/hatch.h"
#include "screens/home.h"
#include "screens/menu.h"
#include "screens/status.h"
#include "screens/wifi_scan.h"
#include "screens/settings.h"
#include "screens/sysinfo.h"
#include "screens/gameover.h"
#include "screens/dashboard.h"
#include "screens/agents.h"
#include "screens/glance.h"
#include "screens/review.h"
#include "screens/location_view.h"
#include "screens/radio.h"
#include "../hal/display.h"
#include "../hal/gps.h"

// ==========================================================================
// Renderer -- Dispatches to screen functions, pushes framebuffer
// ==========================================================================

// Set by hatch screen when animation completes
static bool s_hatchComplete = false;

void Renderer::init() {
    s_hatchComplete = false;
}

void Renderer::draw(const DrawContext& ctx) {
    TFT_eSprite& fb  = Display::fb();
    TFT_eSprite& pet = Display::petSprite();
    TFT_eSprite& eff = Display::effectSprite();

    s_hatchComplete = false;

    switch (ctx.screen) {
        case SCREEN_BOOT:
            Screens::boot(fb);
            break;

        case SCREEN_HATCH:
            s_hatchComplete = Screens::hatch(fb, pet,
                ctx.pet ? ctx.pet->hatched : false,
                ctx.hatchTriggered);
            break;

        case SCREEN_HOME: {
            Screens::HomeState hs = {};
            hs.pet               = ctx.pet;
            hs.activity          = ctx.activity;
            hs.restPhase         = ctx.restPhase;
            hs.restFrameIndex    = ctx.restFrameIndex;
            hs.hungerEffectActive = ctx.hungerEffectActive;
            hs.hungerEffectFrame = ctx.hungerEffectFrame;
            Screens::home(fb, pet, eff, hs);
            break;
        }

        case SCREEN_MENU:
            Screens::menu(fb, ctx.menuIndex);
            break;

        case SCREEN_STATUS:
            if (ctx.pet) Screens::status(fb, *ctx.pet);
            break;

        case SCREEN_WIFI_SCAN:
            if (ctx.wifi) Screens::wifiScan(fb, *ctx.wifi);
            break;

        case SCREEN_SETTINGS:
            if (ctx.settings)
                Screens::settings(fb, ctx.settingsIndex, *ctx.settings);
            break;

        case SCREEN_SYSINFO:
            Screens::sysinfo(fb);
            break;

        case SCREEN_GAMEOVER:
            Screens::gameover(fb, pet);
            break;

        case SCREEN_DASHBOARD:
            if (ctx.cosmania) Screens::dashboard(fb, *ctx.cosmania);
            break;

        case SCREEN_AGENTS:
            if (ctx.cosmania) Screens::agents(fb, *ctx.cosmania, ctx.agentIndex);
            break;

        case SCREEN_GLANCE:
            if (ctx.pet && ctx.cosmania)
                Screens::glance(fb, *ctx.pet, *ctx.cosmania);
            break;

        case SCREEN_REVIEW:
            if (ctx.cosmania) Screens::review(fb, *ctx.cosmania);
            break;

        case SCREEN_LOCATION:
            Screens::locationView(fb, ctx.location,
                GPS::hasFix(), GPS::satellites());
            break;

        case SCREEN_RADIO:
            if (ctx.radio)
                Screens::radio(fb, *ctx.radio, ctx.threats, ctx.threatCount);
            break;

        default:
            Screens::boot(fb);
            break;
    }

    Display::push();
}

bool Renderer::wasHatchComplete() {
    return s_hatchComplete;
}
