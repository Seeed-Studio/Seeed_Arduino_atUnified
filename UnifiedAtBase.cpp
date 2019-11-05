#include"UnifiedAtBase.h"

#define RST             "+RST"
#define GMR             "+GMR"
#define GSLP            "+GSLP"
#define SLEEP           "+SLEEP"
#define UART_CUR        "+UART_CUR"
#define UART_DEF        "+UART_DEF"
#define SYSRAM          "+SYSRAM"
#define SYSFLASH     
#define FS           
#define RFPOWER         "+RFPOWER"

extern volatile bool needWaitWeekup;
extern volatile bool idEndLine;
extern bool atBegin();
extern void waitReady();
extern String readLine();

CMD(atReset, actionMode mode)
    tx(AT RST);
    if (mode == actionMode::waitReady){
        waitReady();
        return success;
    }
    needWaitWeekup = true;
$

CMD(atFirmwareInfo, FirmwareInfo * result)
    tx(AT GMR);
    if (rx("AT version:",   & result->atVersion)   == success &&
        rx("SDK version:",  & result->sdkVersion)  == success &&
        rx("compile time:", & result->compileTime) == success &&
        rx("Bin version:",  & result->binVersion)  == success
    );
$

CMD(atDeepSleep, int32_t ms)
    needWaitWeekup = true;
    tx(AT GSLP SET, ms);
    return success;
$

void uart(const char * header, AtUartConfig const & config){
    tx(
        header,
        config.rate, 
        config.databits, 
        config.stopbits, 
        config.parity, 
        config.flowControl
    );
}
void uart(const char * ask, const char * resp, AtUartConfig & config){
    tx(ask);
    rx(resp,
        & config.rate, 
        & config.databits, 
        & config.stopbits, 
        & config.parity, 
        & config.flowControl
    );
}

CMD(atUartTemp, AtUartConfig const & config)
    uart(AT UART_CUR SET, config);
$

CMD(atUartTemp, AtUartConfig * config)
    uart(AT UART_CUR ASK, UART_CUR TOKEN, config[0]);
$

CMD(atUartNovolatile, AtUartConfig const & config)
    uart(AT UART_DEF SET, config);
$

CMD(atUartNovolatile, AtUartConfig * config)
    uart(AT UART_DEF ASK, UART_DEF TOKEN, config[0]);
$

CMD(atSleepMode, bool enable)
    tx(AT SLEEP SET, enable);
$

CMD(atSleepMode, bool * result)
    int32_t tmp;
    tx(AT SLEEP ASK);
    rx(AT SLEEP TOKEN, & tmp);
    result[0] = tmp;
$

