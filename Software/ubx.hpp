#ifndef INC_UBX_HPP_
#define INC_UBX_HPP_

#include <string.h>
#include <stdint.h>
#include <vector>

namespace CFG
{
    enum KEYS : uint32_t
    {
        // --------------- NAVSPG ---------------
        NAVSPG_DYNMODEL = 0x20110021
        // --------------------------------------
    };

    enum LAYER : uint8_t
    {
        RAM = 0,
        BBR = 1,
        FLASH = 2,
        DEFAULT = 7
    };

    LAYER getLayer(uint8_t layer);

    namespace NAVSPG
    {
        enum DYNMODEL : uint8_t
        {
            PORT = 0,
            STAT = 2,
            PED = 3,
            AUTOMOT = 4,
            SEA = 5,
            AIR1 = 6,
            AIR2 = 7,
            AIR4 = 8,
            WRIST = 9,
            BIKE = 10,
            MOWER = 11,
            ESCOOTER = 12
        };
    };
};

namespace UBX_DTYPES
{
    uint16_t convertU2(const uint8_t * const littleEndian);
    uint16_t convertU2(const uint16_t littleEndian);
    uint32_t convertU4(const uint8_t * const littleEndian);
    uint32_t convertU4(const uint32_t littleEndian);
};

class CFGData
{
    public:
    class CFGDataPair
    {
        private:
        union KeyData
        {
            uint8_t B1;
            uint16_t B2;
            uint32_t B4;
            uint64_t B8;
        };

        enum KeyDataDtype
        {
            BIT8 = 0,
            BIT16 = 1,
            BIT32 = 2,
            BIT64 = 3
        };

        CFG::KEYS key;
        KeyData value;
        KeyDataDtype dtype;

        public:
        CFGDataPair(CFG::KEYS key, uint8_t value);
        CFGDataPair(CFG::KEYS key, uint16_t value);
        CFGDataPair(CFG::KEYS key, uint32_t value);
        CFGDataPair(CFG::KEYS key, uint64_t value);

        std::vector<uint8_t> getPair();
    };
    
    CFGData(uint8_t * cfgData);

    private:
    std::vector<CFGDataPair> pairs;

    public:
    std::vector<CFGDataPair> getPairs();
    std::vector<uint8_t> getData();
};

class UBX
{
    protected:
    UBX();

    protected:
    bool valid;
    uint8_t clazz;
    uint8_t id;
    uint16_t length;
    uint8_t * payload;  // May change to std::vector later
    uint8_t checksum[2];

    virtual uint8_t getClass() {return 0x00;}
    virtual uint8_t getID() {return 0x00;}

    protected:
    virtual void readPayload(const uint8_t * const payload);
    virtual std::vector<uint8_t> getPayload();

    public:
    virtual void readUBX(const uint8_t * const message);
    virtual std::vector<uint8_t> getUBX();
    static uint16_t ubxChecksum(const uint8_t * const checksumRegion, uint16_t length);
};

class CFG_VALGET : public UBX
{
    public:
    CFG_VALGET(CFG::LAYER layer, uint16_t position, std::vector<CFG::KEYS> keys);

    protected:
    uint8_t getClass() override {return 0x06;}
    uint8_t getID() override {return 0x8b;}

    // Payload:
    uint8_t version = 0x00;
    CFG::LAYER layer;
    uint16_t position;
    uint32_t * keys = NULL;

    public:
    void readPayload(const uint8_t * const payload) override;
    std::vector<uint8_t> getPayload() override;

    protected:
    ~CFG_VALGET();
};

class CFG_VALSET : public UBX
{
    public:
    CFG_VALSET(CFG::LAYER layer, CFG::KEYS key, uint8_t data);

    protected:
    uint8_t getClass() override {return 0x06;}
    uint8_t getID() override {return 0x8a;}

    // Payload:
    uint8_t version = 0x00;
    uint8_t layers = 0x00;
    uint8_t reserved[2] = {0x00, 0x00};
    uint8_t * cfgData = NULL;

    public:
    std::vector<uint8_t> getPayload() override;

    public:
    ~CFG_VALSET();
};

#endif