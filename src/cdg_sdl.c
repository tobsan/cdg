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

#include "cdg_sdl.h"

int cdg_sdl_render_packet(CDG_Data *packet, CDG_SDL_State *state)
{
    switch(packet->packet_type) {
        case MEMORY_PRESET:
            return cdg_sdl_render_mem_preset(packet->mem_preset, state);
            break;
    }
}

int cdg_sdl_render_mem_preset(unsigned char color, CDG_SDL_State *state)
{
    unsigned char red = state->CLUT[color]->red;
    unsigned char green = state->CLUT[color]->green;
    unsigned char blue = state->CLUT[color]->blue;

    // Set internal state
    int i, j;
    for (int i = 0; i < CDG_SCREEN_WIDTH; i++) {
        for (int j = 0; i < CDG_SCREEN_HEIGHT; j++) {
            state->pixels[i][j]->red = red;
            state->pixels[i][j]->green = green;
            state->pixels[i][j]->blue = blue;
        }
    }
    
    // Then fill the SDL surface
    Uint32 color = SDL_MapRGB(state->surface->format, red, green, blue);
    SDL_FillRect(state->surface, NULL, color); // NULL means fill all of it
    return SDL_Flip(state->surface);
}

