// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#include "common.h"

char* build_file_path (char* directory, char* file_name) {
  // Allocate memory for the full path
  size_t length = strlen(directory) + strlen(file_name) + 1;
                                             // +1 for null terminator
  char *full_path = (char *)malloc(length);

  if (full_path == NULL) {
      perror("Memory allocation failed");
      return NULL;
  }

  // Merge strings safely
  snprintf(full_path, length, "%s%s", directory, file_name);

  return full_path;
}
