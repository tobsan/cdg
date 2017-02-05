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
    cdg_read_file(filename);
    
    return 0;
}
