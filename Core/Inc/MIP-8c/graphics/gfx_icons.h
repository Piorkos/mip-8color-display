/*
 * gfx_icons.h
 *
 *  Created on: 28.10.2026
 *      Author: Dominik Piórkowski
 */


#ifndef GFX_ICONS_H
#define GFX_ICONS_H

#include <stdint.h>

typedef struct
{
    uint8_t width;
    uint8_t height;
    const uint8_t *data;
} gfx_icons_t;

/* Available icons */
extern const gfx_icons_t gfx_icon_envelope;

#endif /* GFX_ICONS_H */