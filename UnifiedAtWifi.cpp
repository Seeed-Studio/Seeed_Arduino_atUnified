#include"UnifiedAtWifi.h"

#define SET_CWMODE              "AT+CWMODE=%d", a0
#define ASK_CWMODE              "AT+CWMODE?"
#define GET_CWMODE              "+CWMODE:%d", a0

#define SET_CWJAP               "AT+CWJAP=%s,%s%+s", a0.ssid, a0.pwd, a0.bssid
#define ASK_CWJAP               "AT+CWJAP?"
#define GET_CWJAP               "+CWJAP:%s,%s,%d,%d", a0->ssid, a0->bssid, a0->channel, a0->rssi

#define ASK_CWLAP               "AT+CWLAP?"
#define GET_CWLAP               "+CWLAP:"

#define SET_CWQAP               "AT+CWQAP"

#define CWSAP_ARG(...)                  \
        (__VA_ARGS__ a0)->ssid,         \
        (__VA_ARGS__ a0)->pwd,          \
        (__VA_ARGS__ a0)->channel,      \
        (__VA_ARGS__ a0)->ecn,          \
        (__VA_ARGS__ a0)->maxConnect,   \
        (__VA_ARGS__ a0)->isSsidHidden
#define SET_CWSAP               "AT+CWSAP=%s,%s,%d,%d%+d%+d", CWSAP_ARG(&)
#define ASK_CWSAP               "AT+CWSAP?"
#define GET_CWSAP               "+CWSAP:%s,%s,%d,%d,%d,%d", CWSAP_ARG()

#define ASK_CWLIF               "AT+CWLIF?"
#define GET_CWLIF               "+CWLIF:%i,%m", a0.ip, a0.mac

#define SET_CWDHCP              "AT+CWDHCP=%d,%d", a0, a1
#define ASK_CWDHCP              "AT+CWDHCP?"
#define GET_CWDHCP              "+CWDHCP:%d", a0

#define CWDHCPS_ARG(...)                \
        (__VA_ARGS__ a0)->leaseMinute,  \
        (__VA_ARGS__ a0)->startIp,      \
        (__VA_ARGS__ a0)->endIp

#define SET_CWDHCPS_DISABLE     "AT+CWDHCPS=0"
#define SET_CWDHCPS             "AT+CWDHCPS:1,%d,%i,%i", CWDHCPS_ARG(&)
#define ASK_CWDHCPS             "AT+CWDHCPS?"
#define GET_CWDHCPS             "+CWDHCPS:%d,%i,%i", CWDHCPS_ARG()

#define SET_CWAUTOCONN          "AT+CWAUTOCONN=%d", a0
#define SET_CWSTARTSMART        "AT+CWSTARTSMART=%d", a0
#define SET_CWSTOPSMART         "AT+CWSTOPSMART"
#define SET_WPS                 "AT+WPS=%d", a0
#define SET_CWHOSTNAME          "AT+CWHOSTNAME=%s", a0
#define ASK_CWHOSTNAME          "AT+CWHOSTNAME?"
#define GET_CWHOSTNAME          "+CWHOSTNAME:%s", a0
#define SET_MDNS_DISABLE        "AT+MDNS=0"
#define SET_MDNS                "AT+MDNS=1,%s,%s,%d", a0, String("_") + a1, a2

extern String readLine();

CMD(atWifiMode, int32_t a0)
    tx(SET_CWMODE);
$
CMD(atWifiMode, int32_t * a0)
    tx(ASK_CWMODE);
    rx(GET_CWMODE);
$

CMD(atWifiConnect, WifiToken const & a0)
    tx(SET_CWJAP);
$

CMD(atWifiConnect, WifiLinkedAp * a0)
    tx(ASK_CWJAP);
    tx(GET_CWJAP);
$

CMD(atWifiScan, std::function<void (WifiLinkInfo &)> && call)
    String current;
    tx(ASK_CWLAP);
    while((current = readLine()).length() == 0){
        delay(1);
    }
    auto lastIndexOf = [](const char * line, char chr, size_t times){
        for (int i = strlen(line); i-- > 0; ){
            if (line[i] != chr){
                continue;
            }
            if (--times == 0){
                return i;
            }
        }
        return -1;
    };

    while (current.indexOf(GET_CWLAP) != -1){
        //char    line[] = "+CWLAP:(3,\"POT\",-AL00a\",-53,\"12:34:56:78:9a:bc\",1)";
        char *  line = (char *)current.c_str();
        char *  p = nullptr;
        char *  t;
        int32_t mac[6]; //need 32bit when use sscanf
        int32_t ecn;
        int32_t rssi;
        int32_t channel;

        p = line + lastIndexOf(line, ',', 3);

        sscanf(line, GET_CWLAP "(%d,", & ecn);
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
    tx(SET_CWQAP);
$

CMD(atWifiApConfigure, WifiApConfigure const & a0)
    tx(SET_CWSAP);
$

CMD(atWifiApConfigure, WifiApConfigure * a0)
    tx(ASK_CWSAP);
    rx(GET_CWSAP);
$

CMD(atWifiUserList, std::function<void (WifiUserList &)> && call)
    WifiUserList a0;
    for (tx(ASK_CWLIF); rx(GET_CWLIF) == success; ){
        call(a0);
    }
$

CMD(atDhcp, bool a0, int32_t a1)
    tx(SET_CWDHCP);
$

CMD(atDhcp, int32_t * a0)
    tx(ASK_CWDHCP);
    rx(GET_CWDHCP);
$

CMD(atDhcpIpRangeClear)
    tx(SET_CWDHCPS_DISABLE);
$

CMD(atDhcpIpRange, IpRange const & a0)
    tx(SET_CWDHCPS);
$

CMD(atDhcpIpRange, IpRange * a0)
    tx(ASK_CWDHCPS);
    rx(GET_CWDHCPS);
$

CMD(atApAutoConnect, bool a0)
    tx(SET_CWAUTOCONN);
$

CMD(atApStartSmart, int32_t a0)
    tx(SET_CWSTARTSMART);
$

CMD(atApStopSmart)
    tx(SET_CWSTOPSMART);
$

CMD(atWps, bool a0)
    tx(SET_WPS);
$

CMD(atHostNameTemp, String const & a0)
    tx(SET_CWHOSTNAME);
$

CMD(atHostNameTemp, String * a0)
    tx(ASK_CWHOSTNAME);
    rx(GET_CWHOSTNAME);
$

CMD(atMdnsDisable)
    tx(SET_MDNS_DISABLE);
$

CMD(atMdns, String a0, String a1, int32_t a2)
    tx(SET_MDNS);
$
