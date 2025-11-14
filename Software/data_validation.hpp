/**
 * FILE: data_validation.hpp
 * PURPOSE: To contain the function and class definitions for container classes pertaining to
 *          data validation so that it may be known whether a value, such as a uint8_t, is a
 *          valid value and should be used in processing or not.
 *
 * UPDATED: 13 Nov. 2025
 */

#ifndef INC_DATA_VALIDATION_HPP_
#define INC_DATA_VALIDATION_HPP_

#include <string>

/**
 * A template class that contains a given value and a `bool` determining whether it is valid or not.
 * The validity of the value must be given when creating the `Field`, otherwise it is assumed to be
 * invalid.
 * If `valid` is `false`, then the value is invalid.
 * If `valid` is `true`, then the value is valid.
 * This can be defined depending on user preference, however this should be the consistent standard.
 */
template <typename T> class Field
{
    T value;
    bool valid;

    public:
    friend bool operator==(const Field<T>& a, const T& b) { return equals<T>(a, b); }
    friend bool operator==(const T& a, const Field<T>& b) { return equals<T>(b, a); }
    friend bool operator!=(const Field<T>& a, const T& b) { return !equals<T>(a, b); }
    friend bool operator!=(const T& a, const Field<T>& b) { return !equals<T>(b, a); }
    friend bool operator<(const Field<T>& a, const T& b) { return a.value < b; }
    friend bool operator<(const T& a, const Field<T>& b) { return a < b.value; }
    friend bool operator>(const Field<T>& a, const T& b) { return a.value > b; }
    friend bool operator>(const T& a, const Field<T>& b) { return a > b.value; }
    friend bool operator<=(const Field<T>& a, const T& b) { return a.value <= b; }
    friend bool operator<=(const T& a, const Field<T>& b) { return a <= b.value; }
    friend bool operator>=(const Field<T>& a, const T& b) { return a.value >= b; }
    friend bool operator>=(const T& a, const Field<T>& b) { return a >= b.value; }
    friend T operator+(const Field<T>& a, const T& b) { return a.value + b; }
    friend T operator+(const T& a, const Field<T>& b) { return a + b.value; }
    friend T operator*(const Field<T>& a, const T& b) { return a.value * b; }
    friend T operator*(const T& a, const Field<T>& b) { return a * b.value; }
    friend T operator/(const Field<T>& a, const T& b) { return a.value / b; }
    friend T operator/(const T& a, const Field<T>& b) { return a / b.value; }
    friend T operator-(const Field<T>& a, const T& b) { return a.value - b; }
    friend T operator-(const T& a, const Field<T>& b) { return a - b.value; }

    public:
    Field();
    Field(const Field<T>& field);
    Field(T value);
    Field(T value, bool valid);
    void setValue(T value, bool valid);
    void apply(T (* function)(T value));
    const T * const getValue();
    bool getValid();

    private:
    template <typename U> requires (!std::same_as<U, std::string>) static bool equals(const Field<U>& a, const U& b);
    template <typename U> requires (std::same_as<U, std::string>) static bool equals(const Field<U>& a, const U& b);
};

/**
 * Include the template implementation after declaration
 * NOTE: This is done as templates must either be fully defined in the header
 *       or have specific implementations specified. To circumvent this, a file
 *       with the template implementation can be included in the header to include
 *       the implementation. This reduces the code in the header file.
 * NOTE: Do NOT include data_validation.tpp at the beginning of this file or at any point
 *       in other header files.
 * REFERENCE: https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
 * REFERENCE: https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
 */
#include "data_validation.tpp"

#endif