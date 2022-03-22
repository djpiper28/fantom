#pragma once
#include "ansi_colour.h"

#define LOG_ERROR ANSI_RED      "error"   ANSI_RESET
#define LOG_WARNING ANSI_YELLOW "warning" ANSI_RESET
#define LOG_INFO ANSI_GREEN     "info"    ANSI_RESET
#define LOG_STREAM stderr

void __lprintf(const char *tag, const char *fmt, ...);

#ifdef DEBUG
/* __FILENAME__ is defined in ../CMakeLists.txt as this:

=====
	 
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
add_definitions("-D__FILENAME__=(__FILE__ + SOURCE_PATH_SIZE)")

=====

It removes the path to the files and gives you the src/ bit - very noice
*/
#define lprintf fprintf(LOG_STREAM, "(" ANSI_YELLOW "%s" ANSI_RESET \
				":" ANSI_YELLOW "%d" ANSI_RESET ") ", __FILENAME__, __LINE__ ),\
				__lprintf
#else
#define lprintf __lprintf
#endif

