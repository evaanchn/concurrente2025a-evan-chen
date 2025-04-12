// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "common.h"

char* build_file_path(const char* directory, const char* filename) {
  if (!directory || !filename) return NULL;  // In case of NULL chars

  // Calculate required memory: directory + '/' + filename + '\0'
  size_t length = strlen(directory) + strlen(filename) + 2;

  // Allocate memory for the full path
  char *full_path = (char *)calloc(1, length);
  if (!full_path) {
      fprintf(stderr, "Error: Memory allocation failed in build_file_path\n");
      return NULL;
  }

  // Construct the full path safely, using snprintf
  snprintf(full_path, length, "%s/%s", directory, filename);
  return full_path;
}

char* extract_directory(const char *filepath) {
  if (!filepath) return NULL;  // Handle NULL input

  // Find the last occurrence of '/'
  const char *last_slash = strrchr(filepath, '/');
  if (!last_slash) return "";  // No directory found

  size_t dir_length = last_slash - filepath;  // Length up to last slash
  char *directory = (char*)calloc(1, dir_length + 1);  // Allocate memory

  if (!directory) {
    perror("Error: Memory allocation for directory failed");
    return NULL;
  }

  strncpy(directory, filepath, dir_length);
  directory[dir_length] = '\0';  // Null-terminate

  return directory;
}


char* extract_file_name(const char *filepath) {
  if (!filepath) return NULL;  // Handle NULL input

  // Find the last occurrence of '/'
  const char *last_slash = strrchr(filepath, '/');

  // If no slash is found, return a copy of the whole string
  // (it's already a filename)
  const char *filename = last_slash ? last_slash + 1 : filepath;

  // Allocate memory for the new filename
  char *result = (char *)calloc(1, strlen(filename) + 1);
  if (!result) {
      perror("Memory allocation for file name extraction failed");
      return NULL;
  }

  // Copy the filename
  snprintf(result, strlen(filename) + 1, "%s", filename);

  return result;  // Caller must free()
}

char* modify_extension(const char *file_name, const char *new_extension) {
  if (!file_name || !new_extension) return NULL;  // Handle NULL input

  // Find the last occurrence of '.' to locate the file extension
  const char *last_dot = strrchr(file_name, '.');
  if (!last_dot) return NULL;  // No extension found in filename

  // Calculate required memory:
  // base filename + dot + new extension + null terminator
  size_t name_length = last_dot - file_name;
  size_t new_size = name_length + strlen(new_extension) + 2;

  // Allocate memory for the new filename
  char *new_filename = (char *)calloc(1, new_size);
  if (!new_filename) return NULL;

  // Construct the new filename safely
  // %. *s indicates that the amount printed is limited by an int,
  // in this case name_length
  snprintf(new_filename, new_size, "%.*s.%s",
      (int)name_length, file_name, new_extension);

  return new_filename;
}

double get_elapsed_seconds(struct timespec* start_time
  , struct timespec* finish_time) {
  return finish_time->tv_sec - start_time->tv_sec +
      (finish_time->tv_nsec - start_time->tv_nsec) * 1e-9;
}