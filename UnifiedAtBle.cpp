#include"UnifiedAtBle.h"

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
    Mac     dummy;
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
CMD(atBleAddress, int32_t addressType, Mac const & address)
    tx("AT+BLEADDR=%d,%m", addressType, address);
$

CMD(atBleAddress, Mac * address)
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
CMD(atBleScan, bool enable, int32_t interval)
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
CMD(atBleConnect, int32_t index, Mac const & remoteAddress, int32_t addressType)
    tx("AT+BLECONN=%d,%m%+d", index, remoteAddress, addressType);
    rx("+BLECONN:%d,%m", index, remoteAddress);
$

CMD(atBleConnect, int32_t * index, Mac * remoteAddress)
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
    Mac dummy;
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

CMD(atBleRxDataLength, std::vector<IndexLengthPair> & list)
    IndexLengthPair pair;
    for (
        tx("AT+BLECFGMTU?"); 
        rx("+BLECFGMTU:%d,%d", pair.index, pair.length) != fail;){
        list.push_back(pair);
    }
$

CMD(atBleCreateGattsService)
    tx("AT+BLEGATTSSRVCRE");
$

CMD(atBleStartService)
    tx("AT+BLEGATTSSTART");
$

CMD(atBleStopService)
    tx("AT+BLEGATTSSTOP");
$

struct DiscoverData{
    int32_t index;
    int32_t uuid;
    bool    serviceStarted;
    bool    isPrimary;
};

CMD(atBleDiscoverService, std::vector<DiscoverData> & list)
    DiscoverData data;
    for(
        tx("AT+BLEGATTSSRV?"); 
        rx("+BLEGATTSSRV:%d,%b,%d,%b", 
            data.index,
            data.serviceStarted,
            data.uuid,
            data.isPrimary);){
        list.push_back(data);
    }
$

enum DisconversType{
    Characteristics,
    Descriptor,
};

struct CharacteristicsData{
    DisconversType  type;
    int32_t         serviceIndex;
    int32_t         charIndex;
    union{
        struct{
            int32_t uuid;
            int32_t prop;
        } chr;

        struct{
            int32_t index;
        } desc;
    };
};

CMD(atBleDiscoverCharacteristics, std::vector<CharacteristicsData> & list)
    String              type;
    CharacteristicsData data;
    for (tx("AT+BLEGATTSCHAR?");;){
        if (rx("+BLEGATTSCHAR:%s,%d,%d,", 
            type,
            data.serviceIndex,
            data.charIndex) == fail){
            break;
        }
        if (type == "char"){
            data.type = Characteristics;
            rx("%d", data.desc.index);
        }
        else{
            data.type = Descriptor;
            rx("%d,%d", data.chr.uuid, data.chr.prop);
        }
        list.push_back(data);
    }
$

CMD(atBleCharacteristicsNotifies)
$

