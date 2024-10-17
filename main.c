#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Command should be: '%s <inputFile>'\n", argv[0]);
        return 1;
    }

    char *inputFile = argv[1];
    char *IP_ADDR = argv[2];
    int PORT = atoi(argv[3]);
    read_data(inputFile,IP_ADDR,PORT);
    
}
