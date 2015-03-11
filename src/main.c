#include <config.h>
#include <stdlib.h>
#include <stdio.h>

#include "cdg.h"

int main(int argc, char **argv)
{
    // Open file.
    if(argc < 2) {
        printf("Usage: %s <cdg-file>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    CDG_Array contents = cdg_read_file(filename);
    
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
