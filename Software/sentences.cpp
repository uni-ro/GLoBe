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
    this->isValid = this->checkValidity();
}

/* Ensure that the provided sentence is of the required type. */
bool BASE::verifyType(char ** lineArr, uint16_t length)
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

    if (this->NS != 'N' && this->NS != 'S')
        valid = false;
    
    if (this->EW != 'E' && this->EW !='W')
        valid = false;

    if (this->refDatum.compare("W84") != 0)
        valid = false;
    
    return valid;
}

void DTM::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);

    this->datum = std::string(lineArr[0]);
    this->subDatum = std::string(lineArr[1]);
    this->lat = degMin2DecDeg(std::stof(lineArr[2], NULL));
    this->NS = *lineArr[3];
    this->lon = degMin2DecDeg(std::stof(lineArr[4], NULL));
    this->EW = *lineArr[5];
    this->alt = std::stof(lineArr[6], NULL);
    this->refDatum = std::string(lineArr[7]);
}

/* ------------------------- END DTM Definitions ------------------------ */


/* -------------------------- GLL Definitions --------------------------- */

GLL::GLL(char ** lineArr, uint16_t length) : BASE(lineArr, length)
{

}

bool GLL::checkValidity()
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

void GLL::parseNMEA(char ** lineArr, uint16_t length)
{
    BASE::parseNMEA(lineArr, length);

    this->lat = degMin2DecDeg(std::stof(lineArr[1], NULL));
    this->NS = *lineArr[2];
    this->lon = degMin2DecDeg(std::stof(lineArr[3], NULL));
    this->EW = *lineArr[4];
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

/* ------------------------- END GLL Definitions ------------------------ */