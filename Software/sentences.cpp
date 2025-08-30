#include "sentences.hpp"

class GNSS
{
    Sentence<void> * sentences;
};

template <typename T>
class Sentence
{
    public:
    Sentence(char * line)
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
                    this->sentence = new T(lineArr, length);
                    this->sentence->initialise(lineArr, length);
    
                    /* If the sentence has invalid data, set it to NULL */
                    if (!this->sentence->getIsValid())
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
    
    private:
    T * sentence = NULL;

    public:
    T * const getSentence()
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
    private:
    static int8_t verifyFormat(const char *data)
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

    template<typename U>
    static bool isAcceptedSubtype(char * header)
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

    public:
    static int8_t nmeaChecksum(const char *data)
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

    public:
    ~Sentence()
    {
        delete this->sentence;
    }
};

/**
 * The base data class for all sentences
 */
class BASE
{
    private:
    uint16_t arrLength = 0;

    public:
    static const std::string acceptedTypes[];

    /* Assumes that the sentence is of the required format */
    public:
    BASE(char ** lineArr, uint16_t length)
    {
    }

    virtual void initialise(char ** lineArr, uint16_t length)
    {
        this->parseNMEA(lineArr, length);
        this->isValid = this->checkValidity();
    }
    
    protected:
    BASE(char * header, uint8_t checksum)
    {
        this->header = header;
        this->checksum = checksum;
    }
    
    protected:
    bool isValid = false;
    std::string header;
    Constellation constellation = INVALID;
    uint8_t checksum = 0;

    /* Ensure that the provided sentence is of the required type. */
    virtual bool verifyType(char ** lineArr, uint16_t length)
    {
        uint8_t minLength, maxLength;
        this->getSentenceBounds(&minLength, &maxLength);

        /* If outside the sentence bounds, it is no longer correct */
        if (length < minLength || length > maxLength)
            return false;

        /* If the header is not the same as the current type, return false */
        if (lineArr[0])
            return false;

        return true;
    }

    /**
     * Checks whether the current sentence is valid.
     * 
     * @returns `true` if valid and `false` if invalid.
     */
    virtual bool checkValidity()
    {
        return this->constellation != INVALID;
    }

    virtual void parseNMEA(char ** lineArr, uint16_t length)
    {
        this->header = std::string(lineArr[0]);
        this->constellation = convertConstellation(lineArr[0]);
        this->checksum = strtol(strchr(lineArr[length-1], '*') + 1, NULL, 16);
    }

    virtual void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
    {
        *minLength = 2;
        *maxLength = 23;
    }

    public:
    bool getIsValid()
    {
        return this->isValid;
    }

    virtual ~BASE()
    {

    }
};

/**
 * The base for polling a standard message
 */
class STD_MSG_POLL
{
    char * msgId;

    public:
    static const std::string acceptedTypes[];
};

/**
 * A message that contains position data (not including altitude)
 */
class POS
{
    protected:
    float_t lat = 0;
    char NS = '\0';
    float_t lon = 0;
    char EW = '\0';

    public:
    static const std::string acceptedTypes[];

    public:
    /* Returns the latitude (positive if north, negative if south) */
    const float_t getLatitude()
    {
        return this->lat * (this->NS == 'N' ? 1 : -1);
    }

    /* Returns the longitude (positive if east, negative is west) */
    const float_t getLongitude()
    {
        return this->lon * (this->EW == 'E' ? 1 : -1);
    }

    POS * const getPosition()
    {
        return (POS *) this;
    }
};

/**
 * A message that contains time data
 */
class TIME
{
    protected:
    std::string time;

    public:
    static const std::string acceptedTypes[];

    public:
    const std::string getTime()
    {
        return time;
    }
};

/**
 * The class for the Datum reference sentence
 */
class DTM : public BASE, public POS
{
    std::string datum;
    std::string subDatum;
    float_t alt = 0;
    std::string refDatum;
 
    static const uint8_t nFields = 11;

    public:
    static const std::string acceptedTypes[];

    public:
    DTM(char ** lineArr, uint16_t length) : BASE(lineArr, length)
    {

    }

    protected:
    bool checkValidity() override
    {
        bool valid = BASE::checkValidity();

        if (this->NS != 'N' && this->NS != 'S')
            valid = false;
        
        if (this->EW != 'E' && this->EW !='W')
            valid = false;

        if (this->refDatum.compare("W84") != 0)
            valid = false;
        
        return valid;
    }

    void parseNMEA(char ** lineArr, uint16_t length) override
    {
        BASE::parseNMEA(lineArr, length);

        this->datum = std::string(lineArr[0]);
        this->subDatum = std::string(lineArr[1]);
        this->lat = strtof(lineArr[2], NULL);
        this->NS = *lineArr[3];
        this->lon = strtof(lineArr[4], NULL);
        this->EW = *lineArr[5];
        this->alt = strtof(lineArr[6], NULL);
        this->refDatum = std::string(lineArr[7]);
    }
};

/**
 * The class for polling a standard message (Talker ID: GA)
 */
class GAQ : public BASE, public STD_MSG_POLL
{
    static const uint8_t nFields = 4;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for polling a standard message (Talker ID: GB)
 */
class GBQ : public BASE, public STD_MSG_POLL
{
    static const uint8_t nFields = 4;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for satellite fault detection
 */
class GBS : public BASE, public TIME
{
    float_t errLat;
    float_t errLon;
    float_t errAlt;
    uint8_t svid;
    uint8_t prob;   /* Unsupported */
    float_t bias;
    float_t stddev;
    uint8_t systemId;
    uint8_t signalId;

    static const uint8_t nFields = 13;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for global positioning system fix data
 */
class GGA : public BASE, public POS, public TIME
{
    uint8_t quality;
    uint8_t numSV;
    float_t HDOP;
    float_t alt;
    char altUnit;
    float_t sep;
    char sepUnit;
    uint16_t diffAge;
    uint16_t diffStation;

    static const uint8_t nFields = 17;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for latitude and longitude, with time of position fix and status
 */
class GLL : public BASE, public POS, public TIME
{
    char status = '\0';
    char posMode = '\0';

    static const uint8_t nFields = 10;

    public:
    static const std::string acceptedTypes[];

    public:
    GLL(char ** lineArr, uint16_t length) : BASE(lineArr, length)
    {

    }

    bool checkValidity() override
    {
        bool valid = BASE::checkValidity();

        if (this->status != 'A')
            valid = false;

        if (this->NS != 'N' && this->NS != 'S')
            valid = false;
        
        if (this->EW != 'E' && this->EW !='W')
            valid = false;
        
        return valid;
    }

    void parseNMEA(char ** lineArr, uint16_t length) override
    {
        BASE::parseNMEA(lineArr, length);

        this->lat = strtof(lineArr[1], NULL);
        this->NS = *lineArr[2];
        this->lon = strtof(lineArr[3], NULL);
        this->EW = *lineArr[4];
        this->time = std::string(lineArr[5]);
        this->status = *lineArr[6];
        this->posMode = *lineArr[7];
    }

    public:
    const char getStatus()
    {
        return this->status;
    }

    const char getPosMode()
    {
        return this->posMode;
    }
};

/**
 * The class for polling a standard message (Talker ID: GL)
 */
class GLQ : public BASE, public STD_MSG_POLL
{
    static const uint8_t nFields = 4;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for polling a standard message (Talker ID: GN)
 */
class GNQ : public BASE, public STD_MSG_POLL
{
    static const uint8_t nFields = 4;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting GNSS fix data
 */
class GNS : public BASE, public POS, public TIME
{
    std::string posMode;
    uint8_t numSV;
    float_t HDOP;
    float_t alt;
    float_t sep;
    uint16_t diffAge;
    uint16_t diffStation;
    char navStatus;

    static const uint8_t nFields = 16;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for polling a standard message (Talker ID: GP)
 */
class GPQ : public BASE, public STD_MSG_POLL
{
    static const uint8_t nFields = 4;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting GNSS range residuals
 */
class GRS : public BASE, public TIME
{
    uint8_t mode;
    float_t residual[12];
    uint8_t systemId;
    uint8_t singalId;

    static const uint8_t nFields = 19;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting GNSS DOP and active satellites
 */
class GSA : public BASE
{
    char opMode;
    uint8_t navMode;
    uint8_t svid[12];
    float_t PDOP;
    float_t HDOP;
    float_t VDOP;
    uint8_t systemId;

    static const uint8_t nFields = 21;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting GNSS pseudorange error statistics
 */
class GST : public BASE, public TIME
{
    float_t rangeRms;
    float_t stdMajor;
    float_t stdMinor;
    float_t orient;
    float_t stdLat;
    float_t stdLong;
    float_t stdAlt;

    static const uint8_t nFields = 11;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting the GNSS satellites in view
 */
class GSV : public BASE
{
    uint8_t numMsg;
    uint8_t msgNum;
    uint8_t numSV;
    SatData satellites[4]; /* Note: can appear up to 4 times, not always 4. */
    uint8_t signalId;

    static const uint8_t nFields_min = 7 + 1*4;
    static const uint8_t nFields_max = 7 + 4*4;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for a return link message
 */
class RLM : public BASE, public TIME
{
    uint64_t beacon;
    char code;
    uint64_t body; /* Check that the value cannot exceed 64bit */

    static const uint8_t nFields = 7;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting the recommended minimum data
 */
class RMC : public BASE, public POS, public TIME
{
    char status;
    float_t spd;
    float_t cog;
    std::string date;
    float_t mv;
    char mvEW;
    char posMode;
    char navStatus;

   static const uint8_t nFields = 16; 

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting text transmission
 */
class TXT : public BASE
{
    uint8_t numMsg;
    uint8_t msgNum;
    uint8_t msgType;
    std::string text;

    static const uint8_t nFields = 7;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting both ground/water distance
 */
class VLW : public BASE
{
    uint8_t twd; /* Fixed field: null */
    char twdUnit;
    uint8_t wd; /* Fixed field: null */
    char wdUnit;
    float_t tgd;
    char tgdUnit;
    float_t gd;
    char gdUnit;

    static const uint8_t nFields = 11;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting the course over ground and ground speed
 */
class VTG : public BASE
{
    float_t cogt;
    char cogtUnit; /* Fixed field: T */
    float_t cogm;
    char cogmUnit; /* Fixed field: M */
    float_t sogn;
    char sogkUnit; /* Fixed field: N */
    char posMode;

    static const uint8_t nFields = 12;

    public:
    static const std::string acceptedTypes[];
};

/**
 * The class for getting the time and date
 */
class ZDA : public BASE, public TIME
{
    uint8_t dat;
    uint8_t month;
    uint16_t year;
    uint8_t ltzh; /* Fixed field: 00 */
    uint8_t ltzn; /* Fixed field: 00 */

    static const uint8_t nFields = 9;

    public:
    static const std::string acceptedTypes[];
};