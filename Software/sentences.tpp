/**
 * FILE: sentences.tpp
 * PURPOSE: To serve as the template implementation file for all of the sentences
 * NOTE: Do NOT include this file other than at the end of sentences.hpp.
 *       Any other includes may lead to issues.
 */

/* ------------------------ Sentence Definitions ------------------------ */

template <typename T>
Sentence<T>::Sentence(char * line)
{
    static_assert(std::is_base_of_v<BASE, T>, "Ensure that the given type is a BASE type!");

    int8_t validFormat = -1;
    char ** lineArr = NULL;
    uint16_t length = 0;
    char delim[] = ",";

    /* Perform split and validation here then create sentence object */
    validFormat = Sentence::verifyFormat(line);

    /* Ensure that the format of the sentence is valid */
    if (validFormat == 0)
    {
        /* Ensure that the sentence has a correct checksum */
        if (Sentence::nmeaChecksum(line) == 0)
        {
            /* Split the sentence into its constituent parts and create an object from it */
            lineArr = splitString(line, delim, &length);
            if(isAcceptedSubtype<T>(lineArr[0]))
            {
                bool initialised;

                this->sentence = new T(lineArr, length);
                initialised = this->sentence->initialise(lineArr, length);

                /* If the sentence was not properly initialised, set it to NULL */
                if (!initialised)
                {
                    delete this->sentence;
                    this->sentence = NULL;
                }
            }
            else
            {
                this->sentence = NULL;
            }

            free(*lineArr);
            free(lineArr);
        }
        else
        {
            this->sentence = NULL;
        }
    }
    else
    {
        this->sentence = NULL;
    }
}

template <typename T>
T * const Sentence<T>::getSentence()
{
    return this->sentence;
}

/**
 * Verifies the format of the given NMEA message using regex.
 *
 * @param data The message to check for the correct format
 *
 * @returns 0 if the message is of the correct format, -1 if it is not
 * 			and -2 if the input is NULL.
 */
template <typename T>
int8_t Sentence<T>::verifyFormat(const char *data)
{
    /* TODO: Fix regex matching */
    static std::regex expr("^\\$.{2}.{3},+.*\\*.{2}(?:\r\n)?$", std::regex_constants::ECMAScript);
    bool res;

    /* Compile the regex to match the format of an NMEA message (note this matches
    both messages with \r\n at the end and without where a message always contains \r\n)*/

    if (data == NULL)
    {
        return -2;
    }

    std::string input = data;
    std::smatch match;

    res = std::regex_match(input, match, expr);

    if (res)
    {
        return 0;
    }

    return -1;
}

template <typename T>
template <typename U>
bool Sentence<T>::isAcceptedSubtype(char * header)
{
    static_assert(
        std::is_base_of_v<BASE, U> ||
        std::is_base_of_v<POS, U> ||
        std::is_base_of_v<TIME, U> ||
        std::is_base_of_v<STD_MSG_POLL, U>,
        "Ensure that the given type is an accepted type (BASE, POS, TIME or STD_MSG_POLL)"
    );

    for (std::string s : U::acceptedTypes)
    {
        /* If the given header is found in the accepted types, return true */
        if (s.compare(0, 3, header, 3, 3) == 0)
        {
            return true;
        }
    }

    return false;
}

template <typename T>
int8_t Sentence<T>::nmeaChecksum(const char *data)
{
    uint16_t i = 0;
    uint8_t check = 0;
    const char * checksumRegion = strchr(data, '$') + 1;
    const char * checksumPos = strchr(data, '*');
    if (checksumPos != NULL)
    {
        while(checksumRegion[i] != '*')
        {
            check ^= (uint8_t) checksumRegion[i];
            i++;
        }

        if (check == (uint8_t) strtol(checksumPos + 1, NULL, 16))
        {
            return 0;
        }
    }

    return -1;
}

template <typename T>
Sentence<T>::~Sentence()
{
    delete this->sentence;
}

/* ---------------------- End Sentence Definitions ---------------------- */