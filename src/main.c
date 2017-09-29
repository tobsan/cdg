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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <SDL/SDL.h>
#include "cdg.h"

int cdg_read_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        exit(2);
    }

    SubCode subcode;
    cdg cdg_state;

    // The SDL surface to write to
    SDL_Surface* screen = SDL_SetVideoMode(CDG_SCREEN_WIDTH, CDG_SCREEN_HEIGHT, 32, SDL_SWSURFACE);

    // While file has not ended
    int p = 0;
    while (fread(&subcode, sizeof(SubCode), 1, file)) {
        CDG_Packet packet = cdg_parse_packet(&subcode);
        if(cdg_process_packet(&packet, &cdg_state) != 0) {
            printf("Something went wrong\n");
            return 1;
        }
        // Free the packet
        cdg_packet_put(&packet);

        SDL_LockSurface(screen);
        // Set the pixels in the SDL surface
        for (int i = 0; i < CDG_SCREEN_WIDTH; i++) {
            for (int j = 0; j < CDG_SCREEN_HEIGHT; j++) {
                unsigned char color_index = cdg_state.pixels[i][j];
                CDG_RGB color = cdg_state.color_table[color_index];

                // R-G-B-A. 
                Uint32 sdl_color = (((Uint32)color.red) << 24)
                                 | (((Uint32)color.green) << 16)
                                 | (((Uint32)color.blue << 8));

                sdl_color *= 2;
                /*
                int offset = i * CDG_SCREEN_WIDTH + j;
                Uint32 target_pixel = screen->pixels + offset
                screen->pixels[offset] = 
                */ 
            }
        }
        SDL_UnlockSurface(screen);
        printf("Handled packet %i\n", p++);

        //Update the screen
        if( SDL_Flip( screen ) == -1 ) { 
            return 1;
        }

        // 10x speed
        // SDL_Delay(CDG_MSECS_PER_PACKET / 10);
    }

    // Sanity check and close the file
    if (!feof(file) || ferror(file)) {
        fprintf(stderr, "Reading of file failed\n");
        return 2;
    }
    fclose(file);
    // Done doing file stuff

    return 0;
}

int main(int argc, char **argv)
{
    // Open file.
    if(argc < 2) {
        printf("Usage: %s <cdg-file>\n", argv[0]);
        exit(1);
    }

    SDL_Init( SDL_INIT_EVERYTHING );

    char *filename = argv[1];
    cdg_read_file(filename);
    
    SDL_Quit();
    return 0;
}
