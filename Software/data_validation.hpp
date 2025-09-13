#ifndef INC_DATA_VALIDATION_HPP_
#define INC_DATA_VALIDATION_HPP_

template <typename T> class Field
{
    T value;
    bool valid;

    public:
    friend bool operator==(const Field<T>& a, const T& b) { return equals(a, b); }
    friend bool operator==(const T& a, const Field<T>& b) { return equals(b, a); }
    friend bool operator!=(const Field<T>& a, const T& b) { return a.value != b; }
    friend bool operator!=(const T& a, const Field<T>& b) { return a != b.value; }
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
    const T * const getValue();
    bool getValid();

    private:
    static bool equals(const Field<T>& a, const T& b);
};

/* Include the template implementation after declaration
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