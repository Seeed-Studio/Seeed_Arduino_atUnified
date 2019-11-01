#pragma once
#ifdef ARDUINO
#include<WString.h>
#else
#include<string>
typedef std::string String;
#endif
#include<stdio.h>
#include<string.h>

#define AT_AT           "AT"
#define AT_RST          "AT+RST"
#define AT_GMR          "AT+GMR"
#define AT_ATE          "ATE"
#define AT_GSLP         "AT+GSLP"
#define AT_SLEEP        "AT+SLEEP"
#define AT_UART_CUR     "AT+UART_CUR"
#define AT_UART_DEF     "AT+UART_DEF"
#define AT_SYSRAM       "AT+SYSRAM"
#define AT_SYSFLASH     
#define AT_FS           
#define AT_RFPOWER      "AT+RFPOWER"

#define END_LINE        "\r\n"
#define TOKEN           ":"
#define TOKEN_OFFSET    2
#define AT_ERROR        "ERROR"
#define AT_OK           "OK"
#define AT_READY        "ready"
#define SET             "="
#define ASK             "?"
#define CMD(name,...)                       \
inline bool name(__VA_ARGS__) {             \
    if (isNotWakeup()) return fail;         \
    if (atEcho(false) == fail) return fail; \

#define $                return waitFlag(); }
#define debug(...)       //Serial.printf(__VA_ARGS__);

constexpr bool fail = false;
constexpr bool success = true;
volatile  bool needWaitWeekup = false;

#define USE_AT_SERIAL1

#ifdef USE_AT_SERIAL
    #define USE_SERIAL_COM
    auto & com = Serial;
#elif defined USE_AT_SERIAL1
    #define USE_SERIAL_COM
    auto & com = Serial1;
#elif defined USE_AT_SERIAL2
    #define USE_SERIAL_COM
    auto & com = Seria2;
#endif

bool atTest();

#ifdef USE_SERIAL_COM
    bool available(){
        return com.available() != 0;
    }
    String readLine(){
        String && line = com.readStringUntil('\n');
        if (line.length() == 0){
            return line;
        }
        line = line.substring(0, line.length() - 1);
        debug(line.c_str());
        return line;
    }
    void write(const char * value){
        com.print(value);
    }
    bool atBegin(){
        com.begin(115200);
        return atTest();
    }
#endif

enum state{
    ok,
    over,
};

bool waitFlag(){
    while(true){
        String ack = readLine();
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
    auto && resp = readLine();

    if (sscanf(resp.c_str(), fmt + TOKEN_OFFSET, list...) == sizeof...(list)){
        return ok;
    }
    return over;
}

void snd(){
    write("\r\n");
}
void write(int32_t value){
    char buf[sizeof(int32_t) * 8];
    sprintf(buf, "%d", value);
    write(buf);
}
template<class first, class ... arg>
void snd(first a, arg ... list){
    write(a);
    snd(list...);
}

enum class resetMode{
    waitReady,
    async,
};

void waitReady(){
    while(readLine() != AT_READY){
        ;
    }
}
bool atEcho(bool enable);
bool isNotWakeup(){
    debug("isNotWakeup\n");
    if (needWaitWeekup == false){
        return false;
    }
    debug("isNotWakeup\n");
    if (available()){
        waitReady();
        needWaitWeekup = false;
        return false;
    }
    debug("return\n");
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

class GMR{
public:
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

CMD(atDeepSleep, int32_t ms)
    needWaitWeekup = true;
    snd(AT_GSLP SET, ms);
    return success;
$

//DON'T USE -> CMD(atEcho, bool enable)
//BUT -> bool atEcho(bool enable)
bool atEcho(bool enable){
    //echo will be enable when reset
    //THIS CMD NEED NOT 'SET' SUBFIX
    //AND DON'T USE CMD(atEcho, bool enable) FORMAT, IT WILL RESULT RECURRENCE.
    if (isNotWakeup()) return fail;
    snd(AT_ATE, enable);
    return waitFlag();
}

class AtUartConfig {
public:
    int32_t   rate;
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

CMD(atSleepMode, int32_t enable)
    snd(AT_SLEEP SET, enable);
$

CMD(atSleepMode, int32_t * value)
    snd(AT_SLEEP ASK);
    ask(AT_SLEEP TOKEN "%d", value);
$

CMD(atRamSize, int32_t * bytes)
    snd(AT_SYSRAM ASK);
    ask(AT_SYSRAM TOKEN "%d", bytes);
$

// struct FlashPartition{

// };

class RfPower{
public:
    int32_t wifiPower;      //maybe is negtive number
    int32_t bleAdvPower;
    int32_t bleScanPower;
    int32_t bleConnPower;
};

CMD(atRfPower, int32_t wifi)
    snd(AT_RFPOWER SET, wifi);
$

CMD(atRfPower, RfPower const & config)
    snd(AT_RFPOWER SET, 
        config.wifiPower, ",",
        config.bleAdvPower,  ",",
        config.bleScanPower, ",",
        config.bleConnPower
    );
$

CMD(atRfPower, RfPower * config)
    snd(AT_RFPOWER ASK);
    ask(AT_RFPOWER TOKEN "%d,%d,%d,%d", 
        & config->wifiPower,
        & config->bleAdvPower,
        & config->bleScanPower,
        & config->bleConnPower
    );
$

#define AT_CWMODE       "AT+CWMODE"
#define AT_CWJAP        "AT+CWJAP"
#define AT_CWLAP        "AT+CWLAP"

CMD(atWifiMode, int32_t mode)
    snd(AT_CWMODE SET, mode);
$
CMD(atWifiMode, int32_t * mode)
    snd(AT_CWMODE ASK);
    ask(AT_CWMODE TOKEN, mode);
$

class WifiLinkInfo{
public:
    int32_t ecn;
    String  ssid;
    int8_t  rssi;
    uint8_t mac[6];
    uint8_t channel;
};

CMD(atWifiConnect, 
    const char * ssid, 
    const char * pwd, 
    const char * bssid = "")
    if (bssid == ""){
        snd(AT_CWJAP SET,  ssid, ",", pwd);
    }
    else{
        snd(AT_CWJAP SET, ssid, ",", pwd, ",", bssid);
    }
$

template<class callback>
CMD(atWifiConnect, callback && call)
    String token = AT_CWLAP TOKEN + TOKEN_OFFSET;
    String current;
    snd(AT_CWLAP);
    while((current = readLine()).length() == 0){
        delay(1);
    }
    
    while (current.indexOf(token) != -1){
        //char    line[] = "+CWLAP:(3,\"POT\",-AL00a\",-53,\"12:34:56:78:9a:bc\",1)";
        char *  line = (char *)current.c_str();
        char *  p = NULL;
        char *  t;
        int32_t mac[6]; //need 32bit when use sscanf
        int32_t ecn;
        int32_t rssi;
        int32_t channel;
        p = strrchr(line, ',');
        p[0] = '\0';

        for (size_t i = 0; i < 2; i++) {
            t = p;
            p = strrchr(line, ',');
            p[0] = '\0';
            t[0] = ',';
        }
        sscanf(line, "+CWLAP:(%d,", & ecn);
        sscanf(p + 1, "%d,\"%x:%x:%x:%x:%x:%x\",%d", 
            & rssi,
            & mac[0],
            & mac[1],
            & mac[2],
            & mac[3],
            & mac[4],
            & mac[5],
            & channel
        );

        //replace '\"' to '\0'
        p[0] = ',';
        p[-1] = '\0';

        WifiLinkInfo info;

        info.ecn = ecn;
        info.ssid = strchr(line, ',') + 2; //skip ',' '\"'
        info.rssi = rssi;
        for (size_t i = 0; i < 6; i++) {
            info.mac[i] = (uint8_t)mac[i];
        }
        info.channel = channel;
        call(info);
        current = readLine();
    }
$