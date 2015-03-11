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
