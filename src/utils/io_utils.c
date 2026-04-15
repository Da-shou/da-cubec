#include <stdio.h>
#include <stdlib.h>

#include "utils/io_utils.h"

int read_file(const char* filepath, const char** const out_text) {
    // Checking file in raw bytes
    FILE* fptr = fopen(filepath, "rb");
    if (!fptr) {
        perror("Error opening file");
        return -1;
    }

    // Determine filesize
    if (fseek(fptr, 0, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        (void)fclose(fptr);
        return -1;
    }

    const long fsize = ftell(fptr);
    if (fsize == -1) {
        perror("Error getting file size");
        (void)fclose(fptr);
        return -1;
    }

    if(fseek(fptr, 0, SEEK_SET) != 0) {
        perror("Error seeking to the start of file");
        (void)fclose(fptr);
        return -1;		
	};

    // Allocate memory for file reading
    char* const buffer = malloc((size_t)fsize + 1);
    if (!buffer) {
        perror("Error allocating memory");
        (void)fclose(fptr);
        return -1;
    }

    // Read bytes into buffer
    const size_t bytes = fread((void*)buffer, 1, (size_t)fsize, fptr);
    if (bytes != (size_t)fsize) {
        perror("Error reading file");
        free((void*)buffer);
        (void)fclose(fptr);
        return -1;
    }

    if (fclose(fptr)) {
        perror("Error closing file");
        free((void*)buffer);
        return -1;
    }

    // Null terminate the buffer
    buffer[fsize] = '\0';
    *out_text = buffer;

    return 0;
}

int read_file_bytes(const char* filepath, const unsigned char** const out_text, long* const out_size) {
    FILE* file = fopen(filepath, "rb");

    if (!file) {
        (void)fprintf(stderr, "text_renderer: could not open font file '%s'\n", filepath);
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        (void)fclose(file);
        return -1;
    }

    const long size = ftell(file);
    if (size <= 0) {
        (void)fclose(file);
        return -1;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        (void)fclose(file);
        return -1;
    };

    unsigned char* buf = malloc((size_t)size);
    if (!buf) {
        (void)fclose(file);
        return -1;
    }

    if (fread(buf, 1, (size_t)size, file) != (size_t)size) {
        free(buf);
        (void)fclose(file);
        return -1;
    }

    (void)fclose(file);

    *out_size = size;
    *out_text = buf;
    return 0;
}
