#pragma once

// ==========================================================================
// Power -- Sleep modes, backlight control, battery ADC
// ==========================================================================

namespace Power {

void init();
void tick();

// Display sleep (backlight off, reduce polling)
void sleepDisplay();
void wakeDisplay();
bool isDisplaySleeping();

// Battery
float batteryVoltage();
int batteryPercent();  // 0-100

// Wake sources
void enableWakeOnButton();
void enableWakeOnNFC();

}  // namespace Power
