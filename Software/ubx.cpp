#include "ubx.hpp"

namespace CFG
{
    LAYER getLayer(uint8_t layer)
    {
        switch(layer)
        {
            case 0:
                return RAM;
            case 1:
                return BBR;
            case 2:
                return FLASH;
            case 7: default:
                return DEFAULT;
        }
    }
}

namespace UBX_DTYPES
{
    //TODO: Make these functions less repetitive.
    // NOTE: Assumes arr is initialised and memory is accessible
    void splitInt(uint16_t value, uint8_t * arr)
    {
        arr[0] = (uint8_t) (value >> 8);
        arr[1] = (uint8_t) value;
    }

    // NOTE: Assumes arr is initialised and memory is accessible
    void splitInt(uint32_t value, uint8_t * arr)
    {
        uint8_t i;

        for (i = 0; i < 4; i++)
        {
            arr[i] = (uint8_t) (value >> 8 * (3 - i));
        }
    }

    // NOTE: Assumes arr is initialised and memory is accessible
    void splitInt(uint64_t value, uint8_t * arr)
    {
        uint8_t i;

        for (i = 0; i < 8; i++)
        {
            arr[i] = (uint8_t) (value >> 8 * (7 - i));
        }
    }


    uint16_t convertU2(const uint8_t * const littleEndian)
    {
        return littleEndian[1] | ((uint16_t) littleEndian[0] << 8);
    };

    uint16_t convertU2(const uint16_t littleEndian)
    {
        return (littleEndian << 8) | (littleEndian >> 8);
    };

    uint32_t convertU4(const uint8_t * const littleEndian)
    {
        uint32_t converted = 0;
        uint8_t i;

        for (i = 0; i < 4; i++)
        {
            converted |= (uint32_t) littleEndian[i] << (i * 8);
        }

        return converted;
    };

    uint32_t convertU4(const uint32_t littleEndian)
    {
        uint8_t arr[4] = {};
        
        splitInt(littleEndian, arr);

        uint32_t converted = convertU4(arr);

        return converted;
    };

    uint64_t convertU8(const uint8_t * const littleEndian)
    {
        uint64_t converted = 0;
        uint8_t i;

        for (i = 0; i < 8; i++)
        {
            converted |= (uint64_t) littleEndian[i] << (i * 8);
        }

        return converted;
    };

    uint64_t convertU8(const uint64_t littleEndian)
    {
        uint8_t arr[8] = {};
        
        splitInt(littleEndian, arr);

        uint32_t converted = convertU8(arr);

        return converted;
    };
};


CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint8_t value)
{
    this->key = key;
    this->value.B1 = value;
    this->dtype = UBX_DTYPES::DTYPES::U1;
}

CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint16_t value)
{
    this->key = key;
    this->value.B2 = value;
    this->dtype = UBX_DTYPES::DTYPES::U2;
}

CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint32_t value)
{
    this->key = key;
    this->value.B4 = value;
    this->dtype = UBX_DTYPES::DTYPES::U4;
}

CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint64_t value)
{
    this->key = key;
    this->value.B8 = value;
    this->dtype = UBX_DTYPES::DTYPES::U8;
}

// NOTE: Assumes that all memory is initialised and accessible.
// NOTE: Assumes that all provided keys are actual keys
CFGData::CFGData(uint8_t * bytes, uint16_t nBytes)
{
    std::vector<CFGDataPair> pairs;
    uint16_t i = 0;
    uint8_t * currKey = bytes;
    CFG::KEYS key;
    uint8_t nValueBytes;

    while(i < nBytes)
    {
        uint64_t value;
        currKey = bytes + i;

        key = (CFG::KEYS) UBX_DTYPES::convertU4(currKey);

        // Mask the length of the value (Bits 30..28)
        nValueBytes = ((key >> 24) & 0b01110000) >> 4;

        switch(nValueBytes)
        {
            default:
                // The size was not a valid size, so exit the function as the data may not be aligned.
                return;

            case 0x01: case 0x02:
                value = currKey[4];
            
                pairs.push_back({key, (uint8_t) value});

                i += 5;

                break;

            case 0x03:
                value = UBX_DTYPES::convertU2(currKey + 4);

                pairs.push_back({key, (uint16_t) value});

                i += 6;

                break;

            case 0x04:
                value = UBX_DTYPES::convertU4(currKey + 4);
                
                pairs.push_back({key, (uint32_t) value});

                i += 8;

                break;

            case 0x05:
                value = UBX_DTYPES::convertU8(currKey + 4);

                pairs.push_back({key, (uint64_t) value});

                i += 12;

                break;
        }
    }
    
    this->pairs = pairs;
}

CFGData::CFGData(std::vector<CFGData::CFGDataPair> pairs)
{
    this->pairs = pairs;
}

std::vector<uint8_t> CFGData::CFGDataPair::getPair()
{
    std::vector<uint8_t> data;
    data.reserve(12);   // Ensure underlying array is of 12 bytes (Not too efficient memory-wise but saves time resizing)
    
    // Resize to 5 bytes to cover the key and at least 1 byte. This does not affect the array size as 5 < 12 (capacity)
    // This allows safe access of the data using the [] operator. Without the resize, using the [] operator may be unsafe
    data.resize(5);

    uint32_t flippedKey = UBX_DTYPES::convertU4((uint32_t) this->key);

    UBX_DTYPES::splitInt(flippedKey, data.data());

    switch(this->dtype)
    {
        case UBX_DTYPES::DTYPES::U1: default:
            data[4] = this->value.B1;
            break;
        
        case UBX_DTYPES::DTYPES::U2:
            data.resize(6);

            UBX_DTYPES::splitInt(UBX_DTYPES::convertU2(this->value.B2), data.data() + 4);

            break;

        case UBX_DTYPES::DTYPES::U4:
            data.resize(8);

            UBX_DTYPES::splitInt(UBX_DTYPES::convertU4(this->value.B4), data.data() + 4);

            break;

        case UBX_DTYPES::DTYPES::U8:
            data.resize(12);

            UBX_DTYPES::splitInt(UBX_DTYPES::convertU8(this->value.B8), data.data() + 4);

            break;
    }

    return data;
}

CFG::KEYS CFGData::CFGDataPair::getKey()
{
    return this->key;
}

uint8_t CFGData::CFGDataPair::getValueU1()
{
    return this->value.B1;
}

uint16_t CFGData::CFGDataPair::getValueU2()
{
    return this->value.B2;
}

uint32_t CFGData::CFGDataPair::getValueU4()
{
    return this->value.B4;
}

uint64_t CFGData::CFGDataPair::getValueU8()
{
    return this->value.B8;
}

UBX_DTYPES::DTYPES CFGData::CFGDataPair::getDatatype()
{
    return this->dtype;
}


std::vector<CFGData::CFGDataPair> CFGData::getPairs()
{
    return this->pairs;
}

// NOTE: Assumes that pairs is initialised
std::vector<uint8_t> CFGData::getData()
{
    std::vector<uint8_t> data;

    uint16_t nPairs = this->pairs.size();

    data.reserve(nPairs * 5);   // Reserve at least 5 bytes for each pair. Reduces number of resizings later.

    for (CFGData::CFGDataPair pair : this->pairs)
    {
        for (uint8_t val : pair.getPair())
        {
            data.push_back(val);
        }
    }

    return data;
}

// Returns the pair that is last on the message if more than one instance exist, or NULL if non exist.
CFGData::CFGDataPair * const CFGData::getPair(CFG::KEYS key)
{
    CFGDataPair * pair = NULL;

    uint16_t i;
    
    for (i = 0; i < this->pairs.size(); i++)
    {
        CFGDataPair p = this->pairs[i];

        if (p.getKey() == key)
        {
            pair = &p;
        }
    }

    return pair;
}


UBX::UBX(){}

void UBX::readPayload(const uint8_t * const payload){}
std::vector<uint8_t> UBX::getPayload(){return std::vector<uint8_t>();}

bool UBX::getValidity()
{
    return this->valid;
}

// Currently assumes that a message is present (ie. there exists 0xb5 and 0x62)
// Also assumes that the message is complete and the memory is accessible throughout the message
void UBX::readUBX(const uint8_t * const message)
{
    char preamble[3] = {0xb5, 0x62, '\0'};

    uint8_t * msg = (uint8_t *) strstr((const char * const) message, preamble);

    this->clazz = msg[2];
    this->id = msg[3];
    this->length = (uint16_t) msg[5] << 8 | (uint16_t) msg[4];
    this->payload = msg + 6;
    this->checksum[0] = msg[this->length + 6];
    this->checksum[1] = msg[this->length + 7];

    // Verify if the calculated checksum and the given checksum are the same.
    this->valid = UBX::checkChecksum(message + 2, 4 + this->length, this->checksum[0], this->checksum[1]);

    this->readPayload(this->payload);
}

/** 
 * NOTE: When using the returned vector, ensure that the vector is saved in its own variable
 *       This ensures that the data is saved and the vector is not collected by garbage collection
 *       which would otherwise remove the data saved within.
 *       For example:
 *       This is preferred:
```
std::vector<uint8_t> vec = ubx.getUBX();
uint8_t * data = vec.data();
```
 *       Whereas this results in undefined behaviour:
```
uint8_t * data = ubx.getUBX().data();
```
 */
std::vector<uint8_t> UBX::getUBX()
{
    uint8_t i;

    std::vector<uint8_t> payload = this->getPayload();

    uint16_t payloadSize = payload.size();

    // Only temporary fixed size:
    std::vector<uint8_t> message(2 + 4 + payloadSize + 2, 0);

    message[0] = 0xb5;
    message[1] = 0x62;
    message[2] = this->getClass();
    message[3] = this->getID();
    message[4] = payloadSize & 0x00FF;
    message[5] = (payloadSize & 0xFF00) >> 8;

    for (i = 0; i < payloadSize; i++)
    {
        message[6 + i] = payload[i];
    }

    uint16_t check = UBX::ubxChecksum(message.data() + 2, 4 + payloadSize);

    message[6 + payloadSize] = (check & 0xFF00) >> 8;
    message[6 + payloadSize + 1] = check & 0xFF;

    return message;
}

uint16_t UBX::ubxChecksum(const uint8_t * const checksumRegion, uint16_t length)
{
    uint16_t i = 0;
    uint8_t CK_A = 0, CK_B = 0;

    for (i = 0; i < length; i++)
    {
        CK_A += checksumRegion[i];
        CK_B += CK_A;
    }

    return (uint16_t) CK_A << 8 | (uint16_t) CK_B;
}

bool UBX::checkChecksum(const uint8_t * const checksumRegion, uint16_t length, uint8_t CK_A, uint8_t CK_B)
{
    uint16_t check = UBX::ubxChecksum(checksumRegion, length);

    return check == (uint16_t) CK_A << 8 | CK_B;
}

CFG_VALGET::CFG_VALGET(CFG::LAYER layer, uint16_t position, std::vector<CFG::KEYS> keys)
{
    this->layer = layer;
    this->position = UBX_DTYPES::convertU2(position);
    this->keys = keys;
}

void CFG_VALGET::readPayload(const uint8_t * const payload)
{
    uint16_t i;

    this->version = payload[0];
    this->layer = CFG::getLayer(payload[1]);
    this->position = UBX_DTYPES::convertU2(payload + 2);    // Bytes 2 and 3
    
    // uint16_t nKeys = (this->length - 4) / 4;

    this->cfgData = new CFGData((uint8_t *) payload + 4, this->length - 4);
}

std::vector<uint8_t> CFG_VALGET::getPayload()
{
    uint8_t i;

    // Hard coded for now - only works with altitude setting
    std::vector<uint8_t> payload(4 + this->keys.size() * 4, 0);

    payload[0] = this->version; // 0x00 for sending, 0x01 for receiving
    payload[1] = this->layer;

    // Bytes 2 and 3
    UBX_DTYPES::splitInt(UBX_DTYPES::convertU2(this->position), payload.data() + 2);

    for (i = 0; i < this->keys.size(); i++)
    {
        CFG::KEYS key = this->keys[i];

        // Bytes 4 + 4n, 5 + 4n, 6 + 4n, 7 + 4n
        UBX_DTYPES::splitInt(UBX_DTYPES::convertU4(key), payload.data() + 4 + 4*i);
    }

    return payload;
}

CFGData * const CFG_VALGET::getCFGData()
{
    return this->cfgData;
}

CFG_VALGET::~CFG_VALGET()
{
    if (this->cfgData != NULL)
    {
        delete this->cfgData;
    }
}


// NOTE: Data can be either 1, 2, 4, or 8 bytes
// Currently only one key can be set and only one 1-byte data value can be given
// Will need to accept more keys and more data types
// Can have only max 64 key-value pairs
CFG_VALSET::CFG_VALSET(CFG::LAYER layer, std::vector<CFGData::CFGDataPair> pairs)
{
    this->cfgData = new CFGData(pairs);

    switch (layer)
    {
        case CFG::LAYER::RAM: case CFG::LAYER::DEFAULT:
            this->layers |= (1 << 0);
            break;

        case CFG::LAYER::BBR:
            this->layers |= (1 << 1);
            break;

        case CFG::LAYER::FLASH:
            this->layers |= (1 << 2);
            break;
    }

    this->cfgData = cfgData;
}

/**
 * Returns the byte payload for the current configuration settings.
 */
std::vector<uint8_t> CFG_VALSET::getPayload()
{
    uint8_t i;

    std::vector<uint8_t> cfgData = this->cfgData->getData();

    std::vector<uint8_t> payload(4 + cfgData.size(), 0);

    payload[0] = this->version;
    payload[1] = this->layers;
    payload[2] = this->reserved[0];
    payload[3] = this->reserved[1];

    for (i = 0; i < cfgData.size(); i++)
    {
        payload[4 + i] = cfgData[i];
    }

    return payload;
}

CFG_VALSET::~CFG_VALSET()
{
    if (this->cfgData != NULL)
    {
        delete this->cfgData;
    }
}


ACK::UBX_ACK::UBX_ACK(uint8_t clsID, uint8_t msgID)
{
    this->clsID = clsID;
    this->msgID = msgID;
}

void ACK::UBX_ACK::readPayload(const uint8_t * const payload)
{
    this->clsID = payload[0];
    this->msgID = payload[1];
}


ACK::ACK::ACK(uint8_t clsID, uint8_t msgID) : ACK::UBX_ACK(clsID, msgID){}

ACK::NAK::NAK(uint8_t clsID, uint8_t msgID) : ACK::UBX_ACK(clsID, msgID){}