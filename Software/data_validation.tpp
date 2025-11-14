/**
 * FILE: data_validation.tpp
 * PURPOSE: To serve as the template implementation file for all of the classes
 *          relating to data validation and declared in data_validation.hpp.
 *
 * UPDATED: 13 Nov. 2025
 * 
 * NOTE: Do NOT include this file other than at the end of data_validation.hpp.
 *       Any other includes may lead to issues.
 */

/* ------------------------- Field Definitions -------------------------- */

/**
 * The default constructor where the `value` is initialised to a default state and `valid` is `false`.
 */
template <typename T>
Field<T>::Field() : value() // Automatically initialises the value to the default value
{
    this->valid = false;
}

/**
 * The copy constructor for `Field`.
 * 
 * @param field The field object from which to copy the data.
 */
template <typename T>
Field<T>::Field(const Field<T>& field)
{
    this->value = field.value;
    this->valid = field.valid;
}

/**
 * The constructor where the `value` is initialised, but invalid.
 * 
 * @param value The value to set within the field.
 */
template <typename T>
Field<T>::Field(T value) : Field<T>::Field(value, false)
{}

/**
 * The constructor where both `value` and `valid` are initialised. The validity is to be defined here.
 * 
 * @param value The value to set within the field.
 * @param valid Whether or not the `value` is valid or not. `true` if valid and `false` if invalid.
 */
template <typename T>
Field<T>::Field(T value, bool valid)
{
    this->value = value;
    this->valid = valid;
}

/**
 * Sets the value of the field. The validity must also be provided.
 * 
 * @param value The value to set.
 * @param valid Whether or not the `value` is valid or not. `true` if valid and `false` if invalid.
 */
template <typename T>
void Field<T>::setValue(T value, bool valid)
{
    this->value = value;
    this->valid = valid;
}

/**
 * Applies a given function to the `value` of the field. This is used to update the internal `value`
 * according to the given function.
 * 
 * @note The function is only applied if the `value` is valid.
 * 
 * @param function The function to apply to the `value`.
 */
template <typename T>
void Field<T>::apply(T (* function)(T value))
{
    if (this->valid)
    {
        this->value = function(this->value);
    }
}

/**
 * Returns either the address of the internal value, or a NULL pointer. If the `value` is valid, then
 * a pointer to the `value` is provided. If the `value` is invalid, a NULL pointer is returned.
 * 
 * @returns A pointer to either the `value`, or NULL if the `value` is invalid.
 */
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

/**
 * Returns whether the `value` is valid or not.
 * 
 * @returns The validity of the `value`. If the returned value is `true`, then `value` is valid. If the
 *          returned value is `false`, then `value` is invalid.
 */
template <typename T>
bool Field<T>::getValid()
{
    return this->valid;
}

/**
 * This is used to determine whether a given value is equal to the `value` of the field.
 * 
 * @param a The `Field` object to compare the internal `value` with.
 * @param b The value to be compared with the `value` of the `Field` object from `a`.
 * 
 * @returns `true` if the `value` from the `Field` provided in `a` equals the value from `b`. `false` is
 *          returned otherwise.
 */
template <typename T>
template <typename U>
requires (!std::same_as<U, std::string>)
bool Field<T>::equals(const Field<U>& a, const U& b)
{
    return a.value == b;
}

/**
 * This is used to determine whether a given value is equal to the `value` of the field.
 * 
 * @note This function is a specific implementation of the equals function for `std::string` values
 *       only.
 * 
 * @param a The `Field` object to compare the internal string `value` with.
 * @param b The `std::string` to be compared with the `value` of the `Field` object from `a`.
 * 
 * @returns `true` if the `value` from the `Field` provided in `a` equals the value from `b`. `false` is
 *          returned otherwise.
 */
template <typename T>
template <typename U>
requires (std::same_as<U, std::string>)
bool Field<T>::equals(const Field<U>& a, const U& b)
{
    return a.value.compare(b) == 0;
}

/* ----------------------- End Field Definitions ------------------------ */