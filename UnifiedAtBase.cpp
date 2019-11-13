#include"UnifiedAtBase.h"

#define SET_RST         "AT+RST"
#define GET_GMR         "+GMR:%s\n%+GMR:%s\n%+GMR:%s\n%+GMR:%s",    \
                        info->atVersion,                            \
                        info->sdkVersion,                           \
                        info->compileTime,                          \
                        info->binVersion
#define ASK_GMR         "AT+GMR?"
#define SET_GSLP        "AT+GSLP=%d", ms
#define SET_SLEEP       "AT+SLEEP=%d", mode
#define ASK_SLEEP       "AT+SLEEP?"
#define GET_SLEEP       "+SLEEP:%b", mode

#define UART_ARG(...)   (__VA_ARGS__ config)->rate,                 \
                        (__VA_ARGS__ config)->databits,             \
                        (__VA_ARGS__ config)->stopbits,             \
                        (__VA_ARGS__ config)->parity,               \
                        (__VA_ARGS__ config)->flowControl

#define SET_UART_CUR    "AT+UART_CUR=%d,%d,%d,%d,%d", UART_ARG(&)
#define ASK_UART_CUR    "AT+UART_CUR?"
#define GET_UART_CUR    "+UART_CUR:%d,%d,%d,%d,%d",   UART_ARG()

#define SET_UART_DEF    "AT+UART_DEF=%d,%d,%d,%d,%d", UART_ARG(&)
#define ASK_UART_DEF    "AT+UART_DEF?"
#define GET_UART_DEF    "+UART_DEF:%d,%d,%d,%d,%d",   UART_ARG()

#define ASK_SYSRAM      "AT+SYSRAM?"
#define GET_SYSRAM      "+SYSRAM:%d", bytes

#define SYSFLASH    
#define FS          
#define RFPOWER         "+RFPOWER"

extern volatile bool needWaitWeekup;
extern bool atBegin();
extern void waitReady();
extern String readLine();

CMD(atReset, actionMode mode)
    tx(SET_RST);
    if (mode == actionMode::waitReady){
        waitReady();
        return success;
    }
    needWaitWeekup = true;
$

CMD(atFirmwareInfo, FirmwareInfo * info)
    tx(ASK_GMR);
    rx(GET_GMR);
$

CMD(atDeepSleep, int32_t ms)
    needWaitWeekup = true;
    tx(SET_GSLP);
    return success; // no response, return directly
$

CMD(atUartTemp, AtUartConfig const & config)
    tx(SET_UART_CUR);
$

CMD(atUartTemp, AtUartConfig * config)
    tx(ASK_UART_CUR);
    tx(GET_UART_CUR);
$

CMD(atUartNovolatile, AtUartConfig const & config)
    tx(SET_UART_DEF);
$

CMD(atUartNovolatile, AtUartConfig * config)
    tx(ASK_UART_DEF);
    tx(GET_UART_DEF);
$

CMD(atRam, int32_t * bytes)
    tx(ASK_SYSRAM);
    rx(GET_SYSRAM, bytes);
$

CMD(atSleepMode, bool mode)
    tx(SET_SLEEP);
$

CMD(atSleepMode, bool * mode)
    tx(SET_SLEEP);
    rx(GET_SLEEP);
$

