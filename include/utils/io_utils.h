/**
 * @file io_utils.h
 * @authors Da-shou
 * Header file for helper functions wrapping all the necessary checks when reading the content of a file in C.
 */

#ifndef IO_UTILS_H
#define IO_UTILS_H

/**
 * @brief Reads an entire file.
 * @attention THE RETURNED POINTER NEEDS TO BE FREED MANUALLY.
 * @param filepath Path to the file
 * @param out_text Pointer to the content to be modified with the file content.
 * @return 0 if successful, -1 if an IO error occured.
 **/
int read_file(const char *filepath, const char **out_text);

/**
 * @brief Reads an entire file in bytes.
 * @attention THE RETURNED POINTER NEEDS TO BE FREED MANUALLY.
 * @param filepath Path to the file
 * @param out_text pointer to be modified with pointer to content of file.
 * @param out_size size in bytes of the file.
 * @return 0 if successful, -1 if an IO error occured.
 **/
int read_file_bytes(const char *filepath, const unsigned char **out_text, long *out_size);

#endif // IO_UTILS_H
