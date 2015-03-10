#include "cdg.h"

#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // Open file.
    if(argc < 2) {
        printf("Usage: %s <cdg-file>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    CDG_Array contents = read_cdg_file(filename);
    
    CDG_Data *current;
    int i;
    for(i = 0; i < contents.size; i++) {
        current = contents.packets[i];
        switch(current->packet_type) {
            case EMPTY:
            case MEMORY_PRESET:
            case BORDER_PRESET:
            case TILE_BLOCK:
            case TILE_BLOCK_XOR:
            case LOAD_COLORS_LOW:
            case LOAD_COLORS_HIGH:
            case SCROLL_PRESET:
            case SCROLL_COPY:
            case DEFINE_TRANSPARENT:
                break;
        }
        // Don't forget to do this for EACH block!
        free(current);
    }

    free(contents.packets);
    return 0;
}

CDG_Array read_cdg_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        exit(2);
    }

    // Allocate enough for four minutes.
    int size = CDG_PACKETS_PER_SECOND * 240;
    CDG_Data **buf = malloc(sizeof(CDG_Data *) * size);

    SubCode subcode;
    int i = 0;

    // While file has not ended
    while (fread(&subcode, sizeof(SubCode), 1, file)) {
        CDG_Data *packet = malloc(sizeof(CDG_Data));
        cdg_parse_packet(&subcode, packet);
        buf[i++] = packet;

        // We need to resize the buffer, so we increase by 50%
        if(i == size) {
            size = size * 3 / 2;
            buf = realloc(buf, sizeof(CDG_Data *) * size);
        }
    }

    // We may have allocated too much.
    size = i; 
    buf = realloc(buf, sizeof(CDG_Data *) * size);

    // Sanity check and close the file
    if (!feof(file) || ferror(file)) {
        fprintf(stderr, "Reading of file failed\n");
    }
    fclose(file);
    // Done doing file stuff    

    CDG_Array result;
    result.size = size;
    result.packets = buf;

    return result;
}

int cdg_parse_packet(SubCode *sub, CDG_Data *packet)
{
    char instr;
    int i;

    if (cdg_contains_data(sub)) {
        instr = cdg_get_instruction(sub);
        switch (instr) {
            case CDG_MEMORY_PRESET:
                if( (sub->data[1] & 0x0F) != 0) { // Repeat packet
                    packet->packet_type = EMPTY;
                } else {
                    packet->packet_type = MEMORY_PRESET;
                    packet->packet_data.mem_preset = sub->data[0] & 0x0F; // Color
                }
                break;
            case CDG_BORDER_PRESET:
                packet->packet_type = BORDER_PRESET;
                packet->packet_data.border_preset = sub->data[0] & 0x0F; // Color
                break;
            case CDG_TILE_BLOCK:
            case CDG_TILE_BLOCK_XOR: 
                if (instr == CDG_TILE_BLOCK) {
                    packet->packet_type = TILE_BLOCK;
                } else {
                    packet->packet_type = TILE_BLOCK_XOR;
                }
                packet->packet_data.tile = (CDG_Tile) {
                    sub->data[0] & 0x0F, // Color0
                    sub->data[1] & 0x0F, // Color1
                    sub->data[2] & 0x1F, // Row
                    sub->data[3] & 0x3F, // Column
                };

                for(i = 0; i < 12; i++) {
                    // Only lower 6 bits of each byte are used
                    packet->packet_data.tile.tilePixels[i] = sub->data[i+4] & 0x3F;
                }
                break;
            case CDG_SCROLL_PRESET:
            case CDG_SCROLL_COPY:
                if (instr == CDG_SCROLL_PRESET) {
                    packet->packet_type = SCROLL_PRESET;
                } else {
                    packet->packet_type = SCROLL_COPY;
                }
                
                packet->packet_data.scroll = (CDG_Scroll) {
                    sub->data[0] & 0x0F, // Color
                    sub->data[1] & 0x3F, // hScroll
                    sub->data[2] & 0x3F  // vScroll
                };
                break;
            case CDG_DEFINE_TRANSPARENT:
                packet->packet_type = DEFINE_TRANSPARENT;
                packet->packet_data.transparent = sub->data[0];
                break;
            case CDG_LOAD_COLORS_LOW:
            case CDG_LOAD_COLORS_HIGH:
                if (instr == CDG_LOAD_COLORS_LOW) {
                    packet->packet_type = LOAD_COLORS_LOW;
                } else {
                    packet->packet_type = LOAD_COLORS_HIGH;
                } 

                short data;
                char red;
                char green;
                char blue;
                for (i = 0; i < 8; i++) {
                    // AND with 0x3F3F to clear P and Q channel
                    data = sub->data[i] & 0x3F3F;
                    red = (data & 0x3C00) << 2;
                    green = ((data & 0x0030) << 6) | ((data & 0x0300) << 8);
                    blue = (data & 0x000F) << 12;

                    packet->packet_data.load_clut[i] = (CDG_RGB) {
                        red, green, blue
                    };
                } 
                break;
            default: 
                packet->packet_type = EMPTY;
                return -1;
                break;
        }
    } else {
        // We need all the packets for timing
        packet->packet_type = EMPTY;
    }

    return 0;

}

int cdg_diff_timestamp(struct timeval *start, struct timeval *stop)
{
    return ((stop->tv_sec - start->tv_sec) * 1e6) + (stop->tv_usec - start->tv_usec);
}

char cdg_get_command(SubCode *sub)
{
    return sub->command & CDG_MASK;
}

int cdg_contains_data(SubCode *sub)
{
    return cdg_get_command(sub) == CDG_COMMAND;
}

char cdg_get_instruction(SubCode *sub)
{
    return sub->instruction & CDG_MASK;
}

