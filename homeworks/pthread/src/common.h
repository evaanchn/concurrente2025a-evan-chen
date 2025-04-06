// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define REPORTS_DIRECTORY "reports"

/**
 * @brief Constructs a full file path by combining a directory and a file name.
 * @param directory The directory path (e.g., "home/user/docs").
 * @param file_name The name of the file (e.g., "file.txt").
 * @return A newly allocated string containing the full file path 
 *         (e.g., "/home/user/docs/file.txt"). Allocated memory must be freed
 */
char* build_file_path(const char* directory, const char* file_name);

/**
 * @brief Extracts the directory path from a given file path.
 * 
 * @param filepath A file path (e.g., "/home/user/docs/file.txt").
 * @return A newly allocated string containing the directory path 
 *         (e.g., "/home/user/docs").
 *         Returns NULL if no directory is found.
 */
char* extract_directory(const char* filepath);


/// @brief Extracts the file name (without the directory path)
/// @see extract_directory
char* extract_file_name(const char* path);

/**
 * @brief Modifies the extension of a given filename.
 * 
 * @param file_name The original filename (e.g., "document.txt").
 * @param new_extension The new extension without a leading dot (e.g., "tsv").
 * @return A newly allocated string containing the filename 
 *         with the new extension (e.g., "document.tsv").
 *         Returns NULL if no extension exists in the original filename.
 */
char* modify_extension(const char* file_name, const char* new_extension);

#endif  // COMMON_H
