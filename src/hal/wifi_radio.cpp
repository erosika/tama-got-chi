#include "wifi_radio.h"
#include <WiFi.h>

// ==========================================================================
// WiFi Radio HAL -- Async scan (ported from upstream TamaFi.ino)
// ==========================================================================

static bool s_scanning = false;

void WifiRadio::init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
}

void WifiRadio::startScan() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    WiFi.scanNetworks(true);    // async
    s_scanning = true;
}

bool WifiRadio::isScanDone() {
    if (!s_scanning) return false;
    int n = WiFi.scanComplete();
    return (n != WIFI_SCAN_RUNNING);
}

WifiStats WifiRadio::getResults() {
    s_scanning = false;
    WifiStats stats;

    int n = WiFi.scanComplete();
    if (n < 0) {
        WiFi.scanDelete();
        return stats;
    }

    stats.netCount = n;
    int totalRSSI = 0;

    for (int i = 0; i < n; i++) {
        int rssi = WiFi.RSSI(i);
        totalRSSI += rssi;
        if (rssi > -60) stats.strongCount++;
        if (WiFi.SSID(i).length() == 0) stats.hiddenCount++;
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) stats.openCount++;
        else stats.wpaCount++;
    }

    stats.avgRSSI = (n > 0) ? (totalRSSI / n) : -100;
    WiFi.scanDelete();
    return stats;
}
