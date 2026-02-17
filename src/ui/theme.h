#pragma once
#include <cstdint>

class TFT_eSprite;

// ==========================================================================
// Theme -- RGB565 palette + drawing primitives
// Maps eri's CSS variable palette to 16-bit display color space
// ==========================================================================

namespace Theme {

// -- RGB565 palette (derived from eri's CSS custom properties) -------------
constexpr uint16_t BG          = 0x0841;   // #0b0e14
constexpr uint16_t BG_ELEVATED = 0x10A2;   // #111820
constexpr uint16_t FG          = 0xCE59;   // #c9d1d9
constexpr uint16_t FG_MUTED    = 0x7BCF;   // #7a8899
constexpr uint16_t FG_BRIGHT   = 0xE73C;   // #e6edf3
constexpr uint16_t ACCENT      = 0x7DDF;   // #7eb8f6
constexpr uint16_t GREEN       = 0x7F54;   // #7ee6a8
constexpr uint16_t ORANGE      = 0xE54B;   // #e6a855
constexpr uint16_t RED         = 0xD8A3;   // #d94040
constexpr uint16_t PURPLE      = 0xBC7F;   // #bc8cff
constexpr uint16_t BORDER      = 0x2945;   // #2a2d35

// -- Drawing primitives ----------------------------------------------------

// Horizontal rule: 1px line across full width
void drawRule(TFT_eSprite& fb, int y, uint16_t color = BORDER);

// Header bar: text on BG_ELEVATED strip with ACCENT underline
void drawHeader(TFT_eSprite& fb, const char* title);

// Stat bar: sharp rectangle, 1px border, colored fill
void drawBar(TFT_eSprite& fb, int x, int y, int w, int h,
             int value, uint16_t color);

// Centered text using GLCD font (Font 1, 8px monospace)
void drawCenteredGLCD(TFT_eSprite& fb, int y, const char* text,
                      uint16_t color = FG);

// Centered text using Font 2 (16px proportional)
void drawCenteredFont2(TFT_eSprite& fb, int y, const char* text,
                       uint16_t color = FG);

// Menu cursor line: "> LABEL" in GLCD
void drawMenuItem(TFT_eSprite& fb, int y, const char* label,
                  bool selected, uint16_t color = FG);

}  // namespace Theme
