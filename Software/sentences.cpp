#include "sentences.hpp"

const std::vector<std::string> BASE::acceptedTypes = {
    "DTM", "GAQ", "GBQ", "GBS", "GGA", "GLL", "GLQ", "GNQ", "GNS", "GPQ",
    "GRS", "GSA", "GST", "GSV", "RLM", "RMC", "TXT", "VLW", "VTG", "ZDA"
};
const std::vector<std::string> STD_MSG_POLL::acceptedTypes = {"GAQ", "GBQ", "GLQ", "GNQ", "GPQ"};
const std::vector<std::string> POS::acceptedTypes = {"DTM", "GGA", "GLL", "GNS", "GPQ", "RMC"};
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

class GNSS
{
    Sentence<void> * sentences;
};


/* -------------------------- BASE Definitions -------------------------- */

BASE::BASE(char ** lineArr, uint16_t length)
{

}

void BASE::initialise(char ** lineArr, uint16_t length)
{
    this->parseNMEA(lineArr, length);
    this->verifyBounds(length);
    this->isValid = this->checkValidity();
}

/* Ensure that the provided sentence within the required size. */
void BASE::verifyBounds(uint16_t length)
{
    std::invalid_argument err("The given sentence length is not within the acceptable bounds.");
    uint8_t minLength, maxLength;
    this->getSentenceBounds(&minLength, &maxLength);

    /* If outside the sentence bounds, it is no longer correct */
    if (length < minLength || length > maxLength)
        throw err;
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

void BASE::parseNMEA(char ** lineArr, uint16_t length)
{
    this->header = std::string(lineArr[0]);
    this->constellation = convertConstellation(lineArr[0]);
    this->checksum = strtol(strchr(lineArr[length-1], '*') + 1, NULL, 16);
}
    
void BASE::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 2;
    *maxLength = 23;
}

bool BASE::getIsValid()
{
    return this->isValid;
}

Constellation BASE::getConstellation()
{
    return this->constellation;
}

BASE::~BASE()
{

}

/* ------------------------ END BASE Definitions ------------------------ */


/* -------------------------- POS Definitions --------------------------- */

bool POS::checkValidity()
{
    bool valid = true;

    if (this->NS != 'N' && this->NS != 'S')
            valid = false;
        
    if (this->EW != 'E' && this->EW !='W')
        valid = false;
        
    return valid;
}

void POS::parseNMEA(char * lat, char * NS, char * lon, char * EW)
{
    this->lat = degMin2DecDeg(std::stof(lat));
    this->NS = *NS;
    this->lon = degMin2DecDeg(std::stof(lon));
    this->EW = *EW;
}

/* Returns the latitude (positive if north, negative if south) */
const float_t POS::getLatitude()
{
    return this->lat * (this->NS == 'N' ? 1 : -1);
}

/* Returns the longitude (positive if east, negative is west) */
const float_t POS::getLongitude()
{
    return this->lon * (this->EW == 'E' ? 1 : -1);
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

    deg = (int16_t) coords / 100;
    min = abs(coords) - abs(deg * 100);

    degrees += deg;
    degrees += min / 60.0;

    return degrees;
}

/* ------------------------- END POS Definitions ------------------------ */


/* -------------------------- TIME Definitions -------------------------- */

const std::string TIME::getTime()
{
    return time;
}
    
/* ------------------------ END TIME Definitions ------------------------ */


/* -------------------------- DTM Definitions --------------------------- */

DTM::DTM(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool DTM::checkValidity()
{
    bool valid = BASE::checkValidity();
    valid = valid && POS::checkValidity();

    if (this->refDatum.compare("W84") != 0)
        valid = false;
    
    return valid;
}

void DTM::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    POS::parseNMEA(lineArr[2], lineArr[3], lineArr[4], lineArr[5]);

    this->datum = std::string(lineArr[0]);
    this->subDatum = std::string(lineArr[1]);
    this->alt = std::stof(lineArr[6], NULL);
    this->refDatum = std::string(lineArr[7]);
}

std::string DTM::getDatum()
{
    return this->datum;
}

std::string DTM::getSubDatum()
{
    return this->subDatum;
}

float_t DTM::getAltitude()
{
    return this->alt;
}

std::string DTM::getReferenceDatum()
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

    this->time = std::string(lineArr[1]);
    this->errLat = std::stof(lineArr[2]);
    this->errLon = std::stof(lineArr[3]);
    this->errAlt = std::stof(lineArr[4]);
    this->svid = std::stoi(lineArr[5]);
    this->prob = 255; /* Unsupported value (it is always fixed) */
    this->bias = std::stof(lineArr[7]);
    this->stddev = std::stof(lineArr[8]);
    this->systemId = std::stoi(lineArr[9]);
    this->signalId = std::stoi(lineArr[10]);
}

float_t GBS::getErrLat()
{
    return this->errLat;
}

float_t GBS::getErrLon()
{
    return this->errLon;
}

float_t GBS::getErrAlt()
{
    return this->errAlt;
}

uint8_t GBS::getSVID()
{
    return this->svid;
}

uint8_t GBS::getProb()
{
    return this->prob;
}

float_t GBS::getBias()
{
    return this->bias;
}

float_t GBS::getStdDeviation()
{
    return this->stddev;
}

uint8_t GBS::getSystemId()
{
    return this->systemId;
}

uint8_t GBS::getSignalId()
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
    valid = valid && POS::checkValidity();
    
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
    POS::parseNMEA(lineArr[2], lineArr[3], lineArr[4], lineArr[5]);

    this->time = std::string(lineArr[1]);
    this->quality = std::stoi(lineArr[6]);
    this->numSV = std::stoi(lineArr[7]);
    this->HDOP = std::stof(lineArr[8]);
    this->alt = std::stof(lineArr[9]);
    this->altUnit = *lineArr[10];
    this->sep = std::stof(lineArr[11]);
    this->sepUnit = *lineArr[12];
    this->diffAge = std::stoi(lineArr[13]);
    this->diffStation = std::stoi(lineArr[14]);
}

uint8_t GGA::getQuality()
{
    return this->quality;
}

uint8_t GGA::getNumSatellites()
{
    return this->numSV;
}

float_t GGA::getHDOP()
{
    return this->HDOP;
}

float_t GGA::getAltitude()
{
    return this->alt;
}

char GGA::getAltitudeUnit()
{
    return this->altUnit;
}

float_t GGA::getGEOIDSep()
{
    return this->sep;
}

char GGA::getGEOIDSepUnit()
{
    return this->sepUnit;
}

uint16_t GGA::getDiffAge()
{
    return this->diffAge;
}

uint16_t GGA::getDiffStationID()
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

    this->time = std::string(lineArr[5]);
    this->status = *lineArr[6];
    this->posMode = *lineArr[7];
}

const char GLL::getStatus()
{
    return this->status;
}

const char GLL::getPosMode()
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
    valid = valid && POS::checkValidity();
    
    if (this->navStatus != 'V') /* Fixed field as hardware is not providing nav status info */
        valid = false;

    return valid;
}

void GNS::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);
    POS::parseNMEA(lineArr[2], lineArr[3], lineArr[4], lineArr[5]);

    this->time = std::string(lineArr[1]);
    this->posMode = std::string(lineArr[6]);
    this->numSV = std::stoi(lineArr[7]);
    this->HDOP = std::stof(lineArr[8]);
    this->alt = std::stof(lineArr[9]);
    this->sep = std::stof(lineArr[10]);
    this->diffAge = std::stoi(lineArr[11]);
    this->diffStation = std::stoi(lineArr[12]);
    this->navStatus = *lineArr[13];
}

std::string GNS::getPosMode()
{
    return this->posMode;
}

uint8_t GNS::getNumSV()
{
    return this->numSV;
}

float_t GNS::getHDOP()
{
    return this->HDOP;
}

float_t GNS::getAltitude()
{
    return this->alt;
}

float_t GNS::getGEOIDSep()
{
    return this->sep;
}

uint16_t GNS::getDiffAge()
{
    return this->diffAge;
}

uint16_t GNS::getDiffStationID()
{
    return this->diffStation;
}

char GNS::getNavStatus()
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

    this->time = std::string(lineArr[1]);
    this->mode = std::stoi(lineArr[2]);
    
    for (i = 0; i < 12; i++)
    {
        this->residual[i] = std::stof(lineArr[3 + i]);
    }

    this->systemId = std::stoi(lineArr[15]);
    this->singalId = std::stoi(lineArr[16]);
}

uint8_t GRS::getComputationMethod()
{
    return this->mode;
}

const float_t * const GRS::getResiduals()
{
    return this->residual;
}

uint8_t GRS::getSystemId()
{
    return this->systemId;
}

uint8_t GRS::getSingalId()
{
    return this->singalId;
}

void GRS::getSentenceBounds(uint8_t * minLength, uint8_t * maxLength)
{
    *minLength = 19;
    *maxLength = 19;
}

/* ------------------------- END GRS Definitions ------------------------ */