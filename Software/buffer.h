#ifndef INC_BUFFER_H_
#define INC_BUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t * buffNext(uint8_t ** buffer, const uint8_t * delim, size_t delimLength, size_t bufferStart, size_t bufferLength);
uint8_t * findInBuff(uint8_t * haystack, const uint8_t * needle, size_t needleLength, size_t bufferStart, size_t bufferLength);
uint8_t * copyFromBuff(uint8_t * buffer, size_t destLength, uint8_t * needle, size_t needleLength, size_t bufferStart, size_t bufferLength);
uint16_t numInstances(const uint8_t * buffer, size_t bufferLength, const uint8_t * delim, size_t delimLength);
uint8_t ** splitBuff(const uint8_t * buffer, size_t bufferLength, size_t startIdx, const uint8_t * delim, size_t delimLength, uint16_t * arr_size);


#ifdef __cplusplus
}
#endif

#endif