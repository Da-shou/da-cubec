#include <stdio.h>
#include <stdlib.h>

#include "utils/io_utils.h"

int read_file(const char* filepath, const char** const out) {
    // Checking file in raw bytes
    FILE* fptr = fopen(filepath, "rb");
    if (!fptr) {
        perror("Error opening file");
        return -1;
    }

    // Determine filesize
    if (fseek(fptr, 0, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(fptr);
        return -1;
    }

    const long fsize = ftell(fptr);
    if (fsize == -1) {
        perror("Error getting file size");
        fclose(fptr);
        return -1;
    }
    rewind(fptr);

    // Allocate memory for file reading
    char* const buffer = malloc((size_t)fsize + 1);
    if (!buffer) {
        perror("Error allocating memory");
        fclose(fptr);
        return -1;
    }

    // Read bytes into buffer
    const size_t bytes = fread((void*)buffer, 1, (size_t)fsize, fptr);
    if (bytes != (size_t)fsize) {
        perror("Error reading file");
        free((void*)buffer);
        fclose(fptr);
        return -1;
    }

    if (fclose(fptr)) {
        perror("Error closing file");
        free((void*)buffer);
        return -1;
    }

    // Null terminate the buffer
    buffer[fsize] = '\0';
    *out = buffer;

    return 0;
}
