#include "cosmania_client.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// ==========================================================================
// Cosmania Client -- Polls /status endpoint, parses SystemStatus JSON
// ==========================================================================

static CosmaniaStatus s_status;
static char s_url[128] = {0};
static unsigned long s_lastPoll = 0;
static unsigned long s_pollInterval = COSMANIA_POLL_MS;

// -- Budget tier string -> enum -------------------------------------------
static BudgetTier parseTier(const char* str) {
    if (!str) return TIER_UNKNOWN;
    if (strcmp(str, "GREEN")  == 0) return TIER_GREEN;
    if (strcmp(str, "YELLOW") == 0) return TIER_YELLOW;
    if (strcmp(str, "RED")    == 0) return TIER_RED;
    if (strcmp(str, "BLACK")  == 0) return TIER_BLACK;
    return TIER_UNKNOWN;
}

// -- Parse JSON response --------------------------------------------------
static bool parseResponse(const String& body) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) return false;

    s_status.connected     = true;
    s_status.lastPollMs    = millis();
    s_status.budgetTier    = parseTier(doc["budgetTier"] | "UNKNOWN");
    s_status.totalDailyBudget = doc["totalDailyBudget"] | 0.0f;
    s_status.totalDailySpend  = doc["totalDailySpend"]  | 0.0f;

    // Parse agents array
    JsonArray agents = doc["agents"].as<JsonArray>();
    s_status.agentCount   = 0;
    s_status.overdueCount = 0;
    s_status.activeCount  = 0;
    s_status.errorCount   = 0;

    for (JsonObject agent : agents) {
        if (s_status.agentCount >= 7) break;
        AgentInfo& info = s_status.agents[s_status.agentCount];

        const char* name = agent["name"] | "";
        strncpy(info.name, name, sizeof(info.name) - 1);
        info.name[sizeof(info.name) - 1] = '\0';

        info.overdue     = agent["overdue"]      | false;
        info.overBudget  = agent["overBudget"]    | false;
        info.todayCostUsd = agent["todayCostUsd"] | 0.0f;
        info.todayRuns   = agent["todayRuns"]     | 0;
        info.minutesSince = agent["minutesSinceLastRun"] | -1;

        if (info.overdue) s_status.overdueCount++;
        if (info.todayRuns > 0) s_status.activeCount++;

        s_status.agentCount++;
    }

    // Count recent errors
    JsonArray errors = doc["recentErrors"].as<JsonArray>();
    s_status.errorCount = 0;
    for (JsonVariant v : errors) {
        s_status.errorCount++;
        (void)v;
    }

    return true;
}

void CosmaniaClient::init(const char* baseUrl) {
    if (!baseUrl || baseUrl[0] == '\0') return;
    snprintf(s_url, sizeof(s_url), "%s/status", baseUrl);
}

void CosmaniaClient::tick() {
    if (s_url[0] == '\0') return;
    if (WiFi.status() != WL_CONNECTED) return;

    unsigned long now = millis();
    if (now - s_lastPoll < s_pollInterval) return;
    s_lastPoll = now;

    HTTPClient http;
    http.setTimeout(5000);
    http.begin(s_url);

    int code = http.GET();
    if (code == 200) {
        String body = http.getString();
        parseResponse(body);
    } else {
        s_status.connected = false;
    }

    http.end();
}

void CosmaniaClient::pollNow() {
    s_lastPoll = 0;     // Force next tick to poll
}

bool CosmaniaClient::isConnected() {
    return s_status.connected;
}

const CosmaniaStatus& CosmaniaClient::getStatus() {
    return s_status;
}
