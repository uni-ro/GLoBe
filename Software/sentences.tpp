/**
 * FILE: sentences.tpp
 * PURPOSE: To serve as the template implementation file for all of the sentences
 * 
 * UPDATED: 14 Nov. 2025
 * 
 * NOTE: Do NOT include this file other than at the end of sentences.hpp.
 *       Any other includes may lead to issues.
 */

/* ------------------------ Sentence Definitions ------------------------ */

/**
 * This creates a `Sentence` object from the given string. For the string to be considered a
 * valid sentence, it must begin with a '$' character and it must end with either "\r\n", or with
 * a string terminator ('\0'). If the given line contains an NMEA sentence that is of the same type
 * as the given type (`T`) for the `Sentence` template, then the sentence is created and can be found
 * as `sentence`, otherwise if it is not then the `sentence` field is `NULL`.
 * 
 * @param line The string from which to interpret the sentence.
 */
template <typename T>
Sentence<T>::Sentence(char * line)
{
    this->assertCorrectType();

    int8_t validFormat = -1;
    char ** lineArr = NULL;
    uint16_t length = 0;
    char delim[] = ",";

    /* Use REGEX matching to identify if the sentence is the correct format for an NMEA sentence */
    validFormat = Sentence::verifyFormat(line);

    /* Ensure that the format of the sentence is valid */
    if (validFormat == 0)
    {
        /* Ensure that the sentence has a correct checksum */
        if (Sentence::nmeaChecksum(line) == 0)
        {
            /* Split the sentence into its constituent parts and create an object from it */
            lineArr = splitString(line, delim, &length);
            
            /* If the sentence is the correct type, create it, otherwise set it to NULL */
            if(this->isAcceptedSubtype(lineArr[0]))
            {
                bool initialised = false;

                /* If the given type is a GROUP type, then use a group retrieving method to get the
                sentence value */
                if constexpr (std::is_base_of_v<GROUP, T> && !std::is_base_of_v<BASE, T>)
                {
                    this->sentence = this->getFromHeader(lineArr[0], lineArr, length);
                    initialised = this->sentence != NULL;
                }
                /* Otherwise, create the sentence as is */
                else
                {
                    this->sentence = new T(lineArr, length);
                    initialised = this->sentence->initialise(lineArr, length);
                }

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

/**
 * Retrieves the stored sentence object, or NULL if it has not been created yet.
 * 
 * @returns The sentence object if the given sentence is of the correct type, or NULL otherwise.
 */
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
    /* Compile the regex to match the format of an NMEA message (note this matches
    both messages with \r\n at the end and without whereas a message always contains \r\n) */
    static std::regex expr("^\\$.{2}.{3},+.*\\*.{2}(?:\r\n)?$", std::regex_constants::ECMAScript);
    bool res;

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

/**
 * This converts the given lineArr according to the given header to the type required by the `Sentence`
 * object. If the given sentence is not the required type, it will return `NULL`. This method also
 * casts the found sentence type to the required type as defined in the `Sentence` object.
 * 
 * For example:
 * If the given NMEA sentence is: $GNGLL,1234.56789,S,12345.67891,E,123456.78,A,D*66
 * And the sentence object is of the type: Sentence<POS>
 * Then the method will return a `POS` object with the data read in from the NMEA sentence.
 * ie.
 *      POS{1234.56789, S, 12345.67891, E}
 * 
 * @param header The header to use for comparisons. This must include the '$' symbol and the constellation.
 * @param lineArr The already-split array for the sentence.
 * @param length The length of the `lineArr`.
 * 
 * @returns A pointer to a sentence of the type `T` from `Sentence` if the given sentence is valid, or
 *          `NULL` if the given sentence is invalid.
 * 
 * @note This is mostly used to ensure that sentences can belong to a `GROUP` and be initialise as such
 * @note The header should be the entire header and must be completely accessible memory-wise
 * @note Any `STD_MSG_POLL` sentences are currently not considered
 */
template <typename T>
T * Sentence<T>::getFromHeader(const char * const header, char ** lineArr, uint16_t length)
{
    bool initialised = false;
    T * found = NULL;

    /* NOTE: Can replace with switch-case later, however a significant performance increase may not be observed */
    if (strncmp(header + 3, (char *) "DTM", 3) == 0)
    {
        if (std::is_base_of_v<T, DTM>)
        {
            DTM * sent = new DTM(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    /* else if (strncmp(header + 3, (char *) "GAQ", 3) == 0)
    {
        if (std::is_base_of_v<T, GAQ>)
        {
            GAQ * sent = new GAQ(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "GBQ", 3) == 0)
    {
        if (std::is_base_of_v<T, GBQ>)
        {
            GBQ * sent = new GBQ(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } */
    else if (strncmp(header + 3, (char *) "GBS", 3) == 0)
    {
        if (std::is_base_of_v<T, GBS>)
        {
            GBS * sent = new GBS(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "GGA", 3) == 0)
    {
        if (std::is_base_of_v<T, GGA>)
        {
            GGA * sent = new GGA(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "GLL", 3) == 0)
    {
        if (std::is_base_of_v<T, GLL>)
        {
            GLL * sent = new GLL(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    /* else if (strncmp(header + 3, (char *) "GLQ", 3) == 0)
    {
        if (std::is_base_of_v<T, GLQ>)
        {
            GLQ * sent = new GLQ(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "GNQ", 3) == 0)
    {
        if (std::is_base_of_v<T, GNQ>)
        {
            GNQ * sent = new GNQ(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } */
    else if (strncmp(header + 3, (char *) "GNS", 3) == 0)
    {
        if (std::is_base_of_v<T, GNS>)
        {
            GNS * sent = new GNS(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    /* else if (strncmp(header + 3, (char *) "GPQ", 3) == 0)
    {
        if (std::is_base_of_v<T, GPQ>)
        {
            GPQ * sent = new GPQ(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } */
    else if (strncmp(header + 3, (char *) "GRS", 3) == 0)
    {
        if (std::is_base_of_v<T, GRS>)
        {
            GRS * sent = new GRS(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "GSA", 3) == 0)
    {
        if (std::is_base_of_v<T, GSA>)
        {
            GSA * sent = new GSA(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "GST", 3) == 0)
    {
        if (std::is_base_of_v<T, GST>)
        {
            GST * sent = new GST(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "GSV", 3) == 0)
    {
        if (std::is_base_of_v<T, GSV>)
        {
            GSV * sent = new GSV(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "RLM", 3) == 0)
    {
        if (std::is_base_of_v<T, RLM>)
        {
            RLM * sent = new RLM(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "RMC", 3) == 0)
    {
        if (std::is_base_of_v<T, RMC>)
        {
            RMC * sent = new RMC(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "TXT", 3) == 0)
    {
        if (std::is_base_of_v<T, TXT>)
        {
            TXT * sent = new TXT(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "VLW", 3) == 0)
    {
        if (std::is_base_of_v<T, VLW>)
        {
            VLW * sent = new VLW(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "VTG", 3) == 0)
    {
        if (std::is_base_of_v<T, VTG>)
        {
            VTG * sent = new VTG(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    } 
    else if (strncmp(header + 3, (char *) "ZDA", 3) == 0)
    {
        if (std::is_base_of_v<T, ZDA>)
        {
            ZDA * sent = new ZDA(lineArr, length);
            initialised = sent->initialise(lineArr, length);
            found = new T(*((T *) sent));
            delete sent;
            sent = NULL;
        }
    }

    if (!initialised)
    {
        delete found;
        found = NULL;
    }

    return found;
}

/**
 * A method to statically assert that the `Sentence` `T` type is an accepted type. This `T` type must 
 * be either a child of a `BASE` class, or a child of a `GROUP` class.
 */
template <typename T>
inline void Sentence<T>::assertCorrectType()
{
    static_assert(
        std::is_base_of_v<BASE, T> ||
        std::is_base_of_v<GROUP, T>,
        "Ensure that the given type is an accepted type (BASE, or a GROUP type)"
    );
}

/**
 * A method to determine whether the given header can be found in the `Sentence` `T` type's accepted
 * types vector.
 * 
 * @param header The header to verify. This is the entire header including the '$' symbol and constellation
 * 
 * @returns `true` if the given header is in the `T::acceptedTypes` vector, or `false` otherwise
 * 
 * @note The type `T` must implement a static field that contains the accepted type headers.
 */
template <typename T>
bool Sentence<T>::isAcceptedSubtype(char * header)
{
    this->assertCorrectType();

    for (std::string s : T::acceptedTypes)
    {
        /* If the given header is found in the accepted types, return true */
        if (s.compare(0, 3, header, 3, 3) == 0)
        {
            return true;
        }
    }

    return false;
}

/**
 * A method to determine whether the checksum of the given sentence is valid or not. This follows the
 * NMEA checksum formula as described in the Interface Description of the NEOM9N Module.
 * 
 * @param data The NMEA sentence string, including the checksum (ie. from '$' to "\r\n")
 * 
 * @returns `0` if the calculated checksum from the given sentence equals the checksum at the end of the
 *          given sentence and `-1` otherwise.
 */
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

/**
 * The destructor for the `Sentence` class. This deletes the sentence if it was allocated and sets
 * the value to `NULL`
 */
template <typename T>
Sentence<T>::~Sentence()
{
    delete this->sentence;
    this->sentence = NULL;
}

/* ---------------------- End Sentence Definitions ---------------------- */