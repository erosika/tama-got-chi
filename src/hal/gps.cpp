#include "gps.h"
#include <Arduino.h>

// ==========================================================================
// GPS -- NEO-6M UART with TinyGPSPlus
// ==========================================================================

#if FEATURE_GPS

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

static TinyGPSPlus s_gps;
static HardwareSerial s_serial(1);  // UART1
static bool s_sleeping = false;

void GPS::init() {
    s_serial.begin(9600, SERIAL_8N1, PIN_GPS_TX, PIN_GPS_RX);
    Serial.println("[gps] NEO-6M init");
}

void GPS::tick() {
    if (s_sleeping) return;
    while (s_serial.available()) {
        s_gps.encode(s_serial.read());
    }
}

bool GPS::hasFix() { return s_gps.location.isValid(); }
float GPS::lat() { return s_gps.location.lat(); }
float GPS::lng() { return s_gps.location.lng(); }
int GPS::satellites() { return s_gps.satellites.value(); }

void GPS::sleep() {
    s_sleeping = true;
    // Send PMTK standby command
    s_serial.println("$PMTK161,0*28");
}

void GPS::wake() {
    s_sleeping = false;
    // Any byte wakes NEO-6M from standby
    s_serial.write(0xFF);
}

#else

// -- Stubs when GPS disabled -----------------------------------------------
void GPS::init() {}
void GPS::tick() {}
bool GPS::hasFix() { return false; }
float GPS::lat() { return 0.0f; }
float GPS::lng() { return 0.0f; }
int GPS::satellites() { return 0; }
void GPS::sleep() {}
void GPS::wake() {}

#endif
