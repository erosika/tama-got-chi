#include "wifi_promisc.h"
#include <Arduino.h>

// ==========================================================================
// WiFi Promiscuous -- Captures raw 802.11 management frames
//
// Detects:
//   - Probe requests: reveals saved SSIDs of nearby devices
//   - Deauth frames: classic WiFi attack vector
//
// Runs alongside STA mode on the connected channel.
// Channel hopping available when disconnected.
// ==========================================================================

#if FEATURE_SOVEREIGNTY

#include <esp_wifi.h>
#include <esp_wifi_types.h>

// 802.11 management frame subtypes (in frame control byte 0)
static constexpr uint8_t FC_PROBE_REQ  = 0x40;
static constexpr uint8_t FC_DEAUTH     = 0xC0;

// Ring buffer for probe requests
static ProbeRequest s_probes[MAX_PROBE_REQUESTS];
static volatile int s_probeWriteIdx = 0;
static int s_probeCount = 0;

// Deauth counter
static volatile int s_deauthCount = 0;

// State
static bool s_enabled     = false;
static bool s_channelHop  = false;
static uint8_t s_channel  = 1;
static unsigned long s_lastHopMs = 0;

// FNV-1a hash for MAC address (same as BLE module)
static void hashMAC(const uint8_t* mac, uint8_t* out) {
    uint32_t h = 2166136261u;
    for (int i = 0; i < 6; i++) {
        h ^= mac[i];
        h *= 16777619u;
    }
    out[0] = (h >> 24) & 0xFF;
    out[1] = (h >> 16) & 0xFF;
    out[2] = (h >> 8) & 0xFF;
    out[3] = h & 0xFF;
}

// Promiscuous mode callback -- runs in WiFi task, must be fast
static void IRAM_ATTR promisc_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;

    const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* frame = pkt->payload;
    uint8_t fc0 = frame[0];

    if (fc0 == FC_DEAUTH) {
        s_deauthCount++;
        return;
    }

    if (fc0 == FC_PROBE_REQ) {
        // Source MAC is at bytes 10-15
        const uint8_t* srcMAC = &frame[10];

        // Write to ring buffer (single writer, safe without lock)
        int idx = s_probeWriteIdx % MAX_PROBE_REQUESTS;
        ProbeRequest& pr = s_probes[idx];

        hashMAC(srcMAC, pr.srcHash);
        pr.rssi = pkt->rx_ctrl.rssi;
        pr.timestampMs = millis();

        // SSID tagged parameter: frame body starts at byte 24
        // Tag 0 = SSID, byte 24 = tag number, byte 25 = length
        pr.ssid[0] = '\0';
        if (pkt->rx_ctrl.sig_len > 26) {
            uint8_t tag = frame[24];
            uint8_t len = frame[25];
            if (tag == 0 && len > 0 && len < 33 && (26 + len) <= pkt->rx_ctrl.sig_len) {
                memcpy(pr.ssid, &frame[26], len);
                pr.ssid[len] = '\0';
            }
        }

        s_probeWriteIdx++;
    }
}

void WifiPromisc::init() {
    memset(s_probes, 0, sizeof(s_probes));
    s_probeWriteIdx = 0;
    s_probeCount = 0;
    s_deauthCount = 0;
    s_enabled = false;
    Serial.println("[promisc] ready");
}

void WifiPromisc::tick() {
    if (!s_enabled) return;

    // Update probe count from write index
    s_probeCount = min((int)s_probeWriteIdx, MAX_PROBE_REQUESTS);

    // Channel hopping
    if (s_channelHop) {
        unsigned long now = millis();
        if (now - s_lastHopMs >= PROMISC_CHANNEL_HOP_MS) {
            s_lastHopMs = now;
            s_channel = (s_channel % 13) + 1;  // channels 1-13
            esp_wifi_set_channel(s_channel, WIFI_SECOND_CHAN_NONE);
        }
    }
}

void WifiPromisc::enable() {
    if (s_enabled) return;
    esp_wifi_set_promiscuous_rx_cb(promisc_cb);
    esp_wifi_set_promiscuous(true);
    s_enabled = true;
    Serial.println("[promisc] enabled");
}

void WifiPromisc::disable() {
    if (!s_enabled) return;
    esp_wifi_set_promiscuous(false);
    s_enabled = false;
    Serial.println("[promisc] disabled");
}

bool WifiPromisc::isEnabled() { return s_enabled; }

int WifiPromisc::probeCount() { return s_probeCount; }

int WifiPromisc::uniqueProbers() {
    // Count unique source hashes in the buffer
    int count = 0;
    uint8_t seen[MAX_PROBE_REQUESTS][4];
    int seenCount = 0;

    for (int i = 0; i < s_probeCount; i++) {
        bool found = false;
        for (int j = 0; j < seenCount; j++) {
            if (memcmp(s_probes[i].srcHash, seen[j], 4) == 0) {
                found = true;
                break;
            }
        }
        if (!found && seenCount < MAX_PROBE_REQUESTS) {
            memcpy(seen[seenCount], s_probes[i].srcHash, 4);
            seenCount++;
            count++;
        }
    }
    return count;
}

const ProbeRequest* WifiPromisc::probes() { return s_probes; }

int WifiPromisc::deauthCount() { return s_deauthCount; }
void WifiPromisc::resetDeauthCount() { s_deauthCount = 0; }

void WifiPromisc::setChannelHopping(bool enabled) {
    s_channelHop = enabled;
    if (enabled) s_channel = 1;
}

#else

// -- Stubs when sovereignty disabled ---------------------------------------
void WifiPromisc::init() {}
void WifiPromisc::tick() {}
void WifiPromisc::enable() {}
void WifiPromisc::disable() {}
bool WifiPromisc::isEnabled() { return false; }
int WifiPromisc::probeCount() { return 0; }
int WifiPromisc::uniqueProbers() { return 0; }
const ProbeRequest* WifiPromisc::probes() { return nullptr; }
int WifiPromisc::deauthCount() { return 0; }
void WifiPromisc::resetDeauthCount() {}
void WifiPromisc::setChannelHopping(bool) {}

#endif
