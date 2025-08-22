#include "sentences.hpp"

class GNSS
{
    Sentence<void> * sentences;

    /**
     * Verifies the format of the given NMEA message using regex.
     * 
     * @param data The message to check for the correct format
     * 
     * @returns 0 if the message is of the correct format, -1 if it is not
     * 			and -2 for any regex errors.
     */
    int8_t verifyFormat(const char *data)
    {
        static regex_t expr;
        int8_t res;

        /* Compile the regex to match the format of an NMEA message (note this matches
        both messages with \r\n at the end and without where a message always contains \r\n)*/
        res = regcomp(&expr, "^\\$.{2}.{3},+.*\\*.{2}(?:\r\n)?$", REG_NOSUB);

        if (res != 0)
        {
            return -2;
        }

        res = regexec(&expr, data, 0, NULL, 0);

        if (res == 0)
        {
            return 0;
        }

        return -1;
    }
};

template <typename T>
class Sentence
{
    public:
    Sentence(char ** lineArr, uint16_t length)
    {
        static_assert(std::is_base_of_v<BASE, T>, "Ensure that the given type is a BASE type!");

        this->sentence = new T(lineArr, length);

        /* If the sentence is invalid, set it to NULL */
        if (!this->sentence->getIsValid())
        {
            delete this->sentence;
            this->sentence = NULL;
        }
    }
    
    private:
    const T * sentence;

    public:
    const T * getSentence()
    {
        return this->sentence;
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
    BASE(char * line) : BASE(splitString(line, ",", &arrLength), arrLength)
    {   
    }

    /* Assumes that the sentence is of the required format */
    public:
    BASE(char ** lineArr, uint16_t length)
    {
        this->header = lineArr[0];
        this->checksum = strtol(strchr(lineArr[length-1], '*') + 1, NULL, 16);
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
    char * header;
    uint8_t checksum;

    /**
     * Checks whether the current sentence is valid.
     * 
     * @returns `true` if valid and `false` if invalid.
     */
    virtual bool checkValidity()
    {
        bool validity = false;

        /* TODO: Add validity logic here */

        return validity;
    }

    virtual void parseNMEA()
    {

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
};

/**
 * A message that contains position data (not including altitude)
 */
class POS
{
    float_t lat;
    char NS;
    float_t lon;
    char EW;
};

/**
 * A message that contains time data
 */
class TIME
{
    char * time;
};

/**
 * The class for the Datum reference sentence
 */
class DTM : BASE, POS
{
    char * datum;
    char * subDatum;
    float_t alt;
    char * refDatum;
};

/**
 * The class for polling a standard message (Talker ID: GA)
 */
class GAQ : BASE, STD_MSG_POLL
{
};

/**
 * The class for polling a standard message (Talker ID: GB)
 */
class GBQ : BASE, STD_MSG_POLL
{
};

/**
 * The class for satellite fault detection
 */
class GBS : BASE, TIME
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
};

/**
 * The class for global positioning system fix data
 */
class GGA : BASE, POS, TIME
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
};

/**
 * The class for latitude and longitude, with time of position fix and status
 */
class GLL : BASE, POS, TIME
{
    char status;
    char posMode;
};

/**
 * The class for polling a standard message (Talker ID: GL)
 */
class GLQ : BASE, STD_MSG_POLL
{
};

/**
 * The class for polling a standard message (Talker ID: GN)
 */
class GNQ : BASE, STD_MSG_POLL
{
};

/**
 * The class for getting GNSS fix data
 */
class GNS : BASE, POS, TIME
{
    char * posMode;
    uint8_t numSV;
    float_t HDOP;
    float_t alt;
    float_t sep;
    uint16_t diffAge;
    uint16_t diffStation;
    char navStatus;
};

/**
 * The class for polling a standard message (Talker ID: GP)
 */
class GPQ : BASE, STD_MSG_POLL
{
};

/**
 * The class for getting GNSS range residuals
 */
class GRS : BASE, TIME
{
    uint8_t mode;
    float_t residual[12];
    uint8_t systemId;
    uint8_t singalId;
};

/**
 * The class for getting GNSS DOP and active satellites
 */
class GSA : BASE
{
    char opMode;
    uint8_t navMode;
    uint8_t svid[12];
    float_t PDOP;
    float_t HDOP;
    float_t VDOP;
    uint8_t systemId;
};

/**
 * The class for getting GNSS pseudorange error statistics
 */
class GST : BASE, TIME
{
    float_t rangeRms;
    float_t stdMajor;
    float_t stdMinor;
    float_t orient;
    float_t stdLat;
    float_t stdLong;
    float_t stdAlt;
};

/**
 * The class for getting the GNSS satellites in view
 */
class GSV : BASE
{
    uint8_t numMsg;
    uint8_t msgNum;
    uint8_t numSV;
    SatData satellites[4]; /* Note: can appear up to 4 times, not always 4. */
    uint8_t signalId;
};

/**
 * The class for a return link message
 */
class RLM : BASE, TIME
{
    uint64_t beacon;
    char code;
    uint64_t body; /* Check that the value cannot exceed 64bit */
};

/**
 * The class for getting the recommended minimum data
 */
class RMC : BASE, POS, TIME
{
    char status;
    float_t spd;
    float_t cog;
    char * date;
    float_t mv;
    char mvEW;
    char posMode;
    char navStatus;
};

/**
 * The class for getting text transmission
 */
class TXT : BASE
{
    uint8_t numMsg;
    uint8_t msgNum;
    uint8_t msgType;
    char * text;
};

/**
 * The class for getting both ground/water distance
 */
class VLW : BASE
{
    uint8_t twd; /* Fixed field: null */
    char twdUnit;
    uint8_t wd; /* Fixed field: null */
    char wdUnit;
    float_t tgd;
    char tgdUnit;
    float_t gd;
    char gdUnit;
};

/**
 * The class for getting the course over ground and ground speed
 */
class VTG : BASE
{
    float_t cogt;
    char cogtUnit; /* Fixed field: T */
    float_t cogm;
    char cogmUnit; /* Fixed field: M */
    float_t sogn;
    char sogkUnit; /* Fixed field: N */
    char posMode;
};

/**
 * The class for getting the time and date
 */
class ZDA : BASE, TIME
{
    uint8_t dat;
    uint8_t month;
    uint16_t year;
    uint8_t ltzh; /* Fixed field: 00 */
    uint8_t ltzn; /* Fixed field: 00 */
};