#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

// ==========================================================================
// WiFi Manager -- STA mode for Cosmania polling
// Separate from wifi_radio.h (scan-only HAL for feeding mechanic)
// ==========================================================================

static const char* s_ssid = nullptr;
static const char* s_pass = nullptr;
static unsigned long s_lastReconnect = 0;
static constexpr unsigned long RECONNECT_INTERVAL = 10000;

void WifiManager::init(const char* ssid, const char* pass) {
    s_ssid = ssid;
    s_pass = pass;

    if (!ssid || ssid[0] == '\0') return;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    s_lastReconnect = millis();
}

void WifiManager::tick() {
    if (!s_ssid || s_ssid[0] == '\0') return;
    if (WiFi.status() == WL_CONNECTED) return;

    unsigned long now = millis();
    if (now - s_lastReconnect >= RECONNECT_INTERVAL) {
        s_lastReconnect = now;
        WiFi.disconnect();
        WiFi.begin(s_ssid, s_pass);
    }
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

int WifiManager::rssi() {
    if (!isConnected()) return -100;
    return WiFi.RSSI();
}
