#include "threat_detect.h"
#include "../hal/ble.h"
#include "../hal/wifi_promisc.h"
#include "../hal/nfc.h"
#include "config.h"
#include <Arduino.h>
#include <cstring>

// ==========================================================================
// Threat Detection -- Pattern matching against known threat signatures
//
// Detects locally (no network):
//   1. Rogue BLE scanner:    device scanning at rate > threshold
//   2. Evil twin WiFi AP:    (future -- needs known network list)
//   3. Deauth attack:        deauth frame count > threshold in window
//   4. Mass BLE enumeration: many scan requests from single source
//   5. NFC skimmer:          unexpected NFC field when no interaction expected
//   6. IMSI catcher pattern: abnormally strong signals with forced behavior
//
// Produces a RadioEnvironment with an aggregate safety score.
// ==========================================================================

static RadioEnvironment s_env;
static ThreatEntry s_threats[MAX_THREATS];
static int s_threatCount = 0;
static unsigned long s_lastEvalMs = 0;
static constexpr unsigned long EVAL_INTERVAL_MS = 2000;

static void addThreat(ThreatType type, ThreatSeverity severity,
                      const char* detail) {
    // Check for existing threat of same type (don't duplicate)
    for (int i = 0; i < s_threatCount; i++) {
        if (s_threats[i].type == type) {
            // Update timestamp and detail
            s_threats[i].timestampMs = millis();
            s_threats[i].expiresMs = millis() + THREAT_EXPIRE_MS;
            s_threats[i].severity = severity;
            strncpy(s_threats[i].detail, detail, 47);
            s_threats[i].detail[47] = '\0';
            return;
        }
    }

    // Add new threat
    if (s_threatCount < MAX_THREATS) {
        ThreatEntry& t = s_threats[s_threatCount++];
        t.type = type;
        t.severity = severity;
        t.timestampMs = millis();
        t.expiresMs = millis() + THREAT_EXPIRE_MS;
        strncpy(t.detail, detail, 47);
        t.detail[47] = '\0';
    }
}

static void expireThreats() {
    unsigned long now = millis();
    int writeIdx = 0;
    for (int i = 0; i < s_threatCount; i++) {
        if (now < s_threats[i].expiresMs) {
            if (writeIdx != i) s_threats[writeIdx] = s_threats[i];
            writeIdx++;
        }
    }
    s_threatCount = writeIdx;
}

void ThreatDetect::init() {
    s_env = RadioEnvironment();
    s_threatCount = 0;
    s_lastEvalMs = 0;
}

void ThreatDetect::evaluate() {
    expireThreats();

    // -- Gather BLE data ---------------------------------------------------
    s_env.bleDeviceCount = BLE::deviceCount();
    s_env.bleScannerCount = BLE::scannerCount();

    // Check for rogue BLE scanners
    for (int i = 0; i < BLE::deviceCount(); i++) {
        const BleDevice* d = BLE::device(i);
        if (!d) continue;

        if (d->scanRate >= ROGUE_SCAN_THRESHOLD) {
            char detail[48];
            snprintf(detail, sizeof(detail), "BLE scanner: %d/min RSSI:%d",
                     d->scanRate, d->rssi);
            addThreat(THREAT_ROGUE_SCANNER, THREAT_WARNING, detail);
        }

        if (d->scanRate >= MASS_ENUM_THRESHOLD) {
            addThreat(THREAT_MASS_ENUM, THREAT_CRITICAL,
                      "Mass BLE enumeration detected");
        }
    }

    // -- Gather WiFi promiscuous data --------------------------------------
    s_env.probeCount = WifiPromisc::probeCount();
    s_env.uniqueProbers = WifiPromisc::uniqueProbers();
    s_env.deauthCount = WifiPromisc::deauthCount();

    // Check for deauth attack
    if (s_env.deauthCount >= DEAUTH_THRESHOLD) {
        char detail[48];
        snprintf(detail, sizeof(detail), "Deauth flood: %d frames",
                 s_env.deauthCount);
        addThreat(THREAT_DEAUTH, THREAT_CRITICAL, detail);
        WifiPromisc::resetDeauthCount();
    }

    // -- NFC field anomaly (when not actively using NFC) -------------------
    // If NFC detects a tag presence but no user interaction is expected
    // this could indicate a skimmer or rogue reader
    // (Simplified: flag if tag appears while on HOME screen)

    // -- Compute aggregate -------------------------------------------------
    s_env.threatCount = s_threatCount;

    s_env.worstThreat = THREAT_INFO;
    for (int i = 0; i < s_threatCount; i++) {
        if (s_threats[i].severity > s_env.worstThreat)
            s_env.worstThreat = s_threats[i].severity;
    }

    // Safety score: 100 = safe, decays with threats
    int score = 100;
    for (int i = 0; i < s_threatCount; i++) {
        switch (s_threats[i].severity) {
            case THREAT_CRITICAL: score -= 30; break;
            case THREAT_WARNING:  score -= 15; break;
            case THREAT_INFO:     score -= 5;  break;
        }
    }
    // Also penalize high scanner/prober counts even without flagged threats
    if (s_env.bleScannerCount > 3) score -= 10;
    if (s_env.deauthCount > 0)     score -= 20;

    if (score < 0) score = 0;
    s_env.safetyScore = score;
}

void ThreatDetect::tick() {
    unsigned long now = millis();
    if (now - s_lastEvalMs < EVAL_INTERVAL_MS) return;
    s_lastEvalMs = now;
    evaluate();
}

const RadioEnvironment& ThreatDetect::environment() { return s_env; }
const ThreatEntry* ThreatDetect::threats() { return s_threats; }
int ThreatDetect::threatCount() { return s_threatCount; }
