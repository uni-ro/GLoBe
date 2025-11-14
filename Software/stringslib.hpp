#ifndef INC_STRINGSLIB_HPP_
#define INC_STRINGSLIB_HPP_

#include <cmath>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "data_validation.hpp"

void strtouint8(const char * const str, Field<uint8_t>& field, uint8_t base=10);
void strtouint16(const char * const str, Field<uint16_t>& field, uint8_t base=10);
void strtouint32(const char * const str, Field<uint32_t>& field, uint8_t base=10);
void strtouint64(const char * const str, Field<uint64_t>& field, uint8_t base=10);
void strtoint8(const char * const str, Field<int8_t>& field, uint8_t base=10);
void strtoint16(const char * const str, Field<int16_t>& field, uint8_t base=10);
void strtoint32(const char * const str, Field<int32_t>& field, uint8_t base=10);
void strtofloat(const char * const str, Field<float_t>& field);
void strtodouble(const char * const str, Field<double_t>& field);

#endif