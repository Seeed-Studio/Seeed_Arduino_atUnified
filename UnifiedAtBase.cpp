#include"UnifiedAtBase.h"

#define SET_RST         "AT+RST"
#define GET_GMR         "+GMR:%s\n%+GMR:%s\n%+GMR:%s\n%+GMR:%s",    \
                        & a0->atVersion,                            \
                        & a0->sdkVersion,                           \
                        & a0->compileTime,                          \
                        & a0->binVersion
#define ASK_GMR         "AT+GMR?"
#define SET_GSLP        "AT+GSLP=%d", a0
#define SET_SLEEP       "AT+SLEEP=%d", a0
#define ASK_SLEEP       "AT+SLEEP?"
#define GET_SLEEP       "+SLEEP:%d", a0

#define UART_ARG(...)   (__VA_ARGS__ a0)->rate,                     \
                        (__VA_ARGS__ a0)->databits,                 \
                        (__VA_ARGS__ a0)->stopbits,                 \
                        (__VA_ARGS__ a0)->parity,                   \
                        (__VA_ARGS__ a0)->flowControl

#define SET_UART_CUR    "AT+UART_CUR=%d,%d,%d,%d,%d", UART_ARG(&)
#define ASK_UART_CUR    "AT+UART_CUR?"
#define GET_UART_CUR    "+UART_CUR:%d,%d,%d,%d,%d",   UART_ARG()

#define SET_UART_DEF    "AT+UART_DEF=%d,%d,%d,%d,%d", UART_ARG(&)
#define ASK_UART_DEF    "AT+UART_DEF?"
#define GET_UART_DEF    "+UART_DEF:%d,%d,%d,%d,%d",   UART_ARG()

#define ASK_SYSRAM      "AT+SYSRAM?"
#define GET_SYSRAM      "+SYSRAM:%d", a0

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

CMD(atFirmwareInfo, FirmwareInfo * a0)
    tx(ASK_GMR);
    rx(GET_GMR);
$

CMD(atDeepSleep, int32_t a0)
    needWaitWeekup = true;
    tx(SET_GSLP);
    return success; // no response, return directly
$

CMD(atUartTemp, AtUartConfig const & a0)
    tx(SET_UART_CUR);
$

CMD(atUartTemp, AtUartConfig * a0)
    tx(ASK_UART_CUR);
    tx(GET_UART_CUR);
$

CMD(atUartNovolatile, AtUartConfig const & a0)
    tx(SET_UART_DEF);
$

CMD(atUartNovolatile, AtUartConfig * a0)
    tx(ASK_UART_DEF);
    tx(GET_UART_DEF);
$

CMD(atSleepMode, bool a0)
    tx(SET_SLEEP);
$

CMD(atSleepMode, bool * result)
    int32_t a0;
    tx(SET_SLEEP);
    rx(GET_SLEEP);
    result[0] = a0;
$

