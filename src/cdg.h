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

/**
 * An implementation of the CD+G format, as described here
 * http://jbum.com/cdg_revealed.html
 */

#ifndef CDG_H
#define CDG_H

/**
 * Defines the content of one packet
 */

#define CDG_MASK 0x3F
#define CDG_COMMAND 0x09

/**
 * Valid instruction values
 */
#define CDG_MEMORY_PRESET 1
#define CDG_BORDER_PRESET 2
#define CDG_TILE_BLOCK 6
#define CDG_SCROLL_PRESET 20
#define CDG_SCROLL_COPY 24
#define CDG_DEFINE_TRANSPARENT 28
#define CDG_LOAD_COLORS_LOW 30
#define CDG_LOAD_COLORS_HIGH 31
#define CDG_TILE_BLOCK_XOR 38

#define CDG_PACKETS_PER_SECTOR 4
#define CDG_SECTORS_PER_SECOND 75
#define CDG_PACKETS_PER_SECOND (CDG_PACKETS_PER_SECTOR * CDG_SECTORS_PER_SECOND)

// RAW DATA
typedef struct {
    char  command; // Magic number for CDG
    char  instruction;
    char  parityQ[2];
    char  data[16];
    char  parityP[4];
} SubCode;

// The different instructions available
typedef enum {
    EMPTY,
    MEMORY_PRESET,
    BORDER_PRESET,
    TILE_BLOCK,
    TILE_BLOCK_XOR,
    LOAD_COLORS_LOW,
    LOAD_COLORS_HIGH,
    // These below are less commonly used
    SCROLL_PRESET,
    SCROLL_COPY,
    DEFINE_TRANSPARENT
} packet_t;

// The data length is always 16 bytes, so no this is TLV without the L.
typedef struct {
    packet_t type;
    void *data;
} CDG_Packet;

typedef struct {
    CDG_Packet **packets;
    unsigned int size;
} CDG_Array;

// Tiles are 6x12
typedef struct {
    unsigned char    color0;          // Only lower 4 bits are used, mask with 0x0F
    unsigned char    color1;          // Only lower 4 bits are used, mask with 0x0F
    unsigned char    row;             // Only lower 5 bits are used, mask with 0x1F
    unsigned char    column;          // Only lower 6 bits are used, mask with 0x3F
    unsigned char    tilePixels[12];  // Only lower 6 bits of each byte are used
} CDG_Tile;

typedef struct {
    unsigned char color;   // Only lower 4 bits are used, mask with 0x0F
    unsigned char hScroll; // Only lower 6 bits are used, mask with 0x3F
    unsigned char vScroll; // Only lower 6 bits are used, mask with 0x3F
} CDG_Scroll;

typedef struct {
    unsigned char red;   // 4 bits, located in most significant bits
    unsigned char green; // 4 bits, located in most significant bits
    unsigned char blue;  // 4 bits, located in most significant bits
} CDG_RGB;

// Read functions
CDG_Packet **cdg_read_file(char *filename);
// Parses a single packet given a 24-byte SubCode
CDG_Packet cdg_parse_packet(SubCode *sub);

// General function to free
void cdg_packet_put(CDG_Packet *packet);

// Auxiliary functions
unsigned char cdg_get_command(SubCode *sub);
int cdg_contains_data(SubCode *sub);
unsigned char cdg_get_instruction(SubCode *sub);

#endif // CDG_H
