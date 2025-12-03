#include "sentences.hpp"

const std::vector<std::string> BASE::acceptedTypes = {
    "DTM", "GAQ", "GBQ", "GBS", "GGA", "GLL", "GLQ", "GNQ", "GNS", "GPQ",
    "GRS", "GSA", "GST", "GSV", "RLM", "RMC", "TXT", "VLW", "VTG", "ZDA"
};
const std::vector<std::string> STD_MSG_POLL::acceptedTypes = {"GAQ", "GBQ", "GLQ", "GNQ", "GPQ"};
const std::vector<std::string> POS::acceptedTypes = {"DTM", "GGA", "GLL", "GNS", "RMC"};
const std::vector<std::string> ALTITUDE::acceptedTypes = {"DTM", "GGA", "GNS"};
const std::vector<std::string> POS3D::acceptedTypes = {"DTM", "GGA", "GNS"};
const std::vector<std::string> TIME::acceptedTypes = {"GBS", "GGA", "GLL", "GNS", "GRS", "GST", "RLM", "RMC", "ZDA"};
const std::vector<std::string> DTM::acceptedTypes = {"DTM"};
const std::vector<std::string> GAQ::acceptedTypes = {"GAQ"};
const std::vector<std::string> GBQ::acceptedTypes = {"GBQ"};
const std::vector<std::string> GBS::acceptedTypes = {"GBS"};
const std::vector<std::string> GGA::acceptedTypes = {"GGA"};
const std::vector<std::string> GLL::acceptedTypes = {"GLL"};
const std::vector<std::string> GLQ::acceptedTypes = {"GLQ"};
const std::vector<std::string> GNQ::acceptedTypes = {"GNQ"};
const std::vector<std::string> GNS::acceptedTypes = {"GNS"};
const std::vector<std::string> GPQ::acceptedTypes = {"GPQ"};
const std::vector<std::string> GRS::acceptedTypes = {"GRS"};
const std::vector<std::string> GSA::acceptedTypes = {"GSA"};
const std::vector<std::string> GST::acceptedTypes = {"GST"};
const std::vector<std::string> GSV::acceptedTypes = {"GSV"};
const std::vector<std::string> RLM::acceptedTypes = {"RLM"};
const std::vector<std::string> RMC::acceptedTypes = {"RMC"};
const std::vector<std::string> TXT::acceptedTypes = {"TXT"};
const std::vector<std::string> VLW::acceptedTypes = {"VLW"};
const std::vector<std::string> VTG::acceptedTypes = {"VTG"};
const std::vector<std::string> ZDA::acceptedTypes = {"ZDA"};

/* -------------------------- BASE Definitions -------------------------- */

BASE::BASE(char ** lineArr, uint16_t length)
{

}

/**
 * Initialises the sentence data using the specific override functions implemented by the individual
 * sentence classes.
 * 
 * @param lineArr The array of the string sentence split at each ',' character.
 * @param length The length of the lineArr.
 * 
 * @returns `true` if the object has been successfully initialised, `false` otherwise.
 */
bool BASE::initialise(char ** lineArr, uint16_t length)
{
    /* 
     * Number of fields is length + 2 as the checksum and end delimiter are considered 
     * as fields, but are not considered in the length of the array 
     */
    if(this->verifyBounds(length + 2))  // This ensures parseNMEA does not overstep array bounds
    {
        this->parseNMEA(lineArr, length);
        return this->checkValidity();
    }
    else
    {
        return false;
    }
}

/**
 * Ensure that the provided sentence is within the required size.
 * 
 * @param nFields The number of fields that the sentence currently has.
 * 
 * @returns `true` if the number of fields is in the required bounds and `false` otherwise.
 */
bool BASE::verifyBounds(uint16_t nFields)
{
    std::invalid_argument err("The given sentence length is not within the acceptable bounds.");
    uint8_t minLength, maxLength;
    this->getSentenceBounds(&minLength, &maxLength);

    /* If outside the sentence bounds, it is no longer correct */
    if (nFields < minLength || nFields > maxLength)
        return false;

    return true;
}

/**
 * Checks whether the current sentence is valid.
 * 
 * @returns `true` if valid and `false` if invalid.
 */
bool BASE::checkValidity()
{
    return this->constellation != INVALID;
}

/**
 * Reads the NMEA data and assigns the respective array data to the class fields.
 */
void BASE::parseNMEA(char ** lineArr, uint16_t length)
{
    this->header = std::string(lineArr[0]);
    this->constellation = convertConstellation(lineArr[0]);
    this->checksum = strtol(strchr(lineArr[length-1], '*') + 1, NULL, 16);
}

/**
 * Returns the acceptable size of the array.
 */
void BASE::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 2;
    *maxLength = 23;
}

Constellation BASE::getConstellation()
{
    return this->constellation;
}

BASE::~BASE()
{

}

/* ------------------------ END BASE Definitions ------------------------ */


/* ------------------------- GROUP Definitions -------------------------- */
/* ----------------------- END GROUP Definitions ------------------------ */


/* -------------------------- POS Definitions --------------------------- */

POS::POS(POS& pos)
{
    this->lat = pos.lat;
    this->NS = pos.NS;
    this->lon = pos.lon;
    this->EW = pos.EW;
}

POS::POS()
{
    
}

bool POS::checkValidity()
{
    bool valid = true;

    if (this->NS != 'N' && this->NS != 'S')
        valid = false;
        
    if (this->EW != 'E' && this->EW != 'W')
        valid = false;
        
    return valid;
}

void POS::parseNMEA(char * lat, char * NS, char * lon, char * EW)
{
    strtofloat(lat, this->lat);
    this->lat.apply(degMin2DecDeg);
    this->NS.setValue(*NS, *NS == 'N' || *NS == 'S');
    strtofloat(lon, this->lon);
    this->lon.apply(degMin2DecDeg);
    this->EW.setValue(*EW, *EW == 'E' || *EW == 'W');
}

/* Returns the latitude (positive if north, negative if south) */
Field<float_t> POS::getLatitude()
{
    Field<float_t> lat(this->lat);

    if (lat.getValid() && this->NS.getValid())
    {
        lat.setValue(this->lat * (this->NS == 'N' ? 1 : -1), true);
    }

    return lat;
}

/* Returns the longitude (positive if east, negative is west) */
Field<float_t> POS::getLongitude()
{
    Field<float_t> lon(this->lon);

    if (lon.getValid() && this->EW.getValid())
    {
        lon.setValue(this->lon * (this->EW == 'E' ? 1 : -1), true);
    }

    return lon;
}

POS * const POS::getPosition()
{
    return (POS *) this;
}

/**
 * Returns the given longitude/latitude value in decimal degrees.
 * 
 * @param coords The longitude/latitude value in degrees, minutes and decimal minutes.
 *               This must be of the format: (d)ddmm.mmmmm
 *               -> Where (d) is an optional term and can either be 0 or 1
 * 
 * @returns The given value converted to decimal degrees format.
 * 
 * @note This assumes that the input is between 180 and -180 degrees inclusive.
 */
float_t POS::degMin2DecDeg(float_t coords)
{
    float_t degrees = 0;
    int8_t deg = 0;
    float_t min = 0;

    deg = (int32_t) coords / 100;
    min = fabs(coords) - abs(deg * 100);

    // Calculate the total degrees (without sign so they can add nicely together)
    degrees += abs(deg);
    degrees += min / 60.0;

    // Reapply sign to final degrees value
    degrees *= deg < 0 ? -1 : 1;

    return (float_t) degrees;
}

/* ------------------------- END POS Definitions ------------------------ */


/* ------------------------ ALTITUDE Definitions ------------------------ */

ALTITUDE::ALTITUDE(ALTITUDE& alt)
{
    this->alt = alt.alt;
}

ALTITUDE::ALTITUDE()
{

}

Field<float_t> ALTITUDE::getAltitude()
{
    return this->alt;
}

bool ALTITUDE::checkValidity()
{
    return true;
}

void ALTITUDE::parseNMEA(char * alt)
{
    strtofloat(alt, this->alt);
}

/* ---------------------- END ALTITUDE Definitions ---------------------- */


/* ------------------------- POS3D Definitions -------------------------- */

POS3D::POS3D(POS3D& pos) : POS(pos), ALTITUDE(pos)
{

}

POS3D::POS3D() : POS(), ALTITUDE()
{

}

POS3D * const POS3D::get3DPosition()
{
    return (POS3D *) this;
}

bool POS3D::checkValidity()
{
    bool valid = true;

    valid &= POS::checkValidity();
    valid &= ALTITUDE::checkValidity();

    return valid;
}

void POS3D::parseNMEA(char * lat, char * NS, char * lon, char * EW, char * alt)
{
    POS::parseNMEA(lat, NS, lon, EW);
    ALTITUDE::parseNMEA(alt);
}

/* ----------------------- END POS3D Definitions ------------------------ */


/* -------------------------- TIME Definitions -------------------------- */

TIME::TIME(TIME& time)
{
    this->time = time.time;
}

TIME::TIME()
{

}

Field<std::string> TIME::getTime()
{
    return this->time;
}

bool TIME::checkTimeFormat(char * time)
{
    if (strlen(time) != 9)
    {
        return false;
    }

    if (time[6] != '.')
    {
        return false;
    }

    if (!isdigit(time[0]) || !isdigit(time[1]))
    {
        return false;
    }

    if (!isdigit(time[7]) || !isdigit(time[8]))
    {
        return false;
    }

    if (!isdigit(time[3]) || !isdigit(time[5]))
    {
        return false;
    }

    if (time[2] < '1' || time[2] > '5')
    {
        return false;
    }

    if (time[4] < '1' || time[4] > '5')
    {
        return false;
    }

    return true;
}

void TIME::parseNMEA(char * time)
{
    this->time.setValue(std::string(time), this->checkTimeFormat(time));
}   

/* ------------------------ END TIME Definitions ------------------------ */


/* -------------------------- DTM Definitions --------------------------- */

DTM::DTM(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool DTM::checkValidity()
{
    bool valid = BASE::checkValidity();
    valid = valid && POS3D::checkValidity();

    if (this->refDatum == "W84")
        valid = false;
    
    return valid;
}

void DTM::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    POS3D::parseNMEA(lineArr[2], lineArr[3], lineArr[4], lineArr[5], lineArr[6]);

    this->datum.setValue(std::string(lineArr[0]), true);
    this->subDatum.setValue(std::string(lineArr[1]), true);
    this->refDatum.setValue(std::string(lineArr[7]), true);
}

Field<std::string> DTM::getDatum()
{
    return this->datum;
}

Field<std::string> DTM::getSubDatum()
{
    return this->subDatum;
}

Field<std::string> DTM::getReferenceDatum()
{
    return this->refDatum;
}

void DTM::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 11;
    *maxLength = 11;
}

/* ------------------------- END DTM Definitions ------------------------ */


/* -------------------------- GBS Definitions --------------------------- */

GBS::GBS(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GBS::checkValidity()
{
    bool valid = BASE::checkValidity();
    
    return valid;
}

void GBS::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[1]);

    strtofloat(lineArr[2], this->errLat);
    strtofloat(lineArr[3], this->errLon);
    strtofloat(lineArr[4], this->errAlt);
    strtouint8(lineArr[5], this->svid);
    this->prob.setValue(255, false); /* Unsupported value (it is always fixed) */
    strtofloat(lineArr[7], this->bias);
    strtofloat(lineArr[8], this->stddev);
    strtouint8(lineArr[9], this->systemId);
    strtouint8(lineArr[10], this->signalId);
}

Field<float_t> GBS::getErrLat()
{
    return this->errLat;
}

Field<float_t> GBS::getErrLon()
{
    return this->errLon;
}

Field<float_t> GBS::getErrAlt()
{
    return this->errAlt;
}

Field<uint8_t> GBS::getSVID()
{
    return this->svid;
}

Field<uint8_t> GBS::getProb()
{
    return this->prob;
}

Field<float_t> GBS::getBias()
{
    return this->bias;
}

Field<float_t> GBS::getStdDeviation()
{
    return this->stddev;
}

Field<uint8_t> GBS::getSystemId()
{
    return this->systemId;
}

Field<uint8_t> GBS::getSignalId()
{
    return this->signalId;
}

void GBS::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 13;
    *maxLength = 13;
}

/* ------------------------- END GBS Definitions ------------------------ */


/* -------------------------- GGA Definitions --------------------------- */

GGA::GGA(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GGA::checkValidity()
{
    bool valid = BASE::checkValidity();
    valid = valid && POS3D::checkValidity();
    
    if (this->quality == 0)
        valid = false;

    if (this->altUnit != 'M')   /* Fixed field (ie. metres) */
        valid = false;

    if (this->sepUnit != 'M')   /* Fixed field (ie. metres) */
        valid = false;

    return valid;
}

void GGA::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[1]);
    POS3D::parseNMEA(lineArr[2], lineArr[3], lineArr[4], lineArr[5], lineArr[9]);

    strtouint8(lineArr[6], this->quality);
    strtouint8(lineArr[7], this->numSV);
    strtofloat(lineArr[8], this->HDOP);
    this->altUnit.setValue(*lineArr[10], *lineArr[10] == 'M');
    strtofloat(lineArr[11], this->sep);
    this->sepUnit.setValue(*lineArr[12], *lineArr[12] == 'M');
    strtouint16(lineArr[13], this->diffAge);
    strtouint16(lineArr[14], this->diffStation);
}

Field<uint8_t> GGA::getQuality()
{
    return this->quality;
}

Field<uint8_t> GGA::getNumSatellites()
{
    return this->numSV;
}

Field<float_t> GGA::getHDOP()
{
    return this->HDOP;
}

Field<char> GGA::getAltitudeUnit()
{
    return this->altUnit;
}

Field<float_t> GGA::getGEOIDSep()
{
    return this->sep;
}

Field<char> GGA::getGEOIDSepUnit()
{
    return this->sepUnit;
}

Field<uint16_t> GGA::getDiffAge()
{
    return this->diffAge;
}

Field<uint16_t> GGA::getDiffStationID()
{
    return this->diffStation;
}

void GGA::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 17;
    *maxLength = 17;
}

/* ------------------------- END GGA Definitions ------------------------ */


/* -------------------------- GLL Definitions --------------------------- */

GLL::GLL(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GLL::checkValidity()
{
    bool valid = BASE::checkValidity();
    valid = valid && POS::checkValidity();

    if (this->status != 'A')
        valid = false;
    
    return valid;
}

void GLL::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    POS::parseNMEA(lineArr[1], lineArr[2], lineArr[3], lineArr[4]);
    TIME::parseNMEA(lineArr[5]);

    this->status.setValue(*lineArr[6], true);
    this->posMode.setValue(*lineArr[7], true);
}

Field<char> GLL::getStatus()
{
    return this->status;
}

Field<char> GLL::getPosMode()
{
    return this->posMode;
}

void GLL::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 10;
    *maxLength = 10;
}

/* ------------------------- END GLL Definitions ------------------------ */


/* -------------------------- GNS Definitions --------------------------- */

GNS::GNS(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GNS::checkValidity()
{
    bool valid = BASE::checkValidity();
    valid = valid && POS3D::checkValidity();
    
    if (this->navStatus != 'V') /* Fixed field as hardware is not providing nav status info */
        valid = false;

    return valid;
}

void GNS::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[1]);
    POS3D::parseNMEA(lineArr[2], lineArr[3], lineArr[4], lineArr[5], lineArr[9]);

    this->posMode.setValue(std::string(lineArr[6]), true);
    strtouint8(lineArr[7], this->numSV);
    strtofloat(lineArr[8], this->HDOP);
    strtofloat(lineArr[10], this->sep);
    strtouint16(lineArr[11], this->diffAge);
    strtouint16(lineArr[12], this->diffStation);
    this->navStatus.setValue(*lineArr[13], true);
}

Field<std::string> GNS::getPosMode()
{
    return this->posMode;
}

Field<uint8_t> GNS::getNumSV()
{
    return this->numSV;
}

Field<float_t> GNS::getHDOP()
{
    return this->HDOP;
}

Field<float_t> GNS::getGEOIDSep()
{
    return this->sep;
}

Field<uint16_t> GNS::getDiffAge()
{
    return this->diffAge;
}

Field<uint16_t> GNS::getDiffStationID()
{
    return this->diffStation;
}

Field<char> GNS::getNavStatus()
{
    return this->navStatus;
}

void GNS::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 16;
    *maxLength = 16;
}

/* ------------------------- END GNS Definitions ------------------------ */


/* -------------------------- GRS Definitions --------------------------- */

GRS::GRS(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GRS::checkValidity()
{
    bool valid = BASE::checkValidity();

    return valid;
}

void GRS::parseNMEA(char ** lineArr, uint16_t length)
{
    uint8_t i;

    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[1]);

    strtouint8(lineArr[2], this->mode);
    
    for (i = 0; i < 12; i++)
    {
        strtofloat(lineArr[3 + i], this->residual[i]);
    }

    strtouint8(lineArr[15], this->systemId);
    strtouint8(lineArr[16], this->signalId);
}

Field<uint8_t> GRS::getComputationMethod()
{
    return this->mode;
}

const Field<float_t> * const GRS::getResiduals()
{
    return this->residual;
}

Field<uint8_t> GRS::getSystemId()
{
    return this->systemId;
}

Field<uint8_t> GRS::getSingalId()
{
    return this->signalId;
}

void GRS::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 19;
    *maxLength = 19;
}

/* ------------------------- END GRS Definitions ------------------------ */


/* -------------------------- GSA Definitions --------------------------- */

GSA::GSA(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GSA::checkValidity()
{
    bool valid = BASE::checkValidity();

    if (this->navMode == 1) /* There was no fix for this sentence */
        valid = false;

    return valid;
}

void GSA::parseNMEA(char ** lineArr, uint16_t length)
{
    uint8_t i;

    BASE::parseNMEA(lineArr, length);

    this->opMode.setValue(*lineArr[1], true);
    strtouint8(lineArr[2], this->navMode);

    for (i = 0; i < 12; i++)
    {
        strtouint8(lineArr[3 + i], this->svid[i]);
    }

    strtofloat(lineArr[15], this->PDOP);
    strtofloat(lineArr[16], this->HDOP);
    strtofloat(lineArr[17], this->VDOP);
    strtouint8(lineArr[18], this->systemId);
}

Field<char> GSA::getOpMode()
{
    return this->opMode;
}

Field<uint8_t> GSA::getNavMode()
{
    return this->navMode;
}

const Field<uint8_t> * const GSA::getSVID()
{
    return this->svid;
}

Field<float_t> GSA::getPDOP()
{
    return this->PDOP;
}

Field<float_t> GSA::getHDOP()
{
    return this->HDOP;
}

Field<float_t> GSA::getVDOP()
{
    return this->VDOP;
}

Field<uint8_t> GSA::getSystemId()
{
    return this->systemId;
}

void GSA::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 21;
    *maxLength = 21;
}

/* ------------------------- END GSA Definitions ------------------------ */


/* -------------------------- GST Definitions --------------------------- */

GST::GST(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GST::checkValidity()
{
    bool valid = BASE::checkValidity();

    return valid;
}

void GST::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[1]);

    strtofloat(lineArr[2], this->rangeRms);
    strtofloat(lineArr[3], this->stdMajor);
    strtofloat(lineArr[4], this->stdMinor);
    strtofloat(lineArr[5], this->orient);
    strtofloat(lineArr[6], this->stdLat);
    strtofloat(lineArr[7], this->stdLong);
    strtofloat(lineArr[8], this->stdAlt);
}

Field<float_t> GST::getRangeRMS()
{
    return this->rangeRms;
}

Field<float_t> GST::getStdMajor()
{
    return this->stdMajor;
}

Field<float_t> GST::getStdMinor()
{
    return this->stdMinor;
}

Field<float_t> GST::getOrientation()
{
    return this->orient;
}

Field<float_t> GST::getStdLatitude()
{
    return this->stdLat;
}

Field<float_t> GST::getStdLongitude()
{
    return this->stdLong;
}

Field<float_t> GST::getStdAltitude()
{
    return this->stdAlt;
}

void GST::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 11;
    *maxLength = 11;
}

/* ------------------------- END GST Definitions ------------------------ */


/* -------------------------- GSV Definitions --------------------------- */

GSV::GSV(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GSV::checkValidity()
{
    bool valid = BASE::checkValidity();

    return valid;
}

void GSV::parseNMEA(char ** lineArr, uint16_t length)
{
    uint8_t i, nGroups;

    BASE::parseNMEA(lineArr, length);

    strtouint8(lineArr[1], this->numMsg);
    strtouint8(lineArr[2], this->msgNum);
    strtouint8(lineArr[3], this->numSV);

    /* Number of repeated groups = (total length - fixed length) / fields in group */
    nGroups = (length - 6) / 4;

    this->satellites = new Field<SatData>[nGroups];
    this->satellitesLength = nGroups;

    SatData tempData;

    for (i = 0; i < nGroups; i++)
    {
        bool valid = true;
        char * endPtr;

        tempData.svid = strtoul(lineArr[4 + 4*i], &endPtr, 10);
        valid &= *endPtr == '\0';   // Ensure that the entire integer was consumed

        tempData.elv = strtoul(lineArr[5 + 4*i], &endPtr, 10);
        valid &= *endPtr == '\0';   // Ensure that the entire integer was consumed

        tempData.az = strtoul(lineArr[6 + 4*i], &endPtr, 10);
        valid &= *endPtr == '\0';   // Ensure that the entire integer was consumed

        tempData.cno = strtoul(lineArr[7 + 4*i], &endPtr, 10);
        valid &= *endPtr == '\0';   // Ensure that the entire integer was consumed

        this->satellites[0].setValue(tempData, valid);
    }

    strtouint8(lineArr[4 + 4*nGroups], this->signalId);
}

Field<uint8_t> GSV::getNumMessages()
{
    return this->numMsg;
}

Field<uint8_t> GSV::getMessageNum()
{
    return this->msgNum;
}

Field<uint8_t> GSV::getNumSatellites()
{
    return this->numSV;
}

const Field<SatData> * const GSV::getSatellites(uint8_t * const arrLength)
{
    if (arrLength != NULL)
    {
        *arrLength = this->satellitesLength;
    }
    
    return this->satellites;
}

Field<uint8_t> GSV::getSignalId()
{
    return this->signalId;
}

void GSV::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 11;
    *maxLength = 23;
}

GSV::~GSV()
{
    BASE::~BASE();

    delete this->satellites;
}

/* ------------------------- END GSV Definitions ------------------------ */


/* -------------------------- RLM Definitions --------------------------- */

RLM::RLM(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool RLM::checkValidity()
{
    bool valid = BASE::checkValidity();

    return valid;
}

void RLM::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[2]);

    strtouint64(lineArr[1], this->beacon, 16);
    this->code.setValue(*lineArr[3], true);
    strtouint64(lineArr[1], this->beacon, 16);
}

Field<uint64_t> RLM::getBeacon()
{
    return this->beacon;
}

Field<char> RLM::getCode()
{
    return this->code;
}

Field<uint64_t> RLM::getBody()
{
    return this->body;
}

void RLM::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 7;
    *maxLength = 7;
}

/* ------------------------- END RLM Definitions ------------------------ */


/* -------------------------- RMC Definitions --------------------------- */

RMC::RMC(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool RMC::checkValidity()
{
    bool valid = BASE::checkValidity();
    valid = valid && POS::checkValidity();

    if (this->status != 'A')
        valid = false;
    
    if (this->posMode == 'N')
        valid = false;

    if (this->navStatus != 'V') /* Fixed field so should always be V */
        valid = false;

    return valid;
}

void RMC::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[1]);
    POS::parseNMEA(lineArr[3], lineArr[4], lineArr[5], lineArr[6]);

    this->status.setValue(*lineArr[2], true);
    strtofloat(lineArr[7], this->spd);
    strtofloat(lineArr[8], this->cog);
    this->date.setValue(std::string(lineArr[9]), true);
    strtofloat(lineArr[10], this->mv);
    this->mvEW.setValue(*lineArr[11], *lineArr[11] == 'E' || *lineArr[11] == 'W');
    this->posMode.setValue(*lineArr[12], true);
    this->navStatus.setValue(*lineArr[13], true);
}

Field<char> RMC::getStatus()
{
    return this->status;
}

Field<float_t> RMC::getSpeedOverGround()
{
    return this->spd;
}

Field<float_t> RMC::getCourseOverGround()
{
    return this->cog;
}

Field<std::string> RMC::getDate()
{
    return this->date;
}

Field<float_t> RMC::getMagneticVariation()
{
    return this->mv;
}

Field<char> RMC::getMagneticVariationDir()
{
    return this->mvEW;
}

Field<char> RMC::getPosMode()
{
    return this->posMode;
}

Field<char> RMC::getNavStatus()
{
    return this->navStatus;
}

void RMC::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 16;
    *maxLength = 16;
}

/* ------------------------- END RMC Definitions ------------------------ */


/* -------------------------- TXT Definitions --------------------------- */

TXT::TXT(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool TXT::checkValidity()
{
    bool valid = BASE::checkValidity();

    return valid;
}

void TXT::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);

    strtouint8(lineArr[1], this->numMsg);
    strtouint8(lineArr[2], this->msgNum);
    strtouint8(lineArr[3], this->msgType);
    this->text.setValue(std::string(lineArr[4]), true);
}

Field<uint8_t> TXT::getNumMessages()
{
    return this->numMsg;
}

Field<uint8_t> TXT::getMessageNum()
{
    return this->msgNum;
}

Field<uint8_t> TXT::getMessageType()
{
    return this->msgType;
}

Field<std::string> TXT::getText()
{
    return this->text;
}

void TXT::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 7;
    *maxLength = 7;
}

/* ------------------------- END TXT Definitions ------------------------ */


/* -------------------------- VLW Definitions --------------------------- */

VLW::VLW(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool VLW::checkValidity()
{
    bool valid = BASE::checkValidity();

    if (this->twdUnit != 'N')
        valid = false;

    if (this->wdUnit != 'N')
        valid = false;

    if (this->tgdUnit != 'N')
        valid = false;

    if (this->gdUnit != 'N')
        valid = false;

    return valid;
}

void VLW::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);

    this->twd.setValue(255, false); /* Fixed field: null */
    this->twdUnit.setValue(*lineArr[2], *lineArr[2] == 'N'); /* Fixed field: N */
    this->wd.setValue(255, false); /* Fixed field: null */
    this->wdUnit.setValue(*lineArr[4], *lineArr[4] == 'N'); /* Fixed field: N */
    strtofloat(lineArr[5], this->tgd);
    this->tgdUnit.setValue(*lineArr[6], *lineArr[6] == 'N'); /* Fixed field: N */
    strtofloat(lineArr[7], this->gd);
    this->gdUnit.setValue(*lineArr[8], *lineArr[8] == 'N'); /* Fixed field: N */
}

Field<uint8_t> VLW::getTotalWaterDist()
{
    return this->twd;
}

Field<char> VLW::getTWDUnit()
{
    return this->twdUnit;
}

Field<uint8_t> VLW::getWaterDist()
{
    return this->wd;
}

Field<char> VLW::getWDUnit()
{
    return this->wdUnit;
}

Field<float_t> VLW::getTotalGroundDist()
{
    return this->tgd;
}

Field<char> VLW::getTGDUnit()
{
    return this->tgdUnit;
}

Field<float_t> VLW::getGroundDist()
{
    return this->gd;
}

Field<char> VLW::getGDUnit()
{
    return this->gdUnit;
}

void VLW::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 11;
    *maxLength = 11;
}

/* ------------------------- END VLW Definitions ------------------------ */


/* -------------------------- VTG Definitions --------------------------- */

VTG::VTG(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool VTG::checkValidity()
{
    bool valid = BASE::checkValidity();

    if (this->posMode == 'N')
        valid = false;

    if (this->cogtUnit != 'T')
        valid = false;

    if (this->cogmUnit != 'M')
        valid = false;

    if (this->sognUnit != 'N')
        valid = false;

    if (this->sogkUnit != 'K')
        valid = false;

    return valid;
}

void VTG::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);

    strtofloat(lineArr[1], this->cogt);
    this->cogtUnit.setValue(*lineArr[2], *lineArr[2] == 'T');
    strtofloat(lineArr[3], this->cogm);
    this->cogmUnit.setValue(*lineArr[4], *lineArr[4] == 'M');
    strtofloat(lineArr[5], this->sogn);
    this->sognUnit.setValue(*lineArr[6], *lineArr[6] == 'N');
    strtofloat(lineArr[7], this->sogk);
    this->sogkUnit.setValue(*lineArr[8], *lineArr[8] == 'K');
    this->posMode.setValue(*lineArr[9], true);
}

Field<float_t> VTG::getTrueCourseOverGround()
{
    return this->cogt;
}

Field<char> VTG::getTCOGUnit()
{
    return this->cogtUnit;
}

Field<float_t> VTG::getMagneticCourseOverGround()
{
    return this->cogm;
}

Field<char> VTG::getMCOGUnit()
{
    return this->cogmUnit;
}

Field<float_t> VTG::getSpeedOverGroundKnots()
{
    return this->sogn;
}

Field<char> VTG::getSOGNUnit()
{
    return this->sognUnit;
}

Field<float_t> VTG::getSpeedOverGroundKms()
{
    return this->sogk;
}

Field<char> VTG::getSOGKUnit()
{
    return this->sogkUnit;
}

Field<char> VTG::getPosMode()
{
    return this->posMode;
}

void VTG::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 12;
    *maxLength = 12;
}

/* ------------------------- END VTG Definitions ------------------------ */


/* -------------------------- ZDA Definitions --------------------------- */

ZDA::ZDA(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool ZDA::checkValidity()
{
    bool valid = BASE::checkValidity();

    if (this->day < 1 || this->day > 31)
        valid = false;

    if (this->month < 1 || this-> month > 12)
        valid = false;

    if (this->ltzh != 0)
        valid = false;

    if (this->ltzn != 0)
        valid = false;

    return valid;
}

void ZDA::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    TIME::parseNMEA(lineArr[1]);

    strtouint8(lineArr[2], this->day);
    strtouint8(lineArr[3], this->month);
    strtouint16(lineArr[4], this->year);
    strtouint8(lineArr[5], this->ltzh);
    strtouint8(lineArr[6], this->ltzn);
}

Field<uint8_t> ZDA::getDay()
{
    return this->day;
}

Field<uint8_t> ZDA::getMonth()
{
    return this->month;
}

Field<uint16_t> ZDA::getYear()
{
    return this->year;
}

Field<uint8_t> ZDA::getLocalTimezoneHrs()
{
    return this->ltzh;
}

Field<uint8_t> ZDA::getLocalTimezoneMins()
{
    return this->ltzn;
}

void ZDA::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 9;
    *maxLength = 9;
}

/* ------------------------- END ZDA Definitions ------------------------ */