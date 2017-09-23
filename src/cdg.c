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

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

CDG_Packet **cdg_read_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        exit(2);
    }

    SubCode subcode;

    // While file has not ended
    while (fread(&subcode, sizeof(SubCode), 1, file)) {
        
        CDG_Packet *packet = cdg_parse_packet(&subcode);
        if (!packet) {
            printf("Failed to parse packet\n");
            exit(1);
        }

        printf("Packet instruction: %i\n", packet->type);
        switch(packet->type) {
            case EMPTY:
                printf("Packet is empty\n");
                break;
            case MEMORY_PRESET: {
                unsigned int *color = (unsigned int *)packet->data;
                printf("Memory preset with color: %i\n", *color);
                break;
            }
            case BORDER_PRESET: {
                unsigned int *color = (unsigned int *)packet->data;
                printf("Border preset with color: %i\n", *color);
                break;
            }
            case TILE_BLOCK:
            case TILE_BLOCK_XOR: {
                CDG_Tile *tile = (CDG_Tile *)packet->data;
                printf("Tile block with color0: %i, color1: %i, row: %i, column: %i\n", tile->color0, tile->color1, tile->row, tile->column);
                // TODO: Print tile pixels
                break;
            }
            case LOAD_COLORS_LOW:
            case LOAD_COLORS_HIGH: {
                CDG_RGB *rgbArray = (CDG_RGB *)(packet->data);
                for (int i = 0; i < 8; i++) {
                    CDG_RGB rgb = rgbArray[i];
                    printf("Color %i-%i-%i\n",
                           rgb.red, rgb.green, rgb.blue);
                }
                break;
            }
            case SCROLL_COPY:
            case SCROLL_PRESET: {
                CDG_Scroll *scroll = (CDG_Scroll *)packet->data;
                printf("Scroll color: %i, hscroll: %i, vscroll: %i\n",
                       scroll->color, scroll->hScroll, scroll->vScroll);
                break;
            }
            case DEFINE_TRANSPARENT: {
                unsigned int *color = (unsigned int *)packet->data;
                printf("Transparent color: %i\n", *color);
                break;
            }
            default:
                break;
        }

        cdg_packet_put(packet);
    }

    // Sanity check and close the file
    if (!feof(file) || ferror(file)) {
        fprintf(stderr, "Reading of file failed\n");
    }
    fclose(file);
    // Done doing file stuff    

    return; // TODO: return something proper
}

CDG_Packet *cdg_parse_packet(SubCode *sub)
{
    char instr;
    int i;

    CDG_Packet *packet = malloc(sizeof(CDG_Packet));
    if (!packet) {
        fprintf(stderr, "Couldn't allocate packet\n");
        return NULL;
    }

    // Even if a packet contains no data, we need it for timing purposes
    if (!cdg_contains_data(sub)) {
        packet->type = EMPTY;
        packet->data = NULL;
    }

    instr = cdg_get_instruction(sub);
    switch (instr) {
        case CDG_MEMORY_PRESET:
            if( (sub->data[1] & 0x0F) != 0) { // Repeat packet
                packet->type = EMPTY;
                packet->data = NULL;
            } else {
                packet->type = MEMORY_PRESET;
                unsigned int color = sub->data[0] & 0x0F;
                packet->data = malloc(sizeof(color));
                memcpy(packet->data, &color, sizeof(color));
            }
            break;
        case CDG_BORDER_PRESET:
            packet->type = BORDER_PRESET;
            unsigned int color = sub->data[0] & 0x0F;
            packet->data = malloc(sizeof(color));
            memcpy(packet->data, &color, sizeof(color));
            break;
        case CDG_TILE_BLOCK:
        case CDG_TILE_BLOCK_XOR: 
            if (instr == CDG_TILE_BLOCK) {
                packet->type = TILE_BLOCK;
            } else {
                packet->type = TILE_BLOCK_XOR;
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

            packet->data = tile;
            break;
        case CDG_LOAD_COLORS_LOW:
        case CDG_LOAD_COLORS_HIGH:
            if (instr == CDG_LOAD_COLORS_LOW) {
                packet->type = LOAD_COLORS_LOW;
            } else {
                packet->type = LOAD_COLORS_HIGH;
            } 

            short data;
            char red;
            char green;
            char blue;

            int arrayLength = 8;
            CDG_RGB *array = calloc(arrayLength, sizeof(CDG_RGB));
            for (i = 0; i < arrayLength; i++) {
                // AND with 0x3F3F to clear P and Q channel
                data = sub->data[i] & 0x3F3F;
                printf("Color data: %i\n", data);
                red = (data & 0x3C00) << 2;
                green = ((data & 0x0030) << 6) | ((data & 0x0300) << 8);
                blue = (data & 0x000F) << 12;
                printf("Parsing colors: %i-%i-%i\n", red, green, blue);

                CDG_RGB rgb;
                rgb.red = red;
                rgb.green = green;
                rgb.blue = blue;

                array[i] = rgb;
            } 
            packet->data = array;
            break;
        case CDG_SCROLL_PRESET:
        case CDG_SCROLL_COPY:
            if (instr == CDG_SCROLL_PRESET) {
                packet->type = SCROLL_PRESET;
            } else {
                packet->type = SCROLL_COPY;
            }

            CDG_Scroll *scroll = (CDG_Scroll *)(malloc (sizeof(CDG_Scroll)));
            scroll->color   = sub->data[0] & 0x0F;
            scroll->hScroll = sub->data[1] & 0x3F;
            scroll->vScroll = sub->data[2] & 0x3F;

            packet->data = scroll;
            break;
        case CDG_DEFINE_TRANSPARENT:
            packet->type = DEFINE_TRANSPARENT;
            packet->data = malloc(sizeof(sub->data[0]));
            memcpy(packet->data, &sub->data[0], sizeof(sub->data[0]));
            break;
        default: 
            packet->type = EMPTY;
            packet->data = NULL;
            break;
    }

    return packet;
}

void cdg_packet_put(CDG_Packet *packet)
{
    if (packet->data != NULL) {
        free(packet->data);
    }

    free(packet);
}

int cdg_write_file(CDG_Packet **packets, char *filename)
{
    return 0;
}

SubCode *cdg_create_packet(CDG_Packet *packet)
{
    switch (packet->type) {
        default:
            return 0;
            break;
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

