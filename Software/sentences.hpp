#ifndef INC_SENTENCES_HPP_
#define INC_SENTENCES_HPP_

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <regex>
#include <string>
#include <vector>
#include "stringslib.h"
#include <ctime>
#include "gnss.h"

/**
 * The struct to represent satellite data for getting satellites in view
 */
typedef struct
{
    uint8_t svid;
    uint8_t elv; /* Double check - may be float? */
    uint16_t az; /* Double check - may be float? */
    uint8_t cno;
} SatData;

class GNSS;

template <typename T> class Sentence
{
    public:
    Sentence(char * line);
    T * const getSentence();

    private:
    T * sentence = NULL;
    
    /* -------------- Static Functions -------------- */
    private:
    static int8_t verifyFormat(const char * data);
    template <typename U> static bool isAcceptedSubtype(char * header);

    public:
    static int8_t nmeaChecksum(const char * data);
    /* ------------ End Static Functions ------------ */

    public:
    ~Sentence();
};

/**
 * The base data class for all sentences
 */
class BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    BASE(char ** lineArr, uint16_t length);
    virtual void initialise(char ** lineArr, uint16_t length);
    bool getIsValid();
    Constellation getConstellation();

    protected:
    bool isValid = false;
    std::string header;
    Constellation constellation = INVALID;
    uint8_t checksum = 0;
    void verifyBounds(uint16_t length);
    virtual bool checkValidity();
    virtual void parseNMEA(char ** lineArr, uint16_t length);
    virtual void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength);

    public:
    virtual ~BASE();
};

/**
 * The base for polling a standard message
 */
class STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    char * msgId;
};

/**
 * A message that contains position data (not including altitude)
 */
class POS
{
    public:
    static const std::vector<std::string> acceptedTypes;
    const float_t getLatitude();
    const float_t getLongitude();
    POS * const getPosition();

    static float_t degMin2DecDeg(float_t coords);

    protected:
    float_t lat = 0;
    char NS = '\0';
    float_t lon = 0;
    char EW = '\0';
    
    bool checkValidity();
    void parseNMEA(char * lat, char * NS, char * lon, char * EW);
};

/**
 * A message that contains time data
 */
class TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    const std::string getTime();

    protected:
    std::string time;
};

/**
 * The class for the Datum reference sentence
 */
class DTM : public BASE, public POS
{
    public:
    static const std::vector<std::string> acceptedTypes;
    DTM(char ** lineArr, uint16_t length);

    std::string getDatum();
    std::string getSubDatum();
    float_t getAltitude();
    std::string getReferenceDatum();

    private:
    std::string datum;
    std::string subDatum;
    float_t alt = 0;
    std::string refDatum;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GA)
 */
class GAQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    private:
    static const uint8_t nFields = 4;
};

/**
 * The class for polling a standard message (Talker ID: GB)
 */
class GBQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    private:
    static const uint8_t nFields = 4;
};

/**
 * The class for satellite fault detection
 */
class GBS : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GBS(char ** lineArr, uint16_t length);
    float_t getErrLat();
    float_t getErrLon();
    float_t getErrAlt();
    uint8_t getSVID();
    uint8_t getProb();   /* Unsupported */
    float_t getBias();
    float_t getStdDeviation();
    uint8_t getSystemId();
    uint8_t getSignalId();


    private:
    float_t errLat;
    float_t errLon;
    float_t errAlt;
    uint8_t svid;
    uint8_t prob;   /* Unsupported */
    float_t bias;
    float_t stddev;
    uint8_t systemId;
    uint8_t signalId;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for global positioning system fix data
 */
class GGA : public BASE, public POS, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GGA(char ** lineArr, uint16_t length);
    uint8_t getQuality();
    uint8_t getNumSatellites();
    float_t getHDOP();
    float_t getAltitude();
    char getAltitudeUnit();
    float_t getGEOIDSep();
    char getGEOIDSepUnit();
    uint16_t getDiffAge();
    uint16_t getDiffStationID();
    
    private:
    uint8_t quality;
    uint8_t numSV;
    float_t HDOP;
    float_t alt;
    char altUnit;
    float_t sep;
    char sepUnit;
    uint16_t diffAge;
    uint16_t diffStation;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for latitude and longitude, with time of position fix and status
 */
class GLL : public BASE, public POS, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GLL(char ** lineArr, uint16_t length);
    const char getStatus();
    const char getPosMode();

    private:
    char status = '\0';
    char posMode = '\0';

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GL)
 */
class GLQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    private:
    static const uint8_t nFields = 4;
};

/**
 * The class for polling a standard message (Talker ID: GN)
 */
class GNQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    private:
    static const uint8_t nFields = 4;
};

/**
 * The class for getting GNSS fix data
 */
class GNS : public BASE, public POS, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GNS(char ** lineArr, uint16_t length);
    std::string getPosMode();
    uint8_t getNumSV();
    float_t getHDOP();
    float_t getAltitude();
    float_t getGEOIDSep();
    uint16_t getDiffAge();
    uint16_t getDiffStationID();
    char getNavStatus();

    private:
    std::string posMode;
    uint8_t numSV;
    float_t HDOP;
    float_t alt;
    float_t sep;
    uint16_t diffAge;
    uint16_t diffStation;
    char navStatus;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GP)
 */
class GPQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    private:
    static const uint8_t nFields = 4;
};

/**
 * The class for getting GNSS range residuals
 */
class GRS : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GRS(char ** lineArr, uint16_t length);
    uint8_t getComputationMethod();
    const float_t * const getResiduals();
    uint8_t getSystemId();
    uint8_t getSingalId();
    
    private:
    uint8_t mode;
    float_t residual[12];
    uint8_t systemId;
    uint8_t singalId;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting GNSS DOP and active satellites
 */
class GSA : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GSA(char ** lineArr, uint16_t length);
    char getOpMode();
    uint8_t getNavMode();
    const uint8_t * const getSVID();
    float_t getPDOP();
    float_t getHDOP();
    float_t getVDOP();
    uint8_t getSystemId();

    private:
    char opMode;
    uint8_t navMode;
    uint8_t svid[12];
    float_t PDOP;
    float_t HDOP;
    float_t VDOP;
    uint8_t systemId;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting GNSS pseudorange error statistics
 */
class GST : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GST(char ** lineArr, uint16_t length);
    float_t getRangeRMS();
    float_t getStdMajor();
    float_t getStdMinor();
    float_t getOrientation();
    float_t getStdLatitude();
    float_t getStdLongitude();
    float_t getStdAltitude();

    private:
    float_t rangeRms;
    float_t stdMajor;
    float_t stdMinor;
    float_t orient;
    float_t stdLat;
    float_t stdLong;
    float_t stdAlt;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting the GNSS satellites in view
 */
class GSV : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GSV(char ** lineArr, uint16_t length);
    uint8_t getNumMessages();
    uint8_t getMessageNum();
    uint8_t getNumSatellites();
    const SatData * const getSatellites(uint8_t * const arrLength);
    uint8_t getSignalId();

    private:
    uint8_t numMsg;
    uint8_t msgNum;
    uint8_t numSV;
    SatData * satellites = NULL; /* Note: can appear up to 4 times, not always 4. */
    uint8_t satellitesLength;
    uint8_t signalId;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override; 
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;

    public:
    ~GSV() override;
};

/**
 * The class for a return link message
 */
class RLM : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    uint64_t beacon;
    char code;
    uint64_t body; /* Check that the value cannot exceed 64bit */

    static const uint8_t nFields = 7;
};

/**
 * The class for getting the recommended minimum data
 */
class RMC : public BASE, public POS, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    char status;
    float_t spd;
    float_t cog;
    std::string date;
    float_t mv;
    char mvEW;
    char posMode;
    char navStatus;

   static const uint8_t nFields = 16; 
};

/**
 * The class for getting text transmission
 */
class TXT : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    uint8_t numMsg;
    uint8_t msgNum;
    uint8_t msgType;
    std::string text;

    static const uint8_t nFields = 7;
};

/**
 * The class for getting both ground/water distance
 */
class VLW : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    uint8_t twd; /* Fixed field: null */
    char twdUnit;
    uint8_t wd; /* Fixed field: null */
    char wdUnit;
    float_t tgd;
    char tgdUnit;
    float_t gd;
    char gdUnit;

    static const uint8_t nFields = 11;
};

/**
 * The class for getting the course over ground and ground speed
 */
class VTG : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    float_t cogt;
    char cogtUnit; /* Fixed field: T */
    float_t cogm;
    char cogmUnit; /* Fixed field: M */
    float_t sogn;
    char sogkUnit; /* Fixed field: N */
    char posMode;

    static const uint8_t nFields = 12;
};

/**
 * The class for getting the time and date
 */
class ZDA : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    uint8_t dat;
    uint8_t month;
    uint16_t year;
    uint8_t ltzh; /* Fixed field: 00 */
    uint8_t ltzn; /* Fixed field: 00 */

    static const uint8_t nFields = 9;
};

/* Include the template implementation after declaration
 * NOTE: This is done as templates must either be fully defined in the header
 *       or have specific implementations specified. To circumvent this, a file
 *       with the template implementation can be included in the header to include
 *       the implementation. This reduces the code in the header file.
 * NOTE: Do NOT include sentences.tpp at the beginning of this file or at any point
 *       in other header files.
 * REFERENCE: https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
 * REFERENCE: https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
 */
#include "sentences.tpp"

#endif