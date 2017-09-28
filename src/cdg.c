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

#include "cdg.h"

#include <string.h> // memcpy
#include <stdlib.h> // malloc, calloc
#include <stdio.h> // printf

int cdg_process_packet(CDG_Packet *packet, cdg *cdg_state)
{
    switch(packet->type) {
        case EMPTY:
            printf("Packet is empty\n");
            break;

        case MEMORY_PRESET: {
            unsigned char *color = (unsigned char *)packet->data;
            for (int i = 0; i < CDG_SCREEN_WIDTH; i++) {
                for (int j = 0; j < CDG_SCREEN_HEIGHT; j++) {
                    cdg_state->pixels[i][j] = *color;
                }
            }
            break;
        }

        case BORDER_PRESET: {
            unsigned char *color = (unsigned char *)packet->data;

            int border_width = (CDG_SCREEN_WIDTH - CDG_VIEW_WIDTH) / 2;
            int border_height = (CDG_SCREEN_HEIGHT - CDG_VIEW_HEIGHT) / 2;

            // Left and right borders
            for (int i = 0; i < border_width; i++) {
                for (int j = 0; j < CDG_SCREEN_HEIGHT; j++) {
                    cdg_state->pixels[i][j] = *color;
                    cdg_state->pixels[CDG_SCREEN_WIDTH - i - 1][j] = *color;
                }
            }

            // Top and bottom borders
            for (int i = 0; i < CDG_SCREEN_WIDTH; i++) {
                for (int j = 0; j < border_height; j++) {
                    cdg_state->pixels[i][j] = *color;
                    cdg_state->pixels[i][CDG_SCREEN_HEIGHT - j - 1] = *color;
                }
            }

            break;
        }

        case TILE_BLOCK:
        case TILE_BLOCK_XOR: {
            CDG_Tile *tile = (CDG_Tile *)packet->data;
            unsigned int start_row_px = tile->row * 12;
            unsigned int start_col_px = tile->column * 6;

            for(unsigned int i = 0; i < 12; i++) {
                unsigned char byte = tile->tilePixels[i];
                // Bitmasks for the 6 lower bits
                unsigned char bits[6] = { 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
                for (int j = 0; j < 6; j++) {
                    if (0 == (byte & bits[j])) {
                        // XOR means xor the existing color index for that pixel with color0
                        if (packet->type == TILE_BLOCK_XOR) {
                            unsigned char current_color = cdg_state->pixels[start_row_px + i][start_col_px + j];
                            unsigned char xor_color = current_color ^ tile->color0;
                            cdg_state->pixels[start_row_px + i][start_col_px + j] = xor_color;
                        // Otherwise we just set the color index to color0
                        } else {
                            cdg_state->pixels[start_row_px + i][start_col_px + j] = tile->color0;
                        }
                    } else {
                        // XOR means xor the existing color index for that pixel with color1
                        if (packet->type == TILE_BLOCK_XOR) {
                            unsigned char current_color = cdg_state->pixels[start_row_px + i][start_col_px + j];
                            unsigned char xor_color = current_color ^ tile->color1;
                            cdg_state->pixels[start_row_px + i][start_col_px + j] = xor_color;
                        // Otherwise we just set the color index to color1
                        } else {
                            cdg_state->pixels[start_row_px + i][start_col_px + j] = tile->color1;
                        }
                    }
                }
            }
            printf("Tile block with color0: %i, color1: %i, row: %i, column: %i\n", tile->color0, tile->color1, tile->row, tile->column);

            break;
        }

        case LOAD_COLORS_LOW: {
            CDG_RGB *rgbArray = (CDG_RGB *)(packet->data);
            for (int i = 0; i < 8; i++) {
                cdg_state->color_table[i] = rgbArray[i];
            }
            break;
        }

        case LOAD_COLORS_HIGH: {
            CDG_RGB *rgbArray = (CDG_RGB *)(packet->data);
            for (int i = 8; i < 16; i++) {
                cdg_state->color_table[i] = rgbArray[i];
            }
            break;
        }

        case SCROLL_COPY: {
            // TODO: Handle copy-scrolling
            break;
        }
        case SCROLL_PRESET: {
            CDG_Scroll *scroll = (CDG_Scroll *)packet->data;
            unsigned char preset_color = scroll->color;

            /*
             * Because this is the preset version, we can simply split up
             * the scrolling into a vertical part and horizontal part when
             * acting on it. Any blank spaces on any axis will be filled with
             * the preset_color anyway.
             */

            // Horizontal scrolling is done with 6 pixels
            switch(scroll->hScroll_cmd) {
                case SCROLL_RIGHT:
                    // Scroll pixels to the right
                    for(int i = 6; i < CDG_SCREEN_WIDTH; i++) {
                        for(int j = 0; j < CDG_SCREEN_HEIGHT; j++) {
                            cdg_state->pixels[i][j] = cdg_state->pixels[i-6][j];
                        }
                    }

                    // Fill in the missing 6 x SCREEN_HEIGHT pixels
                    for(int i = 0; i < 6; i++) {
                        for(int j = 0; j < CDG_SCREEN_HEIGHT; j++) {
                            cdg_state->pixels[i][j] = preset_color;
                        }
                    }
                    break;
                case SCROLL_LEFT:
                    // Scroll pixels to the left
                    for(int i = CDG_SCREEN_WIDTH - 6; i > 0; i--) {
                        for(int j = 0; j < CDG_SCREEN_HEIGHT; j++) {
                            cdg_state->pixels[i][j] = cdg_state->pixels[i+6][j];
                        }
                    }

                    // Fill in the missing 6 x SCREEN_HEIGHT pixels
                    for(int i = CDG_SCREEN_WIDTH - 6; i < CDG_SCREEN_WIDTH; i++) {
                        for(int j = 0; j < CDG_SCREEN_HEIGHT; j++) {
                            cdg_state->pixels[i][j] = preset_color;
                        }
                    }
                    break;
                default:
                    break;
            }

            // Vertical scrolling is done with 12 pixels
            switch(scroll->vScroll_cmd) {
                case SCROLL_DOWN:
                    // TODO: Implement
                    break;
                case SCROLL_UP:
                    // TODO: Implement
                    break;
                default:
                    break;
            }
            break;
        }

        case DEFINE_TRANSPARENT: {
            unsigned char *color = (unsigned char *)packet->data;
            cdg_state->transparent_color = *color;
            break;
        }

        default:
            return 1; // This should be an error
            break;
    }

    return 0;
}

CDG_Packet cdg_parse_packet(SubCode *sub)
{
    char instr;
    int i;

    CDG_Packet packet;

    // Even if a packet contains no data, we need it for timing purposes
    if (!cdg_contains_data(sub)) {
        packet.type = EMPTY;
        packet.data = NULL;
    }

    instr = cdg_get_instruction(sub);
    switch (instr) {
        case CDG_MEMORY_PRESET:
            printf("INSTR: MEMORY_PRESET\n");
            if( (sub->data[1] & 0x0F) != 0) { // Repeat packet
                // We treat repeat packets as empty, since we assume
                // a realiable data stream
                packet.type = EMPTY;
                packet.data = NULL;
            } else {
                packet.type = MEMORY_PRESET;
                unsigned char color = sub->data[0] & 0x0F;
                packet.data = malloc(sizeof(color));
                memcpy(packet.data, &color, sizeof(color));
            }
            break;
        case CDG_BORDER_PRESET:
            printf("INSTR: BORDER_PRESET\n");
            packet.type = BORDER_PRESET;
            unsigned char color = sub->data[0] & 0x0F;
            packet.data = malloc(sizeof(color));
            memcpy(packet.data, &color, sizeof(color));
            break;
        case CDG_TILE_BLOCK:
        case CDG_TILE_BLOCK_XOR: 
            if (instr == CDG_TILE_BLOCK) {
                printf("INSTR: TILE_BLOCK\n");
                packet.type = TILE_BLOCK;
            } else {
                printf("INSTR: TILE_BLOCK_XOR\n");
                packet.type = TILE_BLOCK_XOR;
            }

            CDG_Tile *tile = (CDG_Tile *)(malloc (sizeof(CDG_Tile)));
            tile->color0 = sub->data[0] & 0x0F;
            tile->color1 = sub->data[1] & 0x0F;
            tile->row    = sub->data[2] & 0x1F;
            tile->column = sub->data[3] & 0x3F;

            for(i = 0; i < 12; i++) {
                // Only lower 6 bits of each byte are used
                tile->tilePixels[i] = sub->data[i+4] & 0x3F;
            }

            packet.data = tile;
            break;
        case CDG_LOAD_COLORS_LOW:
        case CDG_LOAD_COLORS_HIGH:
            if (instr == CDG_LOAD_COLORS_LOW) {
                printf("INSTR: LOAD_COLORS_LOW\n");
                packet.type = LOAD_COLORS_LOW;
            } else {
                printf("INSTR: LOAD_COLORS_HIGH\n");
                packet.type = LOAD_COLORS_HIGH;
            } 

            short data;
            unsigned char red;
            unsigned char green;
            unsigned char blue;

            int array_length = 8;
            CDG_RGB *array = calloc(array_length, sizeof(CDG_RGB));
            for (i = 0; i < array_length; i++) {
                // AND with 0x3F3F to clear P and Q channel
                data = sub->data[i] & 0x3F3F;

                // Parse out the red, green and blue parts and shift them down
                // to hold the least significant bits.
                //
                // The X's below mark the P and Q channels, which have already
                // been cleared.
                //
                // [---high byte---]   [---low byte----]
                //  7 6 5 4 3 2 1 0     7 6 5 4 3 2 1 0
                //  X X r r r r g g     X X g g b b b b

                red = (data >> 10) & 0x000F;
                blue = data & 0x000F;

                unsigned char green_high = (data >> 6) & 0x000C;
                unsigned char green_low = (data >> 4) & 0x000F;
                green = green_high | green_low;

                CDG_RGB rgb;
                rgb.red = red;
                rgb.green = green;
                rgb.blue = blue;

                array[i] = rgb;
            } 
            packet.data = array;
            break;
        case CDG_SCROLL_PRESET:
        case CDG_SCROLL_COPY:
            if (instr == CDG_SCROLL_PRESET) {
                printf("INSTR: SCROLL_PRESET\n");
                packet.type = SCROLL_PRESET;
            } else {
                printf("INSTR: SCROLL_COPY\n");
                packet.type = SCROLL_COPY;
            }

            CDG_Scroll *scroll = (CDG_Scroll *)(malloc (sizeof(CDG_Scroll)));
            scroll->color   = sub->data[0] & 0x0F;
            unsigned char hScroll = sub->data[1] & 0x3F;
            unsigned char vScroll = sub->data[2] & 0x3F;
            scroll->hScroll_cmd = (hScroll & 0x30) >> 4;
            scroll->hScroll_offset = hScroll & 0x07;
            scroll->vScroll_cmd = (vScroll & 0x30) >> 4;
            scroll->vScroll_offset = vScroll & 0x0F;

            packet.data = scroll;
            break;
        case CDG_DEFINE_TRANSPARENT:
            printf("INSTR: DEFINE_TRANSPARENT\n");
            packet.type = DEFINE_TRANSPARENT;
            packet.data = malloc(sizeof(sub->data[0]));
            sub->data[0] &= 0x0F; // Only lower four bits
            memcpy(packet.data, &sub->data[0], sizeof(sub->data[0]));
            break;
        default: 
            printf("INSTR: Caught default clause!\n");
            packet.type = EMPTY;
            packet.data = NULL;
            break;
    }

    return packet;
}

void cdg_packet_put(CDG_Packet *packet)
{
    if (packet->data != NULL) {
        free(packet->data);
    }
}

unsigned char cdg_get_command(SubCode *sub)
{
    return sub->command & CDG_MASK;
}

int cdg_contains_data(SubCode *sub)
{
    return cdg_get_command(sub) == CDG_COMMAND;
}

unsigned char cdg_get_instruction(SubCode *sub)
{
    return sub->instruction & CDG_MASK;
}

