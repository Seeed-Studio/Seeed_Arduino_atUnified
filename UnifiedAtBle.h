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

    mac     peerAddress;

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

// NOTICE:
// The BLE commands and the UART-Wi-Fi passthrough mode cannot be used together, 
// so before BLE initialization, please ensure that the UART-Wi-Fi passthrough mode 
// is not enabled (AT+CIPMODE=0).

// mode
// - 0 : de-init BLE, disable BLE RF * 
// - 1 : client role 
// - 2 : server role
CMD(atBleMode, int32_t mode)
    // The host side will receive a disconnected response line when other devices are previously connected.
    int32_t index;
    mac     dummy;
    for (
        tx("AT+BLEINIT=%d", mode);
        rx("+BLEDISCONN:%d,%m", index, dummy);
    );
$

CMD(atBleMode, int32_t * mode)
    tx("AT+BLEINIT?");
    rx("+BLEINIT:%d", mode);
$

// NOTICE:
// Current version(2019.11) only support random address type(addressType need be set 1).
// addressType
// - 0 : public address
// - 1 : random address
CMD(atBleAddress, int32_t addressType, mac const & address)
    tx("AT+BLEADDR=%d,%m", addressType, address);
$

CMD(atBleAddress, mac * address)
    tx("AT+BLEADDR?");
    rx("+BLEADDR:%m", address);
$

CMD(atBleName, String const & name)
    tx("AT+BLENAME=%s", name);
$

CMD(atBleName, String * name)
    tx("AT+BLENAME?");
    rx("+BLENAME=%s", name);
$

CMD(atBleScanMethod, BleScanMethod const & method)
    tx("AT+BLESCANPARAM=%d,%d,%d,%d,%d",
        method.mode,
        method.ownAddressType,
        method.filter,
        method.scanInterval,
        method.scanWindow
    );
$

CMD(atBleScanMethod, BleScanMethod * method)
    tx("AT+BLESCANPARAM?");
    rx("+BLESCANPARAM:%d,%d,%d,%d,%d",
        method->mode,
        method->ownAddressType,
        method->filter,
        method->scanInterval,
        method->scanWindow
    );
$

// interval unit : second
CMD(atBleScan, bool enable, int32_t interval = leaveOut)
    tx("AT+BLESCAN=%d%+d", enable, interval);
$

CMD(atBleScanResponse, String const & hexString)
    tx("AT+BLESCANRSPDATA=%s", hexString);
$

CMD(atBleAdvertisingMethod, BleAdvertisingMethod const & method)
    tx("AT+BLEADVPARAM=%d,%d,%d,%d,%d%+d%+d%+m",
        method.minInterval,
        method.maxInterval,
        method.advertisingType,
        method.ownAddressType,
        method.channel,
        method.filter,
        method.peerAddressType,
        method.peerAddress
    );
$

CMD(atBleAdvertisingMethod, BleAdvertisingMethod * method)
    tx("AT+BLEADVPARAM?");
    rx("+BLEADVPARAM:%d,%d,%d,%d,%d%+d%+d%+m",
        method->minInterval,
        method->maxInterval,
        method->advertisingType,
        method->ownAddressType,
        method->channel,
        method->filter,
        method->peerAddressType,
        method->peerAddress
    );
$

CMD(atBleAdvertisingData, String const & hexString)
    tx("AT+BLEADVDATA=%s", hexString);
$

CMD(atBleAdvertisingStart)
    tx("AT+BLEADVSTART");
$

CMD(atBleAdvertisingStop)
    tx("AT+BLEADVSTOP");
$

// addressType
// - 0 : public address
// - 1 : random address
// - 2 : RPA public address
// - 3 : RPA random address
CMD(atBleConnect, int32_t index, mac const & remoteAddress, int32_t addressType = leaveOut)
    tx("AT+BLECONN=%d,%m%+d", index, remoteAddress, addressType);
    rx("+BLECONN:%d,%m", index, remoteAddress);
$

CMD(atBleConnect, int32_t * index, mac * remoteAddress)
    tx("AT+BLECONN?");
    rx("+BLECONN:%d,%m", index, remoteAddress);
$

CMD(atBleConnectMethod, BleConnectMethod const & method)
    tx("AT+BLECONNPARAM=%d,%d,%d,%d,%d", 
        method.index, 
        method.minInterval,
        method.maxInterval,
        method.latency,
        method.timeout
    );
$

CMD(atBleConnectMethod, BleConnectMethod * method)
    tx("AT+BLECONNPARAM?");
    rx("+BLECONNPARAM:%d,%d,%d,%d,%d",
        method->index, 
        method->minInterval,
        method->maxInterval,
        method->currentInterval,
        method->latency,
        method->timeout
    );
$

CMD(atBleDisconnect, int32_t index)
    // Success Format:
    // +BLEDISCONN:3,"30:ae:a4:06:0e:0e"

    // Fail Format(service part need send AT+BLEADVSTART before client connect to it):
    // +BLEDISCONN:3,-1

    // Error just with empty line.
    mac dummy;
    tx("AT+BLEDISCONN=%d", index);
    rx("+BLEDISCONN:%d,%m", index, dummy);
$

// NOTICE:
// connect to the server before set this value 
// length range : 0x1b ~ 0xfb
CMD(atBleTxDataLength, int32_t index, int32_t length)
    tx("AT+BLEDATALEN=%d,%d", index, length);
$

CMD(atBleRxDataLength, int32_t index, int32_t length)
    tx("AT+BLECFGMTU=%d,%d", index, length);
    if (waitFlag() == success){
        rx("+BLECFGMTU:%d,%d", index, length);
        return success;
    }
    else{
        return fail;
    }
$

CMD(atBleRxDataLength, std::function<void(IndexLengthPair)> call)
    IndexLengthPair pair;
    for (
        tx("AT+BLECFGMTU?"); 
        rx("+BLECFGMTU:%d,%d", pair.index, pair.length) != fail;){
        call(pair);
    }
$

CMD(atBleCreateGattsService)
    tx("AT+BLEGATTSSRVCRE");
$

