#pragma once
#include"UnifiedAtType.h"

struct FirmwareInfo{
    String atVersion;
    String sdkVersion;
    String compileTime;
    String binVersion;
};

struct AtUartConfig {
    int32_t  rate;
    int32_t  databits;
    int32_t  stopbits;
    int32_t  parity;
    int32_t  flowControl;
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

