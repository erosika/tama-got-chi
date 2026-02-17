#pragma once

// ==========================================================================
// Sprite extern declarations -- defined in sprite_data.c (C linkage)
// ==========================================================================

#ifdef __cplusplus
extern "C" {
#endif

// StoneGolem.h -- 115x110 pet frames
extern const unsigned short idle_1[];
extern const unsigned short idle_2[];
extern const unsigned short idle_3[];
extern const unsigned short idle_4[];

extern const unsigned short egg_hatch_1[];
extern const unsigned short egg_hatch_2[];
extern const unsigned short egg_hatch_3[];
extern const unsigned short egg_hatch_4[];
extern const unsigned short egg_hatch_5[];

extern const unsigned short dead_1[];
extern const unsigned short dead_2[];
extern const unsigned short dead_3[];

extern const uint16_t attack_0[];
extern const uint16_t attack_1[];
extern const uint16_t attack_2[];

// egg_hatch.h -- 115x110 egg idle frames
extern const unsigned short egg_hatch_11[];
extern const unsigned short egg_hatch_21[];
extern const unsigned short egg_hatch_31[];
extern const unsigned short egg_hatch_41[];
extern const unsigned short egg_hatch_51[];

// effect.h -- 100x95 hunger overlay + death
extern const unsigned short hunger1[];
extern const unsigned short hunger2[];
extern const unsigned short hunger3[];
extern const unsigned short hunger4[];
extern const unsigned short rip_ms[];

// background.h -- 240x222 backgrounds
extern const unsigned short backgroundImage[];
extern const unsigned short backgroundImage1[];
extern const unsigned short backgroundImage2[];

#ifdef __cplusplus
}
#endif
