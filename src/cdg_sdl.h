/*
 * Copyright 2017 - Tobias Olausson
 *
 * This file is part of libcdg.
 *
 * libcdg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libcdg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libcdg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CDG_SDL_H
#define CDG_SDL_H

#include "cdg.h"
#include <SDL.h>

#define CDG_USECS_PER_PACKET (1000000 / CDG_PACKETS_PER_SECOND)
#define CDG_SCREEN_WIDTH 300
#define CDG_SCREEN_HEIGHT 216
#define CDG_VIEW_WIDTH 294
#define CDG_VIEW_HEIGHT 204

typedef struct {
    CDG_RGB *CLUT[16];
    CDG_RGB *pixels[CDG_SCREEN_WIDTH][CDG_SCREEN_HEIGHT];
    SDL_Surface *surface;
} CDG_SDL_State;

// Entrypoint function for rendering
int cdg_sdl_render_packet(CDG_Data *packet, CDG_SDL_State *state);

int cdg_sdl_render_mem_preset(unsigned char color, CDG_SDL_State *state);
int cdg_sdl_render_border_preset(unsigned char color, CDG_SDL_State *state);

#endif // CDG_SDL_H
