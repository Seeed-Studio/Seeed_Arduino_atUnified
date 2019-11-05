#pragma once
#ifdef ARDUINO
#include<WString.h>
#else
#include<string>
typedef std::string String;
#endif
#include<stdio.h>
#include<string.h>


#define CMD(name,...)                       \
inline bool name(__VA_ARGS__) {             \
    if (isNotWakeup()) return fail;         \
    if (atEcho(false) == fail) return fail; \

#define $                return waitFlag(); }
#define debug(...)       Serial.printf(__VA_ARGS__);


constexpr bool disable = false;
constexpr bool enable = true;

constexpr bool fail = false;
constexpr bool success = true;
volatile  bool needWaitWeekup = false;
bool           idEndLine = true;

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
        bool atTest();
        com.begin(115200);
        while(readLine().length());
        return atTest();
    }
#endif

bool waitFlag(){
    while(true){
        String ack = readLine();
        //debug("E:%s\n", ack.c_str());
        if (ack == AT_OK){
            return success;
        }
        if (ack == AT_ERROR){
            return fail;
        }
    }
}

enum hex_t : int32_t {};
enum mac_t : uint8_t {};
enum ip_t : uint8_t {};

enum any_type{
    none,
    digital,
    hex,
    string,
    ip,
};

struct any{
    any(ip_t * v)    : v(v), type(ip),      skip(0){}
    any(hex_t * v)   : v(v), type(hex),     skip(0){}
    any(int32_t * v) : v(v), type(digital), skip(0){}
    any(String * v)  : v(v), type(string),  skip(0){}
    any(const char * v) :    type(none),    skip(strlen(v)){}
    any(char v) :            type(none),    skip(1){}
    template<class type> 
    void set(type const & value){
        *(type *)v = value;
    }
    void *   v;
    uint8_t  skip;
    any_type type;
};


template<class ... arg>
bool rx(char * prefix, arg ... list){
    any     ls[] = { list... };
    return subrx(prefix, ls);
}

enum class txMore{ };
constexpr txMore more = txMore(0);

void write(int32_t value){
    char buf[sizeof(int32_t) * 8];
    sprintf(buf, "%d", value);
    write(buf);
}

void write(String const & value){
    write("\"");
    write(value.c_str());
    write("\"");
}

void write(ip_t * ip){
    write(ip[0]); write(".");
    write(ip[1]); write(".");
    write(ip[2]); write(".");
    write(ip[3]);
}

void write(mac_t one){
    const char * map = "0123456789abcdef";
    char buf[] = { 
        map[one & 0xf], 
        map[one >> 0xf], '\0' };
    write(buf);
}

void write(mac_t * mac){
    write(mac[0]); write(":");
    write(mac[1]); write(":");
    write(mac[2]); write(":");
    write(mac[3]); write(":");
    write(mac[4]); write(":");
    write(mac[5]); write(":");
    write(mac[6]); write(":");
    write(mac[7]);
}

void tx(txMore){}

void tx(){
    write("\r\n");
    idEndLine = true;
}


template<class first, class ... arg>
void tx(first a, arg ... list){
    write(a);
    if (idEndLine){
        idEndLine = false;
    }
    else if (sizeof...(list)){
        write(",");
    }
    tx(list...);
}

enum class actionMode{
    waitReady,
    async,
};

template<class a, class b>
void copy(a * des, b const * src, size_t length){
    while (length--) {
        des[length] = (a)src[length];
    }
}

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
    int32_t  rate;
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
#define CWDHCP       "+CWDHCP"
#define CWDHCPS      "+CWDHCPS"
#define CWAUTOCONN   "+CWAUTOCONN"
#define CWSTARTSMAR  "+CWSTARTSMAR"
#define CWSTOPSMART  "+CWSTOPSMART"
#define WPS          "+WPS"
#define CWHOSTNAME   "+CWHOSTNAME"
#define MDNS         "+MDNS"

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
    String ssid, 
    String pwd, 
    String bssid = "")
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
        copy(info.mac, mac, 6);
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
        copy(user.ip, ip, 4);
        copy(user.mac, mac, 6);
        call(user);
    }
$

CMD(atDhcp, bool enable, int32_t mask)
    tx(AT CWDHCP SET, enable, mask);
$

CMD(atDhcp, int32_t * result)
    tx(AT CWDHCP ASK);
    rx(CWDHCP TOKEN, result);
$

CMD(atDhcpIpRangeClear)
    tx(AT CWDHCPS SET, disable);
$

class IpRange{
public:
    int32_t leaseMinute;
    uint8_t startIp[4];
    uint8_t endIp[4];
};

CMD(atDhcpIpRange, IpRange const & configure)
    //xxx.xxx.xxx.xxx -> 12 + 1 bytes
    char startIp[12 + 1];
    char endIp[12 + 1];
    sprintf(startIp, "%d.%d.%d.%d", 
        configure.startIp[0], 
        configure.startIp[1], 
        configure.startIp[2], 
        configure.startIp[3]
    );
    sprintf(endIp, "%d.%d.%d.%d", 
        configure.endIp[0], 
        configure.endIp[1], 
        configure.endIp[2], 
        configure.endIp[3]
    );
    tx(AT CWDHCPS SET, enable, configure.leaseMinute, startIp, endIp);
$

CMD(atDhcpIpRange, IpRange * configure)
    int32_t startIp[4];
    int32_t endIp[4];
    tx(AT CWDHCPS ASK);
    if (rx(CWDHCPS TOKEN,
        & configure->leaseMinute, 
        & startIp[0], '.',
        & startIp[1], '.',
        & startIp[2], '.',
        & startIp[3],
        & endIp[0], '.',
        & endIp[1], '.',
        & endIp[2], '.',
        & endIp[3]) == success){
        copy(configure->startIp, startIp, 4);
        copy(configure->endIp, endIp, 4);
    }
$

CMD(atApAutoConnect, bool enable)
    tx(AT CWAUTOCONN SET, enable);
$

CMD(atApStartSmart, int32_t type)
    tx(AT CWSTARTSMAR SET, type);
$

CMD(atApStopSmart, int32_t type)
    tx(AT CWSTOPSMART SET, type);
$

CMD(atWps, bool enable)
    tx(AT WPS SET, enable);
$

CMD(atHostNameTemp, String const & name)
    tx(AT CWHOSTNAME SET, name);
$

CMD(atHostNameTemp, String * name)
    tx(AT CWHOSTNAME ASK);
    rx(CWHOSTNAME TOKEN, name);
$

CMD(atMdnsDisable)
    tx(AT MDNS SET, disable);
$

CMD(atMdns, String hostName, String serviceName, int32_t port)
    tx(AT MDNS SET, enable, hostName, String("_") + serviceName, port);
$
