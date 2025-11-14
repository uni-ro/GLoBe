/**
 * FILE: sentences.hpp
 * PURPOSE: The header file to declare all of the sentence structures for NMEA communication.
 * 
 * UPDATED: 13 Nov. 2025
 * 
 * NOTE: The NMEA sentences were implemented based on the definitions found in the interface description
 *       for the u-blox NeoM9N GNSS module. A link to this interface description can be found below.
 * 
 * REFERENCE: https://content.u-blox.com/sites/default/files/u-blox-M9-SPG-4.04_InterfaceDescription_UBX-21022436.pdf
 */

#ifndef INC_SENTENCES_HPP_
#define INC_SENTENCES_HPP_

#include <regex>
#include <string>
#include <vector>
#include <ctime>
#include <compare>

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "stringslib.h"
#include "stringslib.hpp"
#include "gnss.h"
#include "data_validation.hpp"

extern "C"
{
    /**
     * A (C-style) struct to represent satellite data for retrieving the satellites in view.
     * The C-type nature of the struct is to ensure efficient memory management as a full C++
     * struct is not needed.
     */
    typedef struct
    {
        uint8_t svid;   // Satellite ID
        uint8_t elv;    // Elevation
        uint16_t az;    // Azimuth
        uint8_t cno;    // Signal Strength
    } SatData;
}

/**
 * The template class to create and retrieve NMEA sentences from a given string. If the sentence in the
 * given string is of the same class as the provided template class then the sentence created is valid.
 * If the provided string is not the same as the provided template class, then the sentence is invalid
 * and the internal sentence value is `NULL`.
 */
template <typename T> class Sentence
{
    public:
    Sentence(char * line);
    T * const getSentence();

    private:
    T * sentence = NULL;

    inline void assertCorrectType();
    bool isAcceptedSubtype(char * header);
    
    /* -------------- Static Functions -------------- */
    private:
    static int8_t verifyFormat(const char * data);
    static T * getFromHeader(const char * const header, char ** lineArr, uint16_t length);

    public:
    static int8_t nmeaChecksum(const char * data);
    /* ------------ End Static Functions ------------ */

    public:
    ~Sentence();
};

/**
 * The base data class for all sentences. This contains the header, the constellation and the checksum
 * of the sentence. All of the valid NMEA sentences contain this info.
 */
class BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    BASE(char ** lineArr, uint16_t length);
    virtual bool initialise(char ** lineArr, uint16_t length);
    Constellation getConstellation();

    protected:
    std::string header;
    Constellation constellation = INVALID;
    uint8_t checksum = 0;
    bool verifyBounds(uint16_t nFields);
    virtual bool checkValidity();
    virtual void parseNMEA(char ** lineArr, uint16_t length);
    virtual void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength);

    public:
    virtual ~BASE();
};

/**
 * A base class for all of the sentence group types. This allows for easy identification of a `GROUP`
 * sub-class and, hence, the proper processing required for a `GROUP` child class.
 * 
 * @note All NMEA group sentences MUST inherit from this class to be appropriately considered as `GROUP`
 */
class GROUP
{
    
};

/**
 * The base for polling a standard message
 */
class STD_MSG_POLL : public GROUP
{
    public:
    STD_MSG_POLL(STD_MSG_POLL& msg);

    protected:
    STD_MSG_POLL();

    public:
    static const std::vector<std::string> acceptedTypes;

    private:
    char * msgId;
};

/**
 * A group that contains position data (not including altitude). The longitude and latitude is 
 * stored in degrees and minutes and are converted to decimal degrees when retrieving the latitude
 * and longitude values using the appropriate getters.
 */
class POS : public GROUP
{
    public:
    POS(POS& pos);

    protected:
    POS();

    public:
    static const std::vector<std::string> acceptedTypes;
    Field<float_t> getLatitude();
    Field<float_t> getLongitude();
    POS * const getPosition();

    static float_t degMin2DecDeg(float_t coords);

    protected:
    Field<float_t> lat;
    Field<char> NS;
    Field<float_t> lon;
    Field<char> EW;
    
    bool checkValidity();
    void parseNMEA(char * lat, char * NS, char * lon, char * EW);
};

/**
 * A group that contains only altitude information.
 */
class ALTITUDE : public GROUP
{
    public:
    ALTITUDE(ALTITUDE& alt);

    protected:
    ALTITUDE();

    public:
    static const std::vector<std::string> acceptedTypes;
    Field<float_t> getAltitude();

    protected:
    Field<float_t> alt;

    bool checkValidity();
    void parseNMEA(char * alt);
};

/**
 * A group that contains longitude, latitude and altitude information. Similar to the `POS` group, the
 * longitude and latitude values are stored as degrees and minutes and only converted to decimal
 * degrees when queried using the appropriate getters.
 */
class POS3D : public POS, public ALTITUDE
{
    public:
    POS3D(POS3D& pos);

    protected:
    POS3D();

    public:
    static const std::vector<std::string> acceptedTypes;
    POS3D * const get3DPosition();

    protected:
    bool checkValidity();
    void parseNMEA(char * lat, char * NS, char * lon, char * EW, char * alt);
};

/**
 * A group that contains only time data. The time data is stored as a string in the following format:
 *      "HHMMSS.SS", where
 *          HH -> The current hour
 *          MM -> The current minute
 *          SS.SS -> The current seconds
 */
class TIME : public GROUP
{
    public:
    TIME(TIME& time);

    protected:
    TIME();

    public:
    static const std::vector<std::string> acceptedTypes;
    Field<std::string> getTime();

    protected:
    Field<std::string> time{"000000.00"};

    bool checkTimeFormat(char * time);
    void parseNMEA(char * time);
};

/**
 * The class for the Datum reference sentence
 */
class DTM : public BASE, public POS3D
{
    public:
    static const std::vector<std::string> acceptedTypes;
    DTM(char ** lineArr, uint16_t length);

    Field<std::string> getDatum();
    Field<std::string> getSubDatum();
    Field<std::string> getReferenceDatum();

    private:
    Field<std::string> datum;
    Field<std::string> subDatum;
    Field<std::string> refDatum;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GA)
 * 
 * @note Currently not implemented as it is not required.
 */
class GAQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    protected:
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GB)
 * 
 * @note Currently not implemented as it is not required.
 */
class GBQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    protected:
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for satellite fault detection
 */
class GBS : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GBS(char ** lineArr, uint16_t length);
    Field<float_t> getErrLat();
    Field<float_t> getErrLon();
    Field<float_t> getErrAlt();
    Field<uint8_t> getSVID();
    Field<uint8_t> getProb();   /* Unsupported */
    Field<float_t> getBias();
    Field<float_t> getStdDeviation();
    Field<uint8_t> getSystemId();
    Field<uint8_t> getSignalId();


    private:
    Field<float_t> errLat;
    Field<float_t> errLon;
    Field<float_t> errAlt;
    Field<uint8_t> svid;
    Field<uint8_t> prob;   /* Unsupported */
    Field<float_t> bias;
    Field<float_t> stddev;
    Field<uint8_t> systemId;
    Field<uint8_t> signalId;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for global positioning system fix data
 */
class GGA : public BASE, public POS3D, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GGA(char ** lineArr, uint16_t length);
    Field<uint8_t> getQuality();
    Field<uint8_t> getNumSatellites();
    Field<float_t> getHDOP();
    Field<char> getAltitudeUnit();
    Field<float_t> getGEOIDSep();
    Field<char> getGEOIDSepUnit();
    Field<uint16_t> getDiffAge();
    Field<uint16_t> getDiffStationID();
    
    private:
    Field<uint8_t> quality;
    Field<uint8_t> numSV;
    Field<float_t> HDOP;
    Field<char> altUnit;
    Field<float_t> sep;
    Field<char> sepUnit;
    Field<uint16_t> diffAge;
    Field<uint16_t> diffStation;

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
    Field<char> getStatus();
    Field<char> getPosMode();

    private:
    Field<char> status;
    Field<char> posMode;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GL)
 * 
 * @note Currently not implemented as it is not required
 */
class GLQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    protected:
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GN)
 * 
 * @note Currently not implemented as it is not required
 */
class GNQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    protected:
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting GNSS fix data
 */
class GNS : public BASE, public POS3D, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GNS(char ** lineArr, uint16_t length);
    Field<std::string> getPosMode();
    Field<uint8_t> getNumSV();
    Field<float_t> getHDOP();
    Field<float_t> getGEOIDSep();
    Field<uint16_t> getDiffAge();
    Field<uint16_t> getDiffStationID();
    Field<char> getNavStatus();

    private:
    Field<std::string> posMode;
    Field<uint8_t> numSV;
    Field<float_t> HDOP;
    Field<float_t> sep;
    Field<uint16_t> diffAge;
    Field<uint16_t> diffStation;
    Field<char> navStatus;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for polling a standard message (Talker ID: GP)
 * 
 * @note Currently not implemented as it is not required
 */
class GPQ : public BASE, public STD_MSG_POLL
{
    public:
    static const std::vector<std::string> acceptedTypes;
    
    protected:
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting GNSS range residuals
 */
class GRS : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GRS(char ** lineArr, uint16_t length);
    Field<uint8_t> getComputationMethod();
    const Field<float_t> * const getResiduals();
    Field<uint8_t> getSystemId();
    Field<uint8_t> getSingalId();
    
    private:
    Field<uint8_t> mode;
    Field<float_t> residual[12];
    Field<uint8_t> systemId;
    Field<uint8_t> signalId;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting GNSS DOP (Dilution of Precision) and active satellites
 */
class GSA : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    GSA(char ** lineArr, uint16_t length);
    Field<char> getOpMode();
    Field<uint8_t> getNavMode();
    const Field<uint8_t> * const getSVID();
    Field<float_t> getPDOP();
    Field<float_t> getHDOP();
    Field<float_t> getVDOP();
    Field<uint8_t> getSystemId();

    private:
    Field<char> opMode;
    Field<uint8_t> navMode;
    Field<uint8_t> svid[12];
    Field<float_t> PDOP;    // Position Dilution of Precision
    Field<float_t> HDOP;    // Horizontal Dilution of Precision
    Field<float_t> VDOP;    // Vertical Dilution of Precision
    Field<uint8_t> systemId;

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
    Field<float_t> getRangeRMS();
    Field<float_t> getStdMajor();
    Field<float_t> getStdMinor();
    Field<float_t> getOrientation();
    Field<float_t> getStdLatitude();
    Field<float_t> getStdLongitude();
    Field<float_t> getStdAltitude();

    private:
    Field<float_t> rangeRms;
    Field<float_t> stdMajor;
    Field<float_t> stdMinor;
    Field<float_t> orient;
    Field<float_t> stdLat;
    Field<float_t> stdLong;
    Field<float_t> stdAlt;

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
    Field<uint8_t> getNumMessages();
    Field<uint8_t> getMessageNum();
    Field<uint8_t> getNumSatellites();
    const Field<SatData> * const getSatellites(uint8_t * const arrLength);
    Field<uint8_t> getSignalId();

    private:
    Field<uint8_t> numMsg;
    Field<uint8_t> msgNum;
    Field<uint8_t> numSV;
    Field<SatData> * satellites = NULL; /* Note: can appear up to 4 times, not always 4. */
    uint8_t satellitesLength = 0;
    Field<uint8_t> signalId;

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
    RLM(char ** lineArr, uint16_t length);
    Field<uint64_t> getBeacon();
    Field<char> getCode();
    Field<uint64_t> getBody();

    private:
    Field<uint64_t> beacon;
    Field<char> code;
    Field<uint64_t> body; /* This assumes that the value cannot exceed 64bit - this may need to be verified */

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting the recommended minimum data
 */
class RMC : public BASE, public POS, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    RMC(char ** lineArr, uint16_t length);
    Field<char> getStatus();
    Field<float_t> getSpeedOverGround();
    Field<float_t> getCourseOverGround();
    Field<std::string> getDate();
    Field<float_t> getMagneticVariation();
    Field<char> getMagneticVariationDir();
    Field<char> getPosMode();
    Field<char> getNavStatus();

    private:
    Field<char> status;
    Field<float_t> spd;
    Field<float_t> cog;
    Field<std::string> date;
    Field<float_t> mv;
    Field<char> mvEW;
    Field<char> posMode;
    Field<char> navStatus;

   protected:
   bool checkValidity() override;
   void parseNMEA(char ** lineArr, uint16_t length) override;
   void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting text transmission
 */
class TXT : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    TXT(char ** lineArr, uint16_t length);
    Field<uint8_t> getNumMessages();
    Field<uint8_t> getMessageNum();
    Field<uint8_t> getMessageType();
    Field<std::string> getText();

    private:
    Field<uint8_t> numMsg;
    Field<uint8_t> msgNum;
    Field<uint8_t> msgType;
    Field<std::string> text;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting both ground/water distance
 */
class VLW : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    VLW(char ** lineArr, uint16_t length);
    Field<uint8_t> getTotalWaterDist(); /* Fixed field: null */
    Field<char> getTWDUnit();
    Field<uint8_t> getWaterDist(); /* Fixed field: null */
    Field<char> getWDUnit();
    Field<float_t> getTotalGroundDist();
    Field<char> getTGDUnit();
    Field<float_t> getGroundDist();
    Field<char> getGDUnit();

    private:
    Field<uint8_t> twd; /* Fixed field: null */
    Field<char> twdUnit;
    Field<uint8_t> wd; /* Fixed field: null */
    Field<char> wdUnit;
    Field<float_t> tgd;
    Field<char> tgdUnit;
    Field<float_t> gd;
    Field<char> gdUnit;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting the course over ground and ground speed
 */
class VTG : public BASE
{
    public:
    static const std::vector<std::string> acceptedTypes;
    VTG(char ** lineArr, uint16_t length);
    Field<float_t> getTrueCourseOverGround();
    Field<char> getTCOGUnit();
    Field<float_t> getMagneticCourseOverGround();
    Field<char> getMCOGUnit();
    Field<float_t> getSpeedOverGroundKnots();
    Field<char> getSOGNUnit();
    Field<float_t> getSpeedOverGroundKms();
    Field<char> getSOGKUnit();
    Field<char> getPosMode();

    private:
    Field<float_t> cogt;
    Field<char> cogtUnit; /* Fixed field: T */
    Field<float_t> cogm;
    Field<char> cogmUnit; /* Fixed field: M */
    Field<float_t> sogn;
    Field<char> sognUnit; /* Fixed field: N */
    Field<float_t> sogk;
    Field<char> sogkUnit; /* Fixed field: K */
    Field<char> posMode;

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
};

/**
 * The class for getting the time and date
 */
class ZDA : public BASE, public TIME
{
    public:
    static const std::vector<std::string> acceptedTypes;
    ZDA(char ** lineArr, uint16_t length);
    Field<uint8_t> getDay();
    Field<uint8_t> getMonth();
    Field<uint16_t> getYear();
    Field<uint8_t> getLocalTimezoneHrs();
    Field<uint8_t> getLocalTimezoneMins();

    private:
    Field<uint8_t> day;
    Field<uint8_t> month;
    Field<uint16_t> year;
    Field<uint8_t> ltzh; /* Fixed field: 00 */
    Field<uint8_t> ltzn; /* Fixed field: 00 */

    protected:
    bool checkValidity() override;
    void parseNMEA(char ** lineArr, uint16_t length) override;
    void getSentenceBounds(uint8_t * minLength, uint8_t * maxLength) override;
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