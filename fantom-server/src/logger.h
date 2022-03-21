#pragma once
#include "ansi_colour.h"

#define LOG_ERROR ANSI_RED "error" ANSI_RESET
#define LOG_WARNING ANSI_YELLOW "warning" ANSI_RESET
#define LOG_INFO ANSI_GREEN "info" ANSI_RESET
#define LOG_STREAM stderr

void logf(const char *tag, const char *fmt, ...);

