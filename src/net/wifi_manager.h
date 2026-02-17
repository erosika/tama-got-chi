#pragma once

// ==========================================================================
// WiFi Manager -- STA connect/reconnect, credential storage
// ==========================================================================

namespace WifiManager {

void init(const char* ssid, const char* pass);
void tick();        // Call periodically -- handles reconnect

bool isConnected();
int rssi();         // Current signal strength

}  // namespace WifiManager
