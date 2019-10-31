#include<string>
#include<stdio.h>
typedef std::string String;

#define AT_AT           "AT"
#define AT_RST          "AT+RST"
#define AT_GMR          "AT+GMR"
#define AT_ATE          "ATE"
#define AT_GSLP         "AT+GSLP"
#define AT_SLEEP        "AT+SLEEP"
#define AT_UART_CUR     "AT+UART_CUR"
#define AT_UART_DEF     "AT+UART_DEF"
#define AT_SYSRAM       "AT+SYSRAM"

#define TOKEN           ":"

#define AT_ERROR        "ERROR"
#define AT_OK           "OK"
#define AT_READY        "ready"
#define SET             "="
#define ASK             "?"

#define CMD(name,...)    bool name(__VA_ARGS__) { if (isNotWakeup()) return fail;
#define $                return waitFlag(); }

#define SEND(...)        Sstring buf; buf << __VA_ARGS__; snd(buf.str())



constexpr bool fail = false;
constexpr bool success = true;
volatile bool needWaitWeekup = false;

void rcv(String & line){

}

String rcv(){

}

enum class state{};
constexpr state ok = state(0);
constexpr state over = state(1);

bool waitFlag(){
    while(true){
        String ack = rcv();
        if (ack == AT_OK){
            return success;
        }
        if (ack == AT_ERROR){
            return fail;
        }
    }
}

template<class ... arg>
state ask(const char * fmt, arg ... list){
    auto && resp = rcv();

    if (sscanf(resp.c_str(), fmt + sizeof("AT") - 1, list...) == sizeof...(list)){
        return ok;
    }
    return over;
}

void snd(){

}
void snd(size_t value){
    
}
void snd(const char * value){
    
}
template<class first, class ... arg>
void snd(first a, arg ... list){
    snd(a);
    snd(list...);
}

bool available(){
    
}
String rcv(){
    String value;
    rcv(value);
    return value;
}

enum class resetMode{
    waitReady,
    async,
};

void waitReady(){
    while(rcv() != AT_READY){
        ;
    }
}

bool isNotWakeup(){
    if (needWaitWeekup == false){
        return false;
    }
    if (available()){
        waitReady();
        needWaitWeekup = false;
        return false;
    }
    return true;
}


CMD(atTest)
    snd(AT_AT);
$

CMD(atReset, resetMode mode = resetMode::waitReady)
    snd(AT_RST);
    if (mode == resetMode::waitReady){
        waitReady();
        return success;
    }
    needWaitWeekup = true;
$

struct GMR{
    char atVersion[16];
    char sdkVersion[32];
    char compileTime[32];
    char binVersion[32];
};

CMD(atGMR, GMR * result)
    constexpr int length = 4;
    snd(AT_GMR);
    if (ask(AT_AT "AT version:%s(", result->atVersion) == ok &&
        ask(AT_AT "SDK version:%s", result->atVersion) == ok &&
        ask(AT_AT "compile time:%s", result->compileTime) == ok &&
        ask(AT_AT "Bin version:%s", result->binVersion) == ok
    );
$

CMD(atDeepSleep, size_t ms)
    needWaitWeekup = true;
    snd(AT_GSLP SET, ms);
    return success;
$

CMD(atEcho, bool enable)
    snd(AT_ATE, enable); //echo will be enable when reset
$

struct AtUartConfig {
    size_t   rate;
    uint8_t  databits;
    uint8_t  stopbits;
    uint8_t  parity;
    uint8_t  flowControl;
};

void uart(const char * header, AtUartConfig const & config){
    snd(
        header, SET, 
        config.rate, ',', 
        config.databits, ',',
        config.stopbits, ',',
        config.parity, ',', 
        config.flowControl
    );
}

CMD(atUartTemp, AtUartConfig const & config)
    uart(AT_UART_CUR, config);
$

CMD(atUartNovolatile, AtUartConfig const & config)
    uart(AT_UART_DEF, config);
$

CMD(atSleep, size_t enable)
    snd(AT_SLEEP SET, enable);
$

CMD(atSleep, size_t * value)
    snd(AT_SLEEP ASK);
    ask(AT_SLEEP TOKEN, value);
$

CMD(atRamSize, size_t * bytes)
    snd(AT_SYSRAM ASK);
    ask(AT_SYSRAM TOKEN, bytes);
$

struct FlashPartition{

};