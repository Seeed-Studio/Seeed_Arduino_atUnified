#pragma once
#include"UnifiedAtType.h"

struct FirmwareInfo{
    Text atVersion;
    Text sdkVersion;
    Text compileTime;
    Text binVersion;
    Text & operator [](size_t index){
        return ((Text *)this)[index];
    }
};

struct AtUartConfig {
    Ni32    rate;
    Ni8     databits;
    Ni8     stopbits;
    Ni8     parity;
    Ni8     flowControl;
};

bool atReset();
bool atFirmwareInfo(FirmwareInfo * result);
bool atDeepSleep(int32_t ms);
bool atUartTemp(AtUartConfig const & config);
bool atUartSave(AtUartConfig const & config);
bool atSleepMode(bool enable);
bool atSleepMode(bool * result);
bool atRfPower(
    int32_t const & wifiPower, 
    int32_t const & bleAdvertisingPower = nullref, 
    int32_t const & bleScanPower = nullref, 
    int32_t const & bleConntionPower = nullref);
bool atRfPower(
    int32_t * wifiPower,
    int32_t * bleAdvertisingPower = nullptr,
    int32_t * bleScanPower = nullptr,
    int32_t * bleConntionPower = nullptr);

