#ifndef INC_STRINGSLIB_H_
#define INC_STRINGSLIB_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Make the file both c and c++ compatible
#ifdef __cplusplus
extern "C"
{
#endif

char * strnext(char ** input, const char * delim);
uint16_t numTokens(const char* string, const char * token);
char ** splitString(const char * string, const char * delim, uint16_t * arr_size);

#ifdef __cplusplus
}
#endif

#endif