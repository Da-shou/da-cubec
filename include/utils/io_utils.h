#ifndef IO_UTILS_H
#define IO_UTILS_H

/**
 * @brief Reads an entire file and returns a pointer to a
 * dynamically allocated character array.
 * @warn THE RETURNED POINTER NEEDS TO BE FREED MANUALLY.
 * @param filepath Path to the file
 * @param out_text pointer to be modified with pointer to content of file.
 **/
int read_file(const char* filepath, const char** out_text);

/**
 * @brief Reads an entire file and returns a pointer to a
 * dynamically allocated character array.
 * @warn THE RETURNED POINTER NEEDS TO BE FREED MANUALLY.
 * @param filepath Path to the file
 * @param out_text pointer to be modified with pointer to content of file.
 * @param out_size size in bytes of the file.
 **/
int read_file_bytes(const char* filepath, const unsigned char** out_text, long* out_size);

#endif // IO_UTILS_H