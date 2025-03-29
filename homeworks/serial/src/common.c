// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "common.h"

char* build_file_path (char* directory, char* file_name) {
  // Allocate memory for the full path, +1 for null terminator +1 for slash
  size_t length = strlen(directory) + strlen(file_name) + 2;

  char *full_path = (char *)malloc(length);

  if (!full_path) {
      perror("Error: Memory allocation failed in build_file_path\n");
      return NULL;
  }

  // Merge strings safely
  // int snprintf ( char * s, size_t n, const char * format, ... )
  snprintf(full_path, length, "%s/%s", directory, file_name);

  return full_path;
}

char* extract_directory(const char *filepath) {
  // Find the last occurrence of '/'.
  // char* strrchcr(char* str, int chr) -> returns pointer 
  // to last occurence of chr
  const char *last_slash = strrchr(filepath, '/');

  if (!last_slash) {
    perror ("Error: No file directory specified\n");
      return NULL;
  }

  size_t dir_length = last_slash - filepath; // Length up to last slash
  char *directory = (char *)malloc(dir_length + 1); // Allocate memory

  if (!directory) {
    perror("Error: Memory allocation for directory failed");
    return NULL;
  }

  strncpy(directory, filepath, dir_length);
  directory[dir_length] = '\0'; // Null-terminate

  return directory;
}

char *extract_file_name(const char *filepath) {
  if (!filepath) return NULL; // Handle NULL input

  // Find the last occurrence of '/'
  const char *last_slash = strrchr(filepath, '/');

  // If no slash is found, return a copy of the whole string (it's already a filename)
  const char *filename = last_slash ? last_slash + 1 : filepath;

  // Allocate memory for the new filename
  char *result = (char *)malloc(strlen(filename) + 1);
  if (!result) {
      perror("Memory allocation failed");
      return NULL;
  }

  // Copy the filename
  strcpy(result, filename);

  return result; // Caller must free()
}

char* modify_extension(const char *filename, const char *new_extension) {
  const char *last_dot = strrchr(filename, '.'); // Find the last '.'
  
  if (!last_dot) {
    perror("Error: in modify_extension(), no extension specified for file");
    return NULL;
  }

  size_t name_length = last_dot - filename;
  // Allocate memory: name + dot + extension + null terminator
  size_t new_size = name_length + strlen(new_extension) + 2;

  char *new_filename = (char *)malloc(new_size);

  if (!new_filename) {
      perror("Error: in modify_extension(), memory allocation failed");
      return NULL;
  }

  // Construct the new filename safely
  snprintf(new_filename, new_size, "%.*s.%s", (int)name_length, filename, 
      new_extension);

  return new_filename; // Caller must free()
}
