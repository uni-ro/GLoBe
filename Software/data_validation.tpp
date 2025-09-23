/**
 * FILE: data_validation.tpp
 * PURPOSE: To serve as the template implementation file for all of the classes
 *          relating to data validation.
 * NOTE: Do NOT include this file other than at the end of data_validation.hpp.
 *       Any other includes may lead to issues.
 */

/* ------------------------- Field Definitions -------------------------- */

template <typename T>
Field<T>::Field() : value() // Automatically initialises the value to the default value
{
    this->valid = false;
}

template <typename T>
Field<T>::Field(const Field<T>& field)
{
    this->value = field.value;
    this->valid = field.valid;
}

template <typename T>
Field<T>::Field(T value) : Field<T>::Field(value, false)
{
}

template <typename T>
Field<T>::Field(T value, bool valid)
{
    this->value = value;
    this->valid = valid;
}

template <typename T>
void Field<T>::setValue(T value, bool valid)
{
    this->value = value;
    this->valid = valid;
}

template <typename T>
const T * const Field<T>::getValue()
{
    if (this->valid)
    {
        return &value;
    }
    else
    {
        return NULL;
    }
}

template <typename T>
bool Field<T>::getValid()
{
    return this->valid;
}

/* Equals implementation for all T */
template <typename T>
bool Field<T>::equals(const Field<T>& a, const T& b)
{
    return a.value == b;
}

/* Equals implementation for specifically std::string */
template <>
bool Field<std::string>::equals(const Field<std::string>& a, const std::string& b)
{
    return a.value.compare(b) == 0;
}

/* ----------------------- End Field Definitions ------------------------ */