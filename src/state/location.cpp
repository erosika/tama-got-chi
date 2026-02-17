#include "location.h"
#include "../hal/gps.h"
#include <Arduino.h>
#include <cmath>

// ==========================================================================
// Location -- Zone transitions from GPS geofence + NFC overrides
// ==========================================================================

static LocationZone s_zone = LOC_HOME;
static LocationZone s_override = LOC_UNKNOWN;
static unsigned long s_overrideExpiry = 0;
static unsigned long s_lastGeoCheck = 0;
static constexpr unsigned long GEO_CHECK_MS = 10000;
static constexpr unsigned long OVERRIDE_DURATION_MS = 4UL * 60 * 60 * 1000;

// Haversine distance in meters (approximate, good enough for geofencing)
static float distanceM(float lat1, float lng1, float lat2, float lng2) {
    float dLat = (lat2 - lat1) * 0.0174533f;
    float dLng = (lng2 - lng1) * 0.0174533f;
    float a = sinf(dLat / 2) * sinf(dLat / 2) +
              cosf(lat1 * 0.0174533f) * cosf(lat2 * 0.0174533f) *
              sinf(dLng / 2) * sinf(dLng / 2);
    return 6371000.0f * 2.0f * atan2f(sqrtf(a), sqrtf(1 - a));
}

void Location::init(const Settings&) {
    s_zone = LOC_HOME;
    s_override = LOC_UNKNOWN;
    s_overrideExpiry = 0;
}

void Location::tick(const Settings& settings) {
    unsigned long now = millis();

    // Check NFC override expiry
    if (s_overrideExpiry > 0 && now >= s_overrideExpiry) {
        s_override = LOC_UNKNOWN;
        s_overrideExpiry = 0;
    }

    // NFC override takes priority
    if (s_override != LOC_UNKNOWN) {
        s_zone = s_override;
        return;
    }

    // GPS geofence check
    if (now - s_lastGeoCheck >= GEO_CHECK_MS) {
        s_lastGeoCheck = now;

        if (GPS::hasFix()) {
            LocationZone matched = matchGeofence(GPS::lat(), GPS::lng(), settings);
            s_zone = matched;
        }
    }
}

LocationZone Location::current() {
    return s_zone;
}

void Location::setOverride(LocationZone zone) {
    s_override = zone;
    s_overrideExpiry = millis() + OVERRIDE_DURATION_MS;
}

void Location::clearOverride() {
    s_override = LOC_UNKNOWN;
    s_overrideExpiry = 0;
}

LocationZone Location::matchGeofence(float lat, float lng,
                                     const Settings& settings) {
    for (int i = 0; i < 4; i++) {
        const GeoProfile& gp = settings.geoProfiles[i];
        if (gp.radius <= 0) continue;
        float d = distanceM(lat, lng, gp.lat, gp.lng);
        if (d <= gp.radius) return gp.zone;
    }
    return LOC_UNKNOWN;
}

const char* Location::zoneName(LocationZone zone) {
    switch (zone) {
        case LOC_HOME:    return "HOME";
        case LOC_WORK:    return "WORK";
        case LOC_TRAVEL:  return "TRAVEL";
        case LOC_UNKNOWN: return "UNKNOWN";
    }
    return "?";
}
