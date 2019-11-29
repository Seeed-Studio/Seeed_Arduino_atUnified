#pragma once
#include"UnifiedAtType.h"

struct BleScanMethod{
    // - 0 : passive scan
    // - 1 : active scan
    bool    mode;

    // - 0 : public address
    // - 1 : random address
    // - 2 : RPA public address
    // - 3 : RPA random address
    uint8_t ownAddressType;

    // - 0 : BLE_SCAN_FILTER_ALLOW_ALL 
    // - 1 : BLE_SCAN_FILTER_ALLOW_ONLY_WLST 
    // - 2 : BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR 
    // - 3 : BLE_SCAN_FILTER_ALLOW_WLIST_PRA_DIR
    uint8_t filter;

    // unit : micro second
    int32_t scanInterval;

    // NOTICE:
    // CANNOT be larger than scanInterval
    int32_t scanWindow;
};

struct BleAdvertisingMethod{
    // range : 0x0020 ~ 0x4000
    int32_t minInterval;

    // range : 0x0020 ~ 0x4000
    int32_t maxInterval;

    // - 0 : ADV_TYPE_IND 
    // - 1 : ADV_TYPE_DIRECT_IND_HIGH
    // - 2 : ADV_TYPE_SCAN_IND
    // - 3 : ADV_TYPE_NONCONN_IND
    int32_t advertisingType;

    // - 0 : public address
    // - 1 : random address
    int32_t ownAddressType;

    // - 1 : channel 37
    // - 2 : channel 38
    // - 4 : channel 39
    // - 7 : all
    int32_t channel;

    // 0 : ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY
    // 1 : ADV_FILTER_ALLOW_SCAN_WLST_CON_ANY
    // 2 : ADV_FILTER_ALLOW_SCAN_ANY_CON_WLST
    // 3 : ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST
    int32_t filter;

    // - 0 : public address
    // - 1 : random address
    uint8_t peerAddressType;

    Mac     peerAddress;

    BleAdvertisingMethod() : 
        filter(leaveOut), 
        peerAddressType(leaveOut){ }
};

struct BleConnectMethod{
    int32_t index;

    // range : 0x0006 ~ 0x0c80
    int32_t minInterval;

    // range : 0x0006 ~ 0x0c80
    int32_t maxInterval;

    // range : 0x0000 ~ 0x01f3
    int32_t latency;

    // range : 0x000a ~ 0x0c80
    int32_t timeout;

    // read only.
    const int32_t currentInterval;

    BleConnectMethod() : currentInterval(leaveOut){}
};

struct IndexLengthPair{
    int32_t index;
    int32_t length;
};

bool atBleMode(int32_t mode);
bool atBleMode(int32_t * mode);
bool atBleAddress(int32_t addressType, Mac const & address);
bool atBleAddress(Mac * address);
bool atBleName(String const & name);
bool atBleName(String * name);
bool atBleScanMethod(BleScanMethod const & method);
bool atBleScanMethod(BleScanMethod * method);
bool atBleScan(bool enable, int32_t interval = leaveOut);
bool atBleScanResponse(String const & hexString);
bool atBleAdvertisingMethod(BleAdvertisingMethod const & method);
bool atBleAdvertisingMethod(BleAdvertisingMethod * method);
bool atBleAdvertisingData(String const & hexString);
bool atBleAdvertisingStart();
bool atBleAdvertisingStop();
bool atBleConnect(int32_t index, Mac const & remoteAddress, int32_t addressType = leaveOut);
bool atBleConnect(int32_t * index, Mac * remoteAddress);
bool atBleConnectMethod(BleConnectMethod const & method);
bool atBleConnectMethod(BleConnectMethod * method);
bool atBleDisconnect(int32_t index);
bool atBleTxDataLength(int32_t index, int32_t length);
bool atBleRxDataLength(int32_t index, int32_t length);
bool atBleRxDataLength(std::vector<IndexLengthPair> & list);
bool atBleCreateGattsService();

