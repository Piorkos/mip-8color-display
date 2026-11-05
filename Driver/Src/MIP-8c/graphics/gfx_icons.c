/*
 * gfx_icons.c
 *
 *  Created on: 29.10.2026
 *      Author: Dominik Piórkowski
 */


#include "graphics/gfx_icons.h"

#define ENVELOPE_WIDTH   30
#define ENVELOPE_HEIGHT  20

/*
 * Envelope icon bitmap.
 *
 * Pixel format:
 * 0 = white
 * 1 = black
 *
 * One bit per pixel, packed MSB-first.
 * Each row is padded to a whole number of bytes; unused bits are ignored.
 */
static const uint8_t envelope_data
    [ENVELOPE_HEIGHT][(ENVELOPE_WIDTH + 7) / 8] = {
    {0b11111111, 0b11111111, 0b11111111, 0b11111100},
    {0b11111111, 0b11111111, 0b11111111, 0b11111100},
    {0b11111111, 0b11111111, 0b11111111, 0b11111100},
    {0b11111000, 0b00000000, 0b00000000, 0b01111100},
    {0b11110110, 0b00000000, 0b00000000, 0b01101110},
    {0b11100111, 0b00000000, 0b00000011, 0b11001110},
    {0b11100011, 0b11000000, 0b00001111, 0b00001110},
    {0b11100011, 0b10000000, 0b00011110, 0b00001110},
    {0b11100001, 0b11100000, 0b00011110, 0b00001110},
    {0b11100000, 0b11110000, 0b00111100, 0b00001110},
    {0b11100000, 0b01111111, 0b11111000, 0b00001110},
    {0b11100000, 0b00111111, 0b11110000, 0b00001110},
    {0b11100000, 0b00000000, 0b00000000, 0b00001110},
    {0b11100000, 0b00000000, 0b00000000, 0b00001110},
    {0b11100000, 0b00000000, 0b00000000, 0b00001110},
    {0b11100000, 0b00000000, 0b00000000, 0b00001110},
    {0b11100000, 0b00000000, 0b00000000, 0b00001110},
    {0b11111111, 0b11111111, 0b11111111, 0b11111100},
    {0b11111111, 0b11111111, 0b11111111, 0b11111100},
    {0b11111111, 0b11111111, 0b11111111, 0b11111100}
};



/* Public icon descriptor */
const gfx_icons_t gfx_icon_envelope = {
    .width  = ENVELOPE_WIDTH,
    .height = ENVELOPE_HEIGHT,
    .data   = &envelope_data[0][0]
};
