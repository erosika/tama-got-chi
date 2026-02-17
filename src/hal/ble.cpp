#include "ble.h"
#include <Arduino.h>

// ==========================================================================
// BLE -- ESP32-S3 passive BLE scanning
// Detects nearby BLE devices, identifies scanners, collects metadata.
// MAC addresses are hashed -- raw MACs never stored.
// ==========================================================================

#if FEATURE_SOVEREIGNTY

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

static BleDevice s_devices[MAX_BLE_DEVICES];
static int s_deviceCount  = 0;
static int s_scannerCount = 0;
static BLEScan* s_scanner = nullptr;
static bool s_scanning    = false;
static unsigned long s_lastScanMs = 0;

// Simple hash of 6-byte MAC to 4-byte fingerprint
static void hashAddr(const uint8_t* addr, uint8_t* out) {
    // FNV-1a 32-bit
    uint32_t h = 2166136261u;
    for (int i = 0; i < 6; i++) {
        h ^= addr[i];
        h *= 16777619u;
    }
    out[0] = (h >> 24) & 0xFF;
    out[1] = (h >> 16) & 0xFF;
    out[2] = (h >> 8) & 0xFF;
    out[3] = h & 0xFF;
}

static bool hashMatch(const uint8_t* a, const uint8_t* b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static int findDevice(const uint8_t* hash) {
    for (int i = 0; i < s_deviceCount; i++) {
        if (hashMatch(s_devices[i].addrHash, hash)) return i;
    }
    return -1;
}

static uint8_t classifyDevice(BLEAdvertisedDevice& dev) {
    if (dev.haveAppearance()) {
        uint16_t app = dev.getAppearance();
        if (app >= 0x0040 && app <= 0x007F) return 1; // phone
        if (app >= 0x0080 && app <= 0x00BF) return 2; // computer
        if (app >= 0x00C0 && app <= 0x00FF) return 3; // wearable
    }
    if (dev.haveServiceUUID()) return 1; // likely phone/wearable
    return 0;
}

class ScanCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        uint8_t hash[4];
        const uint8_t* rawAddr = advertisedDevice.getAddress().getNative();
        hashAddr(rawAddr, hash);

        int idx = findDevice(hash);
        unsigned long now = millis();

        if (idx >= 0) {
            // Update existing device
            BleDevice& d = s_devices[idx];
            d.rssi = advertisedDevice.getRSSI();

            // Track scan rate: if seen again within 10s, increment
            unsigned long delta = now - d.lastSeenMs;
            if (delta < 10000) {
                d.scanRate = min(255, d.scanRate + 1);
            }
            d.lastSeenMs = now;
        } else {
            // New device
            if (s_deviceCount < MAX_BLE_DEVICES) {
                idx = s_deviceCount++;
            } else {
                // Overwrite oldest
                uint32_t oldest = UINT32_MAX;
                idx = 0;
                for (int i = 0; i < MAX_BLE_DEVICES; i++) {
                    if (s_devices[i].lastSeenMs < oldest) {
                        oldest = s_devices[i].lastSeenMs;
                        idx = i;
                    }
                }
            }

            BleDevice& d = s_devices[idx];
            memcpy(d.addrHash, hash, 4);
            d.rssi = advertisedDevice.getRSSI();
            d.lastSeenMs = now;
            d.scanRate = 0;
            d.isScanner = false;
            d.deviceType = classifyDevice(advertisedDevice);

            if (advertisedDevice.haveName()) {
                strncpy(d.name, advertisedDevice.getName().c_str(), 19);
                d.name[19] = '\0';
            } else {
                d.name[0] = '\0';
            }
        }
    }
};

static ScanCallbacks s_callbacks;

void BLE::init() {
    BLEDevice::init("");
    s_scanner = BLEDevice::getScan();
    s_scanner->setAdvertisedDeviceCallbacks(&s_callbacks, true);
    s_scanner->setActiveScan(false);  // passive -- don't send scan requests
    s_scanner->setInterval(100);
    s_scanner->setWindow(99);         // near-continuous window
    Serial.println("[ble] scanner ready");
}

void BLE::tick() {
    unsigned long now = millis();

    if (!s_scanning && (now - s_lastScanMs >= BLE_SCAN_INTERVAL_MS)) {
        s_scanner->start(BLE_SCAN_DURATION_S, false);
        s_scanning = true;
        s_lastScanMs = now;
    }

    // Check if scan window ended
    if (s_scanning) {
        BLEScanResults results = s_scanner->getResults();
        if (results.getCount() > 0 || (now - s_lastScanMs > (BLE_SCAN_DURATION_S * 1000 + 500))) {
            s_scanning = false;
            s_scanner->clearResults();

            // Recount scanners
            s_scannerCount = 0;
            for (int i = 0; i < s_deviceCount; i++) {
                if (s_devices[i].scanRate >= ROGUE_SCAN_THRESHOLD) {
                    s_devices[i].isScanner = true;
                    s_scannerCount++;
                }
            }
        }
    }
}

int BLE::deviceCount() { return s_deviceCount; }
int BLE::scannerCount() { return s_scannerCount; }
const BleDevice* BLE::devices() { return s_devices; }
const BleDevice* BLE::device(int idx) {
    if (idx < 0 || idx >= s_deviceCount) return nullptr;
    return &s_devices[idx];
}
void BLE::clearDevices() {
    s_deviceCount = 0;
    s_scannerCount = 0;
}

#else

// -- Stubs when sovereignty disabled ---------------------------------------
void BLE::init() {}
void BLE::tick() {}
int BLE::deviceCount() { return 0; }
int BLE::scannerCount() { return 0; }
const BleDevice* BLE::devices() { return nullptr; }
const BleDevice* BLE::device(int) { return nullptr; }
void BLE::clearDevices() {}

#endif
