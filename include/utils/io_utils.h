#ifndef IO_UTILS_H
#define IO_UTILS_H

/**
 * @brief Reads an entire file and returns a pointer to a
 * dynamically allocated character array.
 * @warn THE RETURNED POINTER NEEDS TO BE FREED MANUALLY.
 * @param filepath Path to the file
 * @param out pointer to be modified with pointer to content of file.
 **/
int read_file(const char* filepath, const char** out);

#endif // IO_UTILS_H
