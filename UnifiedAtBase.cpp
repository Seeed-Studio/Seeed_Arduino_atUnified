#include"UnifiedAtBase.h"

#define UART_ARG(...)   (__VA_ARGS__ config)->rate,                 \
                        (__VA_ARGS__ config)->databits,             \
                        (__VA_ARGS__ config)->stopbits,             \
                        (__VA_ARGS__ config)->parity,               \
                        (__VA_ARGS__ config)->flowControl

#define SYSFLASH    
#define FS

extern volatile bool needWaitWeekup;
extern bool atBegin();
extern void waitReady();
extern String readLine();

CMD(atReset)
    tx("AT+RST");
    waitReady();
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
    needWaitWeekup = true;
    tx("AT+GSLP=%d", ms);
    return success; // no response, return directly
$

CMD(atUartTemp, AtUartConfig const & config)
    tx("AT+UART_CUR=%d,%d,%d,%d,%d", UART_ARG(&));
$

CMD(atUartTemp, AtUartConfig * config)
    tx("AT+UART_CUR?");
    tx("+UART_CUR:%d,%d,%d,%d,%d",   UART_ARG());
$

CMD(atUartSave, AtUartConfig const & config)
    tx("AT+UART_DEF=%d,%d,%d,%d,%d", UART_ARG(&));
$

CMD(atUartSave, AtUartConfig * config)
    tx("AT+UART_DEF?");
    tx("+UART_DEF:%d,%d,%d,%d,%d",   UART_ARG());
$

CMD(atRam, int32_t * bytes)
    tx("AT+SYSRAM?");
    rx("+SYSRAM:%d", bytes, bytes);
$

CMD(atSleepMode, bool mode)
    tx("AT+SLEEP=%b", mode);
$

CMD(atSleepMode, bool * mode)
    tx("AT+SLEEP=%d", mode);
    rx("+SLEEP:%b", mode);
$

CMD(atRfPower, RfPower const & config)
    tx("AT+RFPOWER=%d%+d%+d%+d", 
        config.wifiPower,
        config.bleAdvertisingPower,
        config.bleScanPower,
        config.bleConntionPower
    );
$

CMD(atRfPower, RfPower * config)
    tx("AT+RFPOWER?");
    rx("+RFPOWER:%d,%d,%d,%d", 
        config->wifiPower,
        config->bleAdvertisingPower,
        config->bleScanPower,
        config->bleConntionPower
    );
$

