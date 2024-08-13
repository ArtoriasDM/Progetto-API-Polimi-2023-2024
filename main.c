#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STANDARD_BUFFER_LENGHT 300

int GetCommand(char ** buffer, FILE * fp, int * buffer_size);

int main()
{
    char * buffer = NULL;
    int buffer_size = 0;
    int line_lenght;

    line_lenght = GetCommand(&buffer, stdin, &buffer_size);
    printf("%d\n", line_lenght);
    printf("%s\n", buffer);

    return 0;
}

int GetCommand(char ** buffer, FILE * fp, int * buffer_size)
{
    int c;
    int line_lenght;

    if(*buffer == NULL)
    {
        *buffer = calloc(STANDARD_BUFFER_LENGHT, sizeof(char));
        *buffer_size = STANDARD_BUFFER_LENGHT;
    }

    line_lenght = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (line_lenght + 1 >= *buffer_size) {
            *buffer_size *= 2;
            *buffer = (char *)realloc(*buffer, *buffer_size);
        }
        if (c == '\n') {
            (*buffer)[line_lenght] = '\0';
            break;
        }
        (*buffer)[line_lenght++] = (char)c;
    }

    if(line_lenght == 0 || c == EOF)
        return -1;
    return line_lenght;
}