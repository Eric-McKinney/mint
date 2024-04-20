#include <stdio.h>
#include <stdlib.h>

#define PROMPT "\033[38;5;49mmint\033[0m|> "
#define BUF_SIZE 1024

int main(int argc, char **argv) {
    FILE *input = stdin;
    char line[BUF_SIZE] = {0};

    /* Determine input stream */
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "File \"%s\" not found\n", argv[1]);
            return 1;
        }
    } else if (argc > 2) {
        fprintf(stderr, "%s: Too many args\n", argv[0]);
        return 1;
    }

    if (argc == 1) {
        printf(PROMPT);
    }

    while(fgets(line, BUF_SIZE, input)) {
        /* processing goes here */

        if (argc == 1) {
            printf(PROMPT);
        }
    }
    
    printf("\n");
    fclose(input);

    return 0;
}
