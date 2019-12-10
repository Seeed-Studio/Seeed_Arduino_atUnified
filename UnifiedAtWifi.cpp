#include"UnifiedAtWifi.h"
#include"UnifiedAtEvent.h"
#define CWSAP_ARG(...)                          \
        (__VA_ARGS__ configure)->ssid,          \
        (__VA_ARGS__ configure)->pwd,           \
        (__VA_ARGS__ configure)->channel,       \
        (__VA_ARGS__ configure)->ecn,           \
        (__VA_ARGS__ configure)->maxConnect,    \
        (__VA_ARGS__ configure)->isSsidHidden

#define CWDHCPS_ARG(...)                        \
        (__VA_ARGS__ configure)->leaseMinute,   \
        (__VA_ARGS__ configure)->startIp,       \
        (__VA_ARGS__ configure)->endIp

extern Text readLine();

// mode
// - 0 : off FR
// - 1 : station mode
// - 2 : softap mode
// - 3 : both station and softap
CMD(atWifiMode, int32_t mode)
    tx("AT+CWMODE=", mode);
$

CMD(atWifiMode, int32_t * mode)
    tx("AT+CWMODE?");
    rx("+CWMODE:", mode);
$

CMD(atWifiConnect, Text const & ssid, Text const & pwd, Mac const & bssid)
    tx("AT+CWJAP=", ssid, pwd, bssid);
$

CMD(atWifiConnect, WifiLinkedAp * token)
    tx("AT+CWJAP?");
    rx("+CWJAP:", & token->ssid, & token->bssid, & token->channel, & token->rssi);
$

bool atWifiScan(Event whenScanFinished){
    // aynsc need static item.
    static WifiApItem item;

    // block if cmd not finished.
    tx("AT+CWLAP"); // no '?'
    rx("+CWLAP:", & item.ecn, & item.ssid, & item.rssi, & item.bssid, & item.channel);
    
    // set after
    esp.wait = WaitWifiScan;
    esp.wifi.whenScanFinished = whenScanFinished;
    esp.wifi.apListEnd = esp.wifi.apList;
    esp.analysis.whenResolutionOneLine = [&](){ 
        esp.wifi.apListEnd[0] = item; 
        esp.wifi.apListEnd += 1;
    };
    flush();
    return Success;
}

bool atWifiScan(Array<WifiApItem> * list){
    atWifiScan([](){});
    auto r = waitFlag();
    if (r == Success){
        list[0] = Array<WifiApItem>(esp.wifi.apList, esp.wifi.apListEnd - esp.wifi.apList);
    }
    return r;
}

CMD(atWifiDisconnect)
    tx("AT+CWQAP");
$

CMD(atWifiApConfigure, WifiApConfigure const & configure)
    tx("AT+CWSAP=", CWSAP_ARG(&));
$

CMD(atWifiApConfigure, WifiApConfigure * configure)
    tx("AT+CWSAP?");
    rx("+CWSAP:", CWSAP_ARG());
$

CMD(atWifiUser, Array<WifiUser> * list)
    static WifiUser item;
    tx("AT+CWLIF"); // no '?'

    // rx ap user list
    rx("+CWLIF:", & item.ip, & item.bssid);

    // set after tx
    esp.wifi.apUserEnd = esp.wifi.apUser;
    esp.analysis.whenResolutionOneLine = [&](){ 
        esp.wifi.apUserEnd[0] = item;
        esp.wifi.apUserEnd += 1;
    };

    flush();
    auto r = waitFlag();
    if (r == Success){
        list[0] = Array<WifiUser>(esp.wifi.apUser, esp.wifi.apUserEnd - esp.wifi.apUser);
    }
    return r;
$

// mask
// - bit 0 : station DHCP
// - bit 1 : softap DHCP
//
// example
// enable station DHCP
// AT+CWDHCP=1,1
// 
// diable station DHCP
// AT+CWDHCP=0,1
//
// enable softap DHCP
// AT+CWDHCP=1,2
// 
// diable softap DHCP
// AT+CWDHCP=0,2
//
// enable station and softap DHCP
// AT+CWDHCP=1,3
// 
// diable station and softap DHCP
// AT+CWDHCP=0,2
CMD(atDhcp, bool enable, int32_t mask)
    tx("AT+CWDHCP=", enable, mask);
$

CMD(atDhcp, int32_t * mask)
    tx("AT+CWDHCP?");
    rx("+CWDHCP:", mask);
$

CMD(atDhcpIpRangeClear)
    tx("AT+CWDHCPS=0");
$

CMD(atDhcpIpRange, IpRange const & configure)
    tx("AT+CWDHCPS:1", CWDHCPS_ARG(&));
$

CMD(atDhcpIpRange, IpRange * configure)
    tx("AT+CWDHCPS?");
    rx("+CWDHCPS:", CWDHCPS_ARG());
$

CMD(atApAutoConnect, bool enable)
    tx("AT+CWAUTOCONN=", enable);
$

CMD(atApStartSmart, int32_t const & type)
    if (type == nullref){
        tx("AT+CWSTARTSMART");
    }
    else{
        tx("AT+CWSTARTSMART=", type);
    }
$

CMD(atApStopSmart)
    tx("AT+CWSTOPSMART");
$

CMD(atWps, bool enable)
    tx("AT+WPS=", enable);
$

CMD(atStationHostName, Text const & name)
    tx("AT+CWHOSTNAME=", name);
$

CMD(atStationHostName, Text * name)
    tx("AT+CWHOSTNAME?");
    rx("+CWHOSTNAME:", name);
$

CMD(atMdnsDisable)
    tx("AT+MDNS=0");
$

CMD(atMdns, Text const & hostName, Text const & serviceName, int32_t port)
    tx("AT+MDNS=1,", hostName, Text("_") + serviceName, port);
$

