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
template <typename T> class Sentence;
class BASE
{
    public:
    bool getIsValid();
};
class STD_MSG_POLL;
class POS;
class TIME;
class DTM;
class GAQ;
class GBQ;
class GBS;
class GGA;
class GLL;
class GLQ;
class GNQ;
class GNS;
class GPQ;
class GRS;
class GSA;
class GST;
class GSV;
class RLM;
class RMC;
class TXT;
class VLW;
class VTG;
class ZDA;


const std::string BASE::acceptedTypes[] = {
    "DTM", "GAQ", "GBQ", "GBS", "GGA", "GLL", "GLQ", "GNQ", "GNS", "GPQ",
    "GRS", "GSA", "GST", "GSV", "RLM", "RMC", "TXT", "VLW", "VTG", "ZDA"
};
const std::string STD_MSG_POLL::acceptedTypes[] = {"GAQ", "GBQ", "GLQ", "GNQ", "GPQ"};
const std::string POS::acceptedTypes[] = {"DTM", "GGA", "GLL", "GNS", "GPQ", "RMC"};
const std::string TIME::acceptedTypes[] = {"GBS", "GGA", "GLL", "GNS", "GRS", "GST", "RLM", "RMC", "ZDA"};
const std::string DTM::acceptedTypes[] = {"DTM"};
const std::string GAQ::acceptedTypes[] = {"GAQ"};
const std::string GBQ::acceptedTypes[] = {"GBQ"};
const std::string GBS::acceptedTypes[] = {"GBS"};
const std::string GGA::acceptedTypes[] = {"GGA"};
const std::string GLL::acceptedTypes[] = {"GLL"};
const std::string GLQ::acceptedTypes[] = {"GLQ"};
const std::string GNQ::acceptedTypes[] = {"GNQ"};
const std::string GNS::acceptedTypes[] = {"GNS"};
const std::string GPQ::acceptedTypes[] = {"GPQ"};
const std::string GRS::acceptedTypes[] = {"GRS"};
const std::string GSA::acceptedTypes[] = {"GSA"};
const std::string GST::acceptedTypes[] = {"GST"};
const std::string GSV::acceptedTypes[] = {"GSV"};
const std::string RLM::acceptedTypes[] = {"RLM"};
const std::string RMC::acceptedTypes[] = {"RMC"};
const std::string TXT::acceptedTypes[] = {"TXT"};
const std::string VLW::acceptedTypes[] = {"VLW"};
const std::string VTG::acceptedTypes[] = {"VTG"};
const std::string ZDA::acceptedTypes[] = {"ZDA"};

#endif