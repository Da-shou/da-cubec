#include <stdio.h>
#include <stdlib.h>

#include "io_utils.h"

const char* const read_file(const char* filepath) {
        // Checking file in raw bytes
        FILE* fptr = fopen(filepath, "rb");
        if (!fptr) {
                perror("Error opening file");
                return 0;
        }

        // Determine filesize
        if (fseek(fptr, 0, SEEK_END) != 0) {
                perror("Error seeking to end of file");
                fclose(fptr);
                return 0;
        }

        long fsize = ftell(fptr);
        if (fsize == -1) {
                perror("Error getting file size");
                fclose(fptr);
                return 0;
        }
        rewind(fptr);

        // Allocate memory for file reading
        char* const buffer = malloc(fsize + 1);
        if (!buffer) {
                perror("Error allocating memory");
                fclose(fptr);
                return 0;
        }

        // Read bytes into buffer
        size_t bytes = fread((void*)buffer, 1, fsize, fptr);
        if (bytes != fsize) {
                perror("Error reading file");
                free((void*)buffer);
                fclose(fptr);
                return 0;
        }

        if (fclose(fptr)) {
                perror("Error closing file");
                free((void*)buffer);
                return 0;
        }

        // Null terminate the buffer
        buffer[fsize] = '\0';
        const char* const source = buffer;
	return source;
}
