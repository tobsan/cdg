/**
 * An implementation of the CD+G format, as described here
 * http://jbum.com/cdg_revealed.html
 */

#ifndef CDG_H
#define CDG_H

#include <sys/time.h>
#include <stdio.h>

#define CDG_SCREEN_WIDTH 300
#define CDG_SCREEN_HEIGHT 216
#define CDG_VIEW_WIDTH 294
#define CDG_VIEW_HEIGHT 204

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

// TODO: Move to some other file
#define CDG_PACKETS_PER_SECTOR 4
#define CDG_SECTORS_PER_SECOND 75
#define CDG_PACKETS_PER_SECOND (CDG_PACKETS_PER_SECTOR * CDG_SECTORS_PER_SECOND)
#define CDG_USECS_PER_PACKET (1000000 / CDG_PACKETS_PER_SECOND)

typedef struct {
    char  command;
    char  instruction;
    char  parityQ[2];
    char  data[16];
    char  parityP[4];
} SubCode;

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

// Tiles ar 6x12
typedef struct {
    char    color0;          // Only lower 4 bits are used, mask with 0x0F
    char    color1;          // Only lower 4 bits are used, mask with 0x0F
    char    row;             // Only lower 5 bits are used, mask with 0x1F
    char    column;          // Only lower 6 bits are used, mask with 0x3F
    char    tilePixels[12];  // Only lower 6 bits of each byte are used
} CDG_Tile;

typedef struct {
    char    color;   // Only lower 4 bits are used, mask with 0x0F
    char    hScroll; // Only lower 6 bits are used, mask with 0x3F
    char    vScroll; // Only lower 6 bits are used, mask with 0x3F
} CDG_Scroll;

typedef struct {
    char red;   // 4 bits, located in most significant bits
    char green; // 4 bits, located in most significant bits
    char blue;  // 4 bits, located in most significant bits
} CDG_RGB;

union CDG_Packet {
    char mem_preset;        // Color only, mask with 0x0F
    char border_preset;     // Color only, mask with 0x0F
    char transparent;       // Color only, mask with 0x0F
    CDG_Tile tile;
    CDG_Scroll scroll;
    CDG_RGB load_clut[8];
}; 

typedef struct {
    packet_t packet_type;
    union CDG_Packet packet_data;
} CDG_Data;

typedef struct {
    unsigned int size;
    CDG_Data **packets;
} CDG_Array;

CDG_Array read_cdg_file(char *filename);
int cdg_parse_packet(SubCode *sub, CDG_Data *packet);
int cdg_diff_timestamp(struct timeval *start, struct timeval *stop);
char cdg_get_command(SubCode *sub);
int cdg_contains_data(SubCode *sub);
char cdg_get_instruction(SubCode *sub);

#endif // CDG_H
