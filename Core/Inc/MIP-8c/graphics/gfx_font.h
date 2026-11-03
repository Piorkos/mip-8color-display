/*
 * font.h
 *
 *  Created on: 25.10.2026
 *      Author: Dominik Piórkowski
 */

#ifndef GFX_FONTS_PUES_H
#define GFX_FONTS_PUES_H

#include <stdint.h>


typedef struct
{
    uint8_t index_of_first_char;
    uint8_t space_in_pixels;
    uint8_t characters_separator_in_pixels;
    uint8_t letter_bin_width_in_px;
    uint8_t letter_bin_height_in_px;
    const uint8_t *letters_width;
    const uint8_t *letters_height;
    const uint8_t *letters_bin;
} gfx_fonts_t;


extern const gfx_fonts_t gfx_font_bitmap_robo_cond_s;
extern const gfx_fonts_t gfx_font_bitmap_robo_cond_m;
extern const gfx_fonts_t gfx_font_bitmap_robo_cond_l;


#endif /* GFX_FONTS_PUES_H */