#pragma once

// ==========================================================================
// Sound HAL -- Non-blocking retro sequencer
// ==========================================================================

namespace Sound {

void init();
void tick();    // Call every loop iteration (advances sequencer)

void click();
void goodFeed();
void badFeed();
void discover();
void restStart();
void restEnd();
void hatch();

void setEnabled(bool on);
bool isEnabled();

}  // namespace Sound
