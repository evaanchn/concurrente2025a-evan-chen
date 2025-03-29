// Copyright 2025 Evan Chen Cheng <evan.chen@ucr.ac.cr>

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define REPORTS_DIRECTORY "reports"

char* build_file_path (char* directory, char* file_name);

char* extract_directory(const char* filepath);

char* extract_file_name(const char* path);

char* modify_extension(const char* file_name, const char* new_extension);

#endif  // COMMON_H
