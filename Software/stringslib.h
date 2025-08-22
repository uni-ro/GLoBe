#ifndef INC_STRINGSLIB_H_
#define INC_STRINGSLIB_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

char * strnext(char ** input, const char * delim);
uint16_t numTokens(const char* string, const char * token);
char ** splitString(const char * string, const char * delim, uint16_t * arr_size);

#endif