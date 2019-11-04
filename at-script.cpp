#pragma once
#ifdef ARDUINO
#include<WString.h>
#else
#include<string>
typedef std::string String;
#endif
#include<stdio.h>
#include<string.h>

#define AT              "AT"
#define RST             "+RST"
#define GMR             "+GMR"
#define ATE             "ATE"
#define GSLP            "+GSLP"
#define SLEEP           "+SLEEP"
#define UART_CUR        "+UART_CUR"
#define UART_DEF        "+UART_DEF"
#define SYSRAM          "+SYSRAM"
#define SYSFLASH     
#define FS           
#define RFPOWER         "+RFPOWER"

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
#define debug(...)       Serial.printf(__VA_ARGS__);

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
        //debug(line.c_str());
        return line;
    }
    void write(const char * value){
        com.print(value);
        debug(value);
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

enum class hex_t : int32_t {};

struct any{
    any(int32_t * v) : v(v), isHex(false), skip(0){}
    any(hex_t * v)   : v(v), isHex(true ), skip(0){}
    any(String * v)  : v(v), isHex(false), skip(0){}
    any(const char * v) :    isHex(false), skip(strlen(v)){}
    any(char v) :    isHex(false), skip(1){}
    template<class type> void set(type const & value){
        *(type *)v = value;
    }
    void *  v;
    uint8_t skip;
    bool    isHex;
};

template<class ... arg>
bool subrx(const char * prefix, any * list){
    String  resp = readLine();
    auto    str = (char *)resp.c_str();
    auto    len = strlen(prefix);

    if (resp.indexOf(prefix) != 0){
        return fail;
    }

    str += len;
    
    while(str[0]){
        if (str[0] == ','){
            str += 1;
            continue;
        }
        if (list->skip){
            str += list->skip;
            list += 1;
        }
        if (str[0] == '\"'){
            str += 1;
            auto len = strchr(str, '\"') - str;
            str[len] = '\0';
            list->set<String>(str);
            str += len + 1;
        }
        else{
            auto end = str;
            for (; isxdigit(end[0]); end++){
                ;
            }
            sscanf(str, list->isHex ? "%x" : "%d", list->v);
            str = end;
        }

        //DON't do this: for(;; list += 1)
        list += 1;
    }
    return success;
}

template<class ... arg>
bool rx(char * prefix, arg ... list){
    any     ls[] = { list... };
    any   * v = ls;
    return subrx(prefix, v);
}

enum class txMore{ };
constexpr txMore more = txMore(0);

void write(int32_t value){
    char buf[sizeof(int32_t) * 8];
    sprintf(buf, "%d", value);
    write(buf);
}

void write(String const & value){
    write(value.c_str());
}

void tx(txMore){
}

void tx(){
    write("\r\n");
}

template<class first, class ... arg>
void tx(first a, arg ... list){
    write(a);
    if (sizeof...(list)){
        write(",");
    }
    tx(list...);
}

enum class actionMode{
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
    tx(AT);
$

CMD(atReset, actionMode mode = actionMode::waitReady)
    tx(AT RST);
    if (mode == actionMode::waitReady){
        waitReady();
        return success;
    }
    needWaitWeekup = true;
$

class FirmwareInfo{
public:
    String atVersion;
    String sdkVersion;
    String compileTime;
    String binVersion;
};

CMD(atFirmwareInfo, FirmwareInfo * result)
    tx(AT GMR);
    if (rx("AT version:",   & result->atVersion)   == ok &&
        rx("SDK version:",  & result->atVersion)   == ok &&
        rx("compile time:", & result->compileTime) == ok &&
        rx("Bin version:",  & result->binVersion)  == ok
    );
$

CMD(atDeepSleep, int32_t ms)
    needWaitWeekup = true;
    tx(AT GSLP SET, ms);
    return success;
$

//DON'T USE -> CMD(atEcho, bool enable)
//BUT -> bool atEcho(bool enable)
bool atEcho(bool enable){
    //echo will be enable when reset
    //THIS CMD NEED NOT 'SET' SUFIX
    //AND DON'T USE CMD(atEcho, bool enable) FORMAT, IT WILL RESULT RECURRENCE.
    if (isNotWakeup()) return fail;
    tx(enable ? ATE "1" : ATE "0"); 
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
    tx(
        header,
        config.rate, 
        config.databits, 
        config.stopbits, 
        config.parity, 
        config.flowControl
    );
}

CMD(atUartTemp, AtUartConfig const & config)
    uart(AT UART_CUR SET, config);
$

CMD(atUartNovolatile, AtUartConfig const & config)
    uart(AT UART_DEF SET, config);
$

CMD(atSleepMode, int32_t enable)
    tx(AT SLEEP SET, enable);
$

CMD(atSleepMode, int32_t * value)
    tx(AT SLEEP ASK);
    rx(AT SLEEP TOKEN, value);
$

CMD(atRamSize, int32_t * bytes)
    tx(AT SYSRAM ASK);
    rx(SYSRAM TOKEN, bytes);
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
    tx(AT RFPOWER SET, wifi);
$

CMD(atRfPower, RfPower const & config)
    tx(AT RFPOWER SET, 
        config.wifiPower,
        config.bleAdvPower,
        config.bleScanPower,
        config.bleConnPower
    );
$

CMD(atRfPower, RfPower * config)
    tx(AT RFPOWER ASK);
    rx(RFPOWER TOKEN, 
        & config->wifiPower,
        & config->bleAdvPower,
        & config->bleScanPower,
        & config->bleConnPower
    );
$

#define CWMODE       "+CWMODE"
#define CWJAP        "+CWJAP"
#define CWLAP        "+CWLAP"
#define CWQAP        "+CWQAP"
#define CWSAP        "+CWSAP"
#define CWLIF        "+CWLIF"

CMD(atWifiMode, int32_t mode)
    tx(AT CWMODE SET, mode);
$
CMD(atWifiMode, int32_t * mode)
    tx(AT CWMODE ASK);
    rx(CWMODE TOKEN, mode);
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
        tx(AT CWJAP SET, ssid, pwd);
    }
    else{
        tx(AT CWJAP SET, ssid, pwd, bssid);
    }
$

template<class callback>
CMD(atWifiScan, callback && call)
    String current;
    tx(AT CWLAP);
    while((current = readLine()).length() == 0){
        delay(1);
    }
    while (current.indexOf(CWLAP TOKEN) != -1){

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

        for (size_t i = 0, length = strlen(line); length-- > 0; ) {
            t = p;
            p = strrchr(line, ',');
            p[0] = '\0';
            t[0] = ',';
        }
        sscanf(line, CWLAP TOKEN "(%d,", & ecn);
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

CMD(atWifiDisconnect)
    tx(AT CWQAP);
$

class WifiApConfigure{
public:
    String  ssid;
    String  pwd;
    int32_t channel;
    int32_t ecn;
    int32_t maxConnect;
    int32_t isSsidHidden;
    WifiApConfigure() : 
        maxConnect(-1),
        isSsidHidden(-1){
    }
};

CMD(atWifiApConfigure, WifiApConfigure * configure)
    tx(AT CWSAP ASK);
    rx(CWSAP TOKEN, 
        & configure->ssid,
        & configure->pwd,
        & configure->channel,
        & configure->ecn,
        & configure->maxConnect,
        & configure->isSsidHidden
    );
$

CMD(atWifiApConfigure, WifiApConfigure const & configure)
    tx(AT CWSAP SET,
        configure.ssid,
        configure.pwd,
        configure.channel,
        configure.ecn,
        more
    );
    if (configure.maxConnect == -1){
        tx();
    }
    else if (configure.isSsidHidden == -1){
        tx(configure.maxConnect);
    }
    else{
        tx(configure.maxConnect, configure.isSsidHidden);
    }
$

class WifiUserList{
public:
    uint8_t ip[4];
    uint8_t mac[6];
};

template<class callback>
CMD(atWifiUserList, callback && call)
    int32_t ip[4];
    hex_t   mac[6];
    tx(AT CWLIF);
    while (rx(CWLIF TOKEN, 
        ip + 0, '.', ip + 1, '.', ip + 2, '.', ip + 3,
        mac + 0, ':', mac + 1, ':', mac + 2, ':', mac + 3, ':', mac + 4, ':', mac + 5
    ) == success){
        WifiUserList user;
        for (size_t i = 0; i < 4; i++){
            user.ip[i] = ip[i];
        }
        for (size_t i = 0; i < 6; i++){
            user.mac[i] = (uint8_t)mac[i];
        }
        call(user);
    }
$
