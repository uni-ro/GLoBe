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
    enum DTYPES
    {
        U1, I1, X1, U2, I2, X2, U4, I4, X4, R4, R8, CH, Un, In, Sn, U8
    };

    // TODO: Move into own file called bit manipulation or something
    void splitInt(uint16_t value, uint8_t * arr);
    void splitInt(uint32_t value, uint8_t * arr);
    void splitInt(uint64_t value, uint8_t * arr);

    uint16_t convertU2(const uint8_t * const littleEndian);
    uint16_t convertU2(const uint16_t littleEndian);
    uint32_t convertU4(const uint8_t * const littleEndian);
    uint32_t convertU4(const uint32_t littleEndian);
    uint64_t convertU8(const uint8_t * const littleEndian);
    uint64_t convertU8(const uint64_t littleEndian);
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

        CFG::KEYS key;
        KeyData value;
        UBX_DTYPES::DTYPES dtype;

        public:
        CFGDataPair(CFG::KEYS key, uint8_t value);
        CFGDataPair(CFG::KEYS key, uint16_t value);
        CFGDataPair(CFG::KEYS key, uint32_t value);
        CFGDataPair(CFG::KEYS key, uint64_t value);

        std::vector<uint8_t> getPair();

        CFG::KEYS getKey();
        uint8_t getValueU1();
        uint16_t getValueU2();
        uint32_t getValueU4();
        uint64_t getValueU8();

        UBX_DTYPES::DTYPES getDatatype();
    };
    
    
    private:
    std::vector<CFGDataPair> pairs;
    
    public:
    CFGData(uint8_t * bytes, uint16_t nBytes);
    CFGData(std::vector<CFGDataPair> pairs);

    std::vector<CFGDataPair> getPairs();
    std::vector<uint8_t> getData();

    CFGDataPair * const getPair(CFG::KEYS key);
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

    public:
    virtual uint8_t getClass() {return 0x00;}
    virtual uint8_t getID() {return 0x00;}

    protected:
    virtual void readPayload(const uint8_t * const payload);
    virtual std::vector<uint8_t> getPayload();

    public:
    bool getValidity();

    virtual void readUBX(const uint8_t * const message);
    virtual std::vector<uint8_t> getUBX();
    static uint16_t ubxChecksum(const uint8_t * const checksumRegion, uint16_t length);
    static bool checkChecksum(const uint8_t * const checksumRegion, uint16_t length, uint8_t CK_A, uint8_t CK_B);
};

class CFG_VALGET : public UBX
{
    public:
    CFG_VALGET(CFG::LAYER layer, uint16_t position, std::vector<CFG::KEYS> keys);

    public:
    uint8_t getClass() override {return 0x06;}
    uint8_t getID() override {return 0x8b;}

    protected:
    // Payload:
    uint8_t version = 0x00;
    CFG::LAYER layer;
    uint16_t position;
    std::vector<CFG::KEYS> keys;
    CFGData * cfgData = NULL;

    public:
    void readPayload(const uint8_t * const payload) override;
    std::vector<uint8_t> getPayload() override;

    CFGData * const getCFGData();

    public:
    ~CFG_VALGET();
};

class CFG_VALSET : public UBX
{
    public:
    CFG_VALSET(CFG::LAYER layer, std::vector<CFGData::CFGDataPair> pairs);

    public:
    uint8_t getClass() override {return 0x06;}
    uint8_t getID() override {return 0x8a;}

    protected:
    // Payload:
    uint8_t version = 0x00;
    uint8_t layers = 0x00;
    uint8_t reserved[2] = {0x00, 0x00};
    CFGData * cfgData = NULL;

    public:
    std::vector<uint8_t> getPayload() override;

    public:
    ~CFG_VALSET();
};

namespace ACK
{
    class UBX_ACK : public UBX
    {
        protected:
        UBX_ACK(uint8_t clsID, uint8_t msgID);

        public:
        uint8_t getClass() override {return 0x05;}

        protected:
        // Payload:
        uint8_t clsID;
        uint8_t msgID;

        public:
        void readPayload(const uint8_t * const payload) override;
    };

    class ACK : public UBX_ACK
    {
        public:
        ACK(uint8_t clsID, uint8_t msgID);
        uint8_t getID() override {return 0x01;}
    };
    
    class NAK : public UBX_ACK
    {
        public:
        NAK(uint8_t clsID, uint8_t msgID);
        uint8_t getID() override {return 0x00;}
    };
}


#endif