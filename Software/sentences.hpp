#ifndef INC_SENTENCES_HPP_
#define INC_SENTENCES_HPP_

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <regex.h>
#include "stringslib.h"

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
class BASE;
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

#endif