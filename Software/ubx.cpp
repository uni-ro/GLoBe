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
        uint8_t arr[4] = {
            littleEndian >> 24,
            littleEndian >> 16,
            littleEndian >> 8,
            littleEndian
        };

        uint32_t converted = convertU4(arr);

        return converted;
    };
};


CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint8_t value)
{
    this->key = key;
    this->value.B1 = value;
    this->dtype = KeyDataDtype::BIT8;
}

CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint16_t value)
{
    this->key = key;
    this->value.B2 = value;
    this->dtype = KeyDataDtype::BIT16;
}

CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint32_t value)
{
    this->key = key;
    this->value.B4 = value;
    this->dtype = KeyDataDtype::BIT32;
}

CFGData::CFGDataPair::CFGDataPair(CFG::KEYS key, uint64_t value)
{
    this->key = key;
    this->value.B8 = value;
    this->dtype = KeyDataDtype::BIT64;
}


UBX::UBX(){}

void UBX::readPayload(const uint8_t * const payload){}
std::vector<uint8_t> UBX::getPayload(){return std::vector<uint8_t>();}

// Currently assumes that a message is present (ie. there exists 0xb5 and 0x62)
// Also assumes that the message is complete and the memory is accessible throughout the message
void UBX::readUBX(const uint8_t * const message)
{
    char preamble[2] = {0xb5, 0x62};

    uint8_t * msg = (uint8_t *) strstr((const char * const) message, preamble);

    this->clazz = msg[2];
    this->id = msg[3];
    this->length = (uint16_t) msg[5] << 8 | (uint16_t) msg[4];
    this->payload = msg + 6;
    this->checksum[0] = msg[this->length + 6];
    this->checksum[1] = msg[this->length + 7];

    // Verify if the calculated checksum and the given checksum are the same.
    this->valid = UBX::ubxChecksum(message + 2, 4 + this->length) == (((uint16_t) this->checksum[0] << 8) | this->checksum[1]);

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
    
    // Only temporary fixed size:
    std::vector<uint8_t> message(2 + 4 + 2 + payload.size() + 2, 0);

    message[0] = 0xb5;
    message[1] = 0x62;
    message[2] = this->getClass();
    message[3] = this->getID();
    message[4] = payload.size() & 0x00FF;
    message[5] = (payload.size() & 0xFF00) >> 8;
    
    for (i = 0; i < payload.size(); i++)
    {
        message[6 + i] = payload[i];
    }

    uint16_t check = UBX::ubxChecksum(message.data() + 2, 4 + payload.size());

    message[15] = (check & 0xFF00) >> 8;
    message[16] = check & 0xFF;

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

CFG_VALGET::CFG_VALGET(CFG::LAYER layer, uint16_t position, std::vector<CFG::KEYS> keys)
{
    this->layer = layer;
    this->position = UBX_DTYPES::convertU2(position);
    this->keys = (uint32_t *) keys.data();
}

void CFG_VALGET::readPayload(const uint8_t * const payload)
{
    uint16_t i;

    this->version = payload[0];
    this->layer = CFG::getLayer(payload[1]);
    this->position = UBX_DTYPES::convertU2(payload + 2);
    
    uint16_t nKeys = (this->length - 4) / 4;

    this->keys = new uint32_t[nKeys];

    for (i = 0; i < nKeys; i++)
    {
        this->keys[i] = UBX_DTYPES::convertU4(&(payload + 4)[i * 4]);
    }
}

std::vector<uint8_t> CFG_VALGET::getPayload()
{
    uint8_t i;

    // Hard coded for now - only works with altitude setting
    std::vector<uint8_t> payload(4 + 5, 0);

    payload[0] = this->version;
    payload[1] = this->layer;
    payload[2] = this->position & 0xFF00;
    payload[3] = this->position & 0x00FF;


    // TODO
    // for (i = 0; i < 5; i++)
    // {
    //     payload[4 + i] = this->keys[i];
    // }

    return payload;
}

CFG_VALGET::~CFG_VALGET()
{
    if (this->keys != NULL)
    {
        delete this->keys;
    }
}


// NOTE: Data can be either 1, 2, 4, or 8 bytes
// Currently only one key can be set and only one 1-byte data value can be given
// Will need to accept more keys and more data types
// Can have only max 64 key-value pairs
CFG_VALSET::CFG_VALSET(CFG::LAYER layer, CFG::KEYS key, uint8_t data)
{
    // 4 Bytes for the key and 1 for data
    this->cfgData = new uint8_t[5];

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

    uint32_t flippedKey = UBX_DTYPES::convertU4(key);

    // Hard coded for now just for testing purposes
    this->cfgData[0] = (flippedKey & (0xFF << 3 * 8)) >> 3 * 8;
    this->cfgData[1] = (flippedKey & (0xFF << 2 * 8)) >> 2 * 8;
    this->cfgData[2] = (flippedKey & (0xFF << 1 * 8)) >> 1 * 8;
    this->cfgData[3] = (flippedKey & (0xFF << 0 * 8)) >> 0 * 8;
    
    this->cfgData[4] = data;
}

std::vector<uint8_t> CFG_VALSET::getPayload()
{
    uint8_t i;

    // Hard coded for now - only works with altitude setting
    std::vector<uint8_t> payload(4 + 5, 0);

    payload[0] = this->version;
    payload[1] = this->layers;
    payload[2] = this->reserved[0];
    payload[3] = this->reserved[1];

    for (i = 0; i < 5; i++)
    {
        payload[4 + i] = this->cfgData[i];
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
