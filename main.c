#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Command should be: '%s <inputFile>'\n", argv[0]);
        return 1;
    }

    char *inputFile = argv[1];
    read_data(inputFile);
}
