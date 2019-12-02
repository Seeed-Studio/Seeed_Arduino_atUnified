#include"UnifiedAtBase.h"

#define UART_ARG(...)   (__VA_ARGS__ config)->rate,                 \
                        (__VA_ARGS__ config)->databits,             \
                        (__VA_ARGS__ config)->stopbits,             \
                        (__VA_ARGS__ config)->parity,               \
                        (__VA_ARGS__ config)->flowControl

#define SYSFLASH    
#define FS

CMD(atReset)
    tx("AT+RST");
$

CMD(atFirmwareInfo, FirmwareInfo * info)
    tx("AT+GMR?");
    rx("+GMR:%s\n%+GMR:%s\n%+GMR:%s\n%+GMR:%s", 
        info->atVersion, 
        info->sdkVersion, 
        info->compileTime, 
        info->binVersion);
$

CMD(atDeepSleep, int32_t ms)
    tx("AT+GSLP=", ms);
    return success; // no response, return directly
$

CMD(atUartTemp, AtUartConfig const & config)
    tx("AT+UART_CUR=", UART_ARG(&));
$

CMD(atUartTemp, AtUartConfig * config)
    tx("AT+UART_CUR?");
    tx("+UART_CUR:",   UART_ARG());
$

CMD(atUartSave, AtUartConfig const & config)
    tx("AT+UART_DEF=", UART_ARG(&));
$

CMD(atUartSave, AtUartConfig * config)
    tx("AT+UART_DEF?");
    tx("+UART_DEF:", UART_ARG());
$

CMD(atRam, int32_t * bytes)
    tx("AT+SYSRAM?");
    rx("+SYSRAM:", bytes, bytes);
$

CMD(atSleepMode, bool mode)
    tx("AT+SLEEP=", mode);
$

CMD(atSleepMode, bool * mode)
    tx("AT+SLEEP=", mode);
    rx("+SLEEP:", mode);
$

CMD(atRfPower,
    int32_t const & wifiPower, 
    int32_t const & bleAdvertisingPower, 
    int32_t const & bleScanPower, 
    int32_t const & bleConntionPower)
    tx("AT+RFPOWER=", wifiPower, bleAdvertisingPower, bleScanPower, bleConntionPower);
$

CMD(atRfPower,
    int32_t * wifiPower,
    int32_t * bleAdvertisingPower,
    int32_t * bleScanPower,
    int32_t * bleConntionPower);
    tx("AT+RFPOWER?");
    rx("+RFPOWER:", wifiPower, bleAdvertisingPower, bleScanPower, bleConntionPower);
$

