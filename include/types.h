#pragma once
#include <cstdint>

// ==========================================================================
// TamaFi -- Single source of truth for all enums and structs
// ==========================================================================

// -- Screens ---------------------------------------------------------------
enum Screen : uint8_t {
    SCREEN_BOOT,
    SCREEN_HATCH,
    SCREEN_HOME,
    SCREEN_GLANCE,          // NFC wake: priority items + mood
    SCREEN_DASHBOARD,       // All 7 agents compact
    SCREEN_REVIEW,          // Pending items, approve/dismiss
    SCREEN_MENU,
    SCREEN_STATUS,          // Pet status detail
    SCREEN_AGENTS,          // Individual agent drill-down
    SCREEN_LOCATION,        // Location zone display
    SCREEN_WIFI_SCAN,       // WiFi scan results
    SCREEN_SETTINGS,
    SCREEN_SYSINFO,
    SCREEN_GAMEOVER,
    SCREEN_RADIO,           // Radio environment + threat dashboard
};

// -- Activities ------------------------------------------------------------
enum Activity : uint8_t {
    ACT_NONE,
    ACT_HUNT,
    ACT_DISCOVER,
    ACT_REST,
};

// -- Evolution stages (milestone-based, never regress) ---------------------
enum EvolutionStage : uint8_t {
    STAGE_EGG,              // Framework exists, no agents running
    STAGE_LARVA,            // sentinel todayRuns > 0
    STAGE_NYMPH,            // dreamer AND coder todayRuns > 0
    STAGE_JUVENILE,         // activeAgentCount >= 7, ageDays >= 7
    STAGE_ADULT,            // budgetTier GREEN for 7 consecutive days
    STAGE_ELDER,            // ageDays >= 180, all agents uptimePct > 80%
};

// -- Moods (priority order, highest wins) ----------------------------------
enum Mood : uint8_t {
    MOOD_SICK,              // budgetTier RED or BLACK
    MOOD_ANGRY,             // errorCount >= 3 or agent drift
    MOOD_ANXIOUS,           // budgetTier YELLOW or overdueCount >= 2
    MOOD_WORKING,           // any agent minutesSinceLastRun < 5
    MOOD_HUNGRY,            // pendingReviews > 5 (future)
    MOOD_SLEEPY,            // activeAgentCount == 0, no interaction 1h
    MOOD_HAPPY,             // GREEN, no overdue, interacted < 30min
    MOOD_CONTENT,           // GREEN, no overdue (default good)
};

// -- Rest phases -----------------------------------------------------------
enum RestPhase : uint8_t {
    REST_NONE,
    REST_ENTER,
    REST_DEEP,
    REST_WAKE,
};

// -- Location zones --------------------------------------------------------
enum LocationZone : uint8_t {
    LOC_HOME,               // Full display, 30s poll, GPS sleeps
    LOC_WORK,               // Simplified display, 60s poll, GPS sleeps
    LOC_TRAVEL,             // Prominent alerts, 10s poll, GPS active
    LOC_UNKNOWN,            // Lockdown option prominent
};

// -- Budget tier (from Cosmania /status) -----------------------------------
enum BudgetTier : uint8_t {
    TIER_GREEN,
    TIER_YELLOW,
    TIER_RED,
    TIER_BLACK,
    TIER_UNKNOWN,           // No Cosmania connection
};

// -- Pet state -------------------------------------------------------------
struct PetState {
    int hunger      = 70;
    int happiness   = 70;
    int health      = 70;

    uint32_t ageMinutes = 0;
    uint32_t ageHours   = 0;
    uint32_t ageDays    = 0;

    EvolutionStage stage = STAGE_EGG;
    Mood mood            = MOOD_CONTENT;
    bool alive           = true;
    bool hatched         = false;
};

// -- WiFi scan results -----------------------------------------------------
struct WifiStats {
    int netCount    = 0;
    int strongCount = 0;
    int hiddenCount = 0;
    int avgRSSI     = -100;
    int openCount   = 0;
    int wpaCount    = 0;
};

// -- Agent info (from Cosmania /status JSON) -------------------------------
struct AgentInfo {
    char name[16]       = {0};
    bool overdue        = false;
    bool overBudget     = false;
    float todayCostUsd  = 0.0f;
    int todayRuns       = 0;
    int minutesSince    = -1;   // -1 = never run
};

// -- Cosmania system status ------------------------------------------------
struct CosmaniaStatus {
    bool connected          = false;
    uint32_t lastPollMs     = 0;
    BudgetTier budgetTier   = TIER_UNKNOWN;
    float totalDailyBudget  = 0.0f;
    float totalDailySpend   = 0.0f;
    AgentInfo agents[7]     = {};
    uint8_t agentCount      = 0;
    uint8_t errorCount      = 0;
    uint8_t overdueCount    = 0;
    uint8_t activeCount     = 0;    // agents with todayRuns > 0
    uint8_t greenDaysStreak = 0;    // consecutive GREEN days
};

// -- Location geofence profile ---------------------------------------------
struct GeoProfile {
    float lat     = 0.0f;
    float lng     = 0.0f;
    float radius  = 100.0f;    // meters
    LocationZone zone = LOC_HOME;
};

// -- NFC tag record --------------------------------------------------------
struct NfcTag {
    uint8_t uid[7]    = {0};
    uint8_t uidLen    = 0;
    LocationZone zone = LOC_UNKNOWN;
};

// -- Sovereignty: threat detection -----------------------------------------
enum ThreatSeverity : uint8_t {
    THREAT_INFO,
    THREAT_WARNING,
    THREAT_CRITICAL,
};

enum ThreatType : uint8_t {
    THREAT_NONE,
    THREAT_ROGUE_SCANNER,   // Device scanning at abnormal rate
    THREAT_EVIL_TWIN,       // AP with known SSID, wrong BSSID
    THREAT_DEAUTH,          // WiFi deauthentication flood
    THREAT_MASS_ENUM,       // Many scan requests from single source
    THREAT_NFC_SKIMMER,     // Unexpected NFC field at unusual power
    THREAT_IMSI_PATTERN,    // Signal pattern consistent with IMSI catcher
};

struct ThreatEntry {
    ThreatType type         = THREAT_NONE;
    ThreatSeverity severity = THREAT_INFO;
    uint32_t timestampMs    = 0;
    uint32_t expiresMs      = 0;
    char detail[48]         = {0};
};

// -- Sovereignty: BLE device record ----------------------------------------
struct BleDevice {
    uint8_t addrHash[4]     = {0};  // truncated hash (not raw MAC)
    int8_t rssi             = -127;
    uint8_t deviceType      = 0;    // 0=unknown 1=phone 2=laptop 3=wearable 4=beacon
    bool isScanner          = false;
    uint8_t scanRate         = 0;    // observed scans/min from this device
    uint32_t lastSeenMs     = 0;
    char name[20]           = {0};
};

// -- Sovereignty: WiFi probe request record --------------------------------
struct ProbeRequest {
    uint8_t srcHash[4]      = {0};
    char ssid[33]           = {0};
    int8_t rssi             = -127;
    uint32_t timestampMs    = 0;
};

// -- Sovereignty: aggregated radio environment -----------------------------
struct RadioEnvironment {
    int bleDeviceCount      = 0;
    int bleScannerCount     = 0;
    int probeCount          = 0;
    int uniqueProbers       = 0;
    int deauthCount         = 0;
    int threatCount         = 0;
    ThreatSeverity worstThreat = THREAT_INFO;
    uint8_t safetyScore     = 100;  // 0-100, 100 = safe
};

// -- Persisted settings ----------------------------------------------------
struct Settings {
    bool soundEnabled       = true;
    bool neoPixelsEnabled   = true;
    uint8_t tftBrightness   = 1;    // 0=low, 1=mid, 2=high
    uint8_t ledBrightness   = 1;
    bool autoSleep          = true;
    uint16_t autoSaveMs     = 30000;
    uint16_t pollIntervalMs = 30000; // Cosmania poll
    char cosmaniaUrl[64]    = {0};
    char wifiSsid[33]       = {0};
    char wifiPass[65]       = {0};
    GeoProfile geoProfiles[4] = {};
    NfcTag nfcTags[8]       = {};
};
