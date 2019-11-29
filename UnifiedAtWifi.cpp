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

extern String readLine();

// mode
// - 0 : off FR
// - 1 : station mode
// - 2 : softap mode
// - 3 : both station and softap
CMD(atWifiMode, int32_t mode)
    tx("AT+CWMODE=%d", mode);
$

CMD(atWifiMode, int32_t * mode)
    tx("AT+CWMODE?");
    rx("+CWMODE:%d", mode);
$

CMD(atWifiConnect, String const & ssid, String const & pwd, Mac bssid)
    tx("AT+CWJAP=%s,%s%+m", ssid, pwd, bssid);
$

CMD(atWifiConnect, WifiLinkedAp * token)
    tx("AT+CWJAP?");
    rx("+CWJAP:%s,%s,%b,%b", token->ssid, token->bssid, token->channel, token->rssi);
$

void atWifiScanSubfunction(String current);

void atWifiScan(){
    tx("AT+CWLAP?");
}

// CMD(atWifiScan, std::vector<WifiApItem> & list)
//     String current;
//     atWifiScan();
//     atWifiScanSubfunction(readLine());
// $

CMD(atWifiScanAsync, std::function<void ()> const & callback)
    atWifiScan();
    esp.wifi.whenFinishScan = callback;
    return success;
$

CMD(atWifiDisconnect)
    tx("AT+CWQAP");
$

CMD(atWifiApConfigure, WifiApConfigure const & configure)
    tx("AT+CWSAP=%s,%s,%d,%d%+d%+d", CWSAP_ARG(&));
$

CMD(atWifiApConfigure, WifiApConfigure * configure)
    tx("AT+CWSAP?");
    rx("+CWSAP:%s,%s,%d,%d,%d,%d", CWSAP_ARG());
$

CMD(atWifiUser, std::vector<WifiUser> & list)
    WifiUser item;
    for (tx("AT+CWLIF?"); rx("+CWLIF:%i,%m", item.ip, item.bssid) == success; ){
        list.push_back(item);
    }
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
    tx("AT+CWDHCP=%b,%d", enable, mask);
$

CMD(atDhcp, int32_t * mask)
    tx("AT+CWDHCP?");
    rx("+CWDHCP:%d", mask);
$

CMD(atDhcpIpRangeClear)
    tx("AT+CWDHCPS=0");
$

CMD(atDhcpIpRange, IpRange const & configure)
    tx("AT+CWDHCPS:1,%d,%i,%i", CWDHCPS_ARG(&));
$

CMD(atDhcpIpRange, IpRange * configure)
    tx("AT+CWDHCPS?");
    rx("+CWDHCPS:%d,%i,%i", CWDHCPS_ARG());
$

CMD(atApAutoConnect, bool enable)
    tx("AT+CWAUTOCONN=%b", enable);
$

CMD(atApStartSmart, int32_t type)
    ta("AT+CWSTARTSMART");
    type != leaveOut && 
    ta("=%d", type);
    tx(""); // END LINE
$

CMD(atApStopSmart)
    tx("AT+CWSTOPSMART");
$

CMD(atWps, bool enable)
    tx("AT+WPS=%b", enable);
$

CMD(atStationHostName, String const & name)
    tx("AT+CWHOSTNAME=%s", name);
$

CMD(atStationHostName, String * name)
    tx("AT+CWHOSTNAME?");
    rx("+CWHOSTNAME:%s", name);
$

CMD(atMdnsDisable)
    tx("AT+MDNS=0");
$

CMD(atMdns, String hostName, String serviceName, int32_t port)
    tx("AT+MDNS=1,%s,%s,%d", hostName, String("_") + serviceName, port);
$

