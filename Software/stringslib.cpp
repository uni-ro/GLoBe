#include "stringslib.hpp"

/* Private Function Declarations */
template <typename T>
void strtounsigned(const char * const str, const Field<T>&, uint64_t MAX_VAL, uint8_t base);
bool verifyUnsigned(const char * const str, const char * const endptr, unsigned long long val, uint64_t MAX_VAL);
template <typename T>
void strtosigned(const char * const str, const Field<T>& field, int64_t MIN_VAL, int64_t MAX_VAL, uint8_t base);
bool verifySigned(const char * const str, const char * const endptr, signed long long val, int64_t MIN_VAL, int64_t MAX_VAL);
/* End Private Function Declarations */

/**
 * Converts a C string to an unsigned long long and places the value in the given field.
 */
template <typename T>
void strtounsigned(const char * const str, Field<T>& field, uint64_t MAX_VAL, uint8_t base)
{
    char * endptr;
    unsigned long long val = 0;
    bool valid = false;

    val = strtoull(str, &endptr, base);

    valid = verifyUnsigned(str, endptr, val, MAX_VAL);

    field.setValue((T) val, valid);
}

/**
 * Verifies that the given string is an unsigned integer.
 */
bool verifyUnsigned(const char * const str, const char * const endptr, unsigned long long val, uint64_t MAX_VAL)
{
    // If the entire string was not consumed, return false
    if (*endptr != '\0' || *str == '\0')
    {
        return false;
    }

    // If the given value is negative, return false
    if (*str == '-')
    {
        return false;
    }

    // If the given value is greater than the max value, return false
    if (val > MAX_VAL)
    {
        return false;
    }

    return true;
}

void strtouint8(const char * const str, Field<uint8_t>& field, uint8_t base)
{
    strtounsigned<uint8_t>(str, field, UINT8_MAX, base);
}

void strtouint16(const char * const str, Field<uint16_t>& field, uint8_t base)
{
    strtounsigned<uint16_t>(str, field, UINT16_MAX, base);
}

void strtouint32(const char * const str, Field<uint32_t>& field, uint8_t base)
{
    strtounsigned<uint32_t>(str, field, UINT32_MAX, base);
}

void strtouint64(const char * const str, Field<uint64_t>& field, uint8_t base)
{
    strtounsigned<uint64_t>(str, field, UINT64_MAX, base);
}

template <typename T>
void strtosigned(const char * const str, Field<T>& field, int64_t MIN_VAL, int64_t MAX_VAL, uint8_t base)
{
    char * endptr;
    signed long long val = 0;
    bool valid = false;

    val = strtoull(str, &endptr, base);

    valid = verifySigned(str, endptr, val, MIN_VAL, MAX_VAL);

    field.setValue((T) val, valid);
}

bool verifySigned(const char * const str, const char * const endptr, signed long long val, int64_t MIN_VAL, int64_t MAX_VAL)
{
    // If the entire string was not consumed, return false
    if (*endptr != '\0' || *str == '\0')
    {
        return false;
    }

    // If the given value is greater than the max value or smaller than the min value, return false
    if (val > MAX_VAL || val < MIN_VAL)
    {
        return false;
    }

    return true;
}

void strtoint8(const char * const str, Field<int8_t>& field, uint8_t base)
{
    strtosigned<int8_t>(str, field, INT8_MIN, INT8_MAX, base);
}

void strtoint16(const char * const str, Field<int16_t>& field, uint8_t base)
{
    strtosigned<int16_t>(str, field, INT16_MIN, INT16_MAX, base);
}

void strtoint32(const char * const str, Field<int32_t>& field, uint8_t base)
{
    strtosigned<int32_t>(str, field, INT32_MIN, INT32_MAX, base);
}

void strtofloat(const char * const str, Field<float_t>& field)
{
    char * endptr;
    float_t val = 0;
    bool valid = false;

    val = strtof(str, &endptr);

    // If the entire string was consumed, set valid to true
    if (*endptr == '\0' && *str != '\0')
    {
        valid = true;
    }

    field.setValue(val, valid);
}

void strtodouble(const char * const str, Field<double_t>& field)
{
    char * endptr;
    double_t val = 0;
    bool valid = false;

    val = strtod(str, &endptr);

    // If the entire string was consumed, set valid to true
    if (*endptr == '\0' && *str != '\0')
    {
        valid = true;
    }

    field.setValue(val, valid);
}
