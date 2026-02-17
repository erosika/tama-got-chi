#include "power.h"
#include "display.h"
#include "config.h"
#include <Arduino.h>

// ==========================================================================
// Power -- Display sleep, battery monitoring, wake sources
// ==========================================================================

static bool s_displaySleeping = false;
static unsigned long s_lastActivityMs = 0;
static constexpr unsigned long AUTO_SLEEP_MS = 60000; // 1 minute idle

// Battery ADC (if wired -- placeholder pin, ESP32-S3 ADC)
static constexpr int PIN_BATTERY_ADC = -1; // Not connected yet

void Power::init() {
    s_lastActivityMs = millis();
    s_displaySleeping = false;
}

void Power::tick() {
    // Auto-sleep after inactivity
    if (!s_displaySleeping && (millis() - s_lastActivityMs) > AUTO_SLEEP_MS) {
        sleepDisplay();
    }
}

void Power::sleepDisplay() {
    if (s_displaySleeping) return;
    s_displaySleeping = true;
    Display::setBrightness(0);
    Serial.println("[power] display sleep");
}

void Power::wakeDisplay() {
    if (!s_displaySleeping) return;
    s_displaySleeping = false;
    s_lastActivityMs = millis();
    Display::setBrightness(1); // Restore to mid
    Serial.println("[power] display wake");
}

bool Power::isDisplaySleeping() {
    return s_displaySleeping;
}

float Power::batteryVoltage() {
    if (PIN_BATTERY_ADC < 0) return 0.0f;
    int raw = analogRead(PIN_BATTERY_ADC);
    // Typical voltage divider: 2x ratio, 3.3V ADC ref, 12-bit
    return (raw / 4095.0f) * 3.3f * 2.0f;
}

int Power::batteryPercent() {
    float v = batteryVoltage();
    if (v <= 0) return -1; // Not connected
    // LiPo: 3.0V = 0%, 4.2V = 100%
    int pct = (int)((v - 3.0f) / 1.2f * 100.0f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

void Power::enableWakeOnButton() {
    // Configure light sleep wake on button GPIOs
    // ESP32-S3 ext0/ext1 wake -- configured when entering light sleep
}

void Power::enableWakeOnNFC() {
    // PN532 IRQ line can trigger GPIO wake
    // Configured when entering light sleep
}
