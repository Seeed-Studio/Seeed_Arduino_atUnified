#pragma once
#include"UnifiedAtType.h"

// when current esp32 as station
// this structure is a connected ap item base infomation. 
struct WifiLinkedAp{
    Text    ssid;
    Mac     bssid;
    Ni8     channel;
    Ni8     rssi;
    void useAll(){
        bssid.isNull = false;
        channel.isNull = false;
        rssi.isNull = false;
    }
};

// when current esp32 as station
// this structure is a connectable ap item which from scan list.
struct WifiApItem{
    Text    ssid;
    Mac     bssid;
    Ni8     ecn;
    Ni8     rssi;
    Ni8     channel;
};

// when current esp32 as ap
// this structure is configure infomation for itself.
struct WifiApConfigure{
    Text    ssid;
    Text    pwd;
    Ni8     channel;
    Ni8     ecn;
    Ni8     maxConnect;
    Ni8     isSsidHidden;
};

// when current esp32 as ap
// this structure is a station base infomation which connected this ap.
struct WifiUser{
    Ipv4    ip;
    Mac     bssid;
};

struct IpRange{
    Ni32    leaseMinute;
    Ipv4    startIp;
    Ipv4    endIp;
};

bool atWifiMode(int32_t mode);
bool atWifiMode(int32_t * mode);
bool atWifiConnect(Text const & ssid, Text const & pwd, Mac const & bssid = nullref);
bool atWifiConnect(WifiLinkedAp * ap);
bool atWifiScan(Event whenScanFinished);
bool atWifiScan(Array<WifiApItem> * list);
bool atWifiDisconnect();
bool atWifiApConfigure(WifiApConfigure const & configure);
bool atWifiApConfigure(WifiApConfigure * configure);
bool atWifiUser(Array<WifiUser> * list);
bool atDhcp(bool enable, int32_t mask);
bool atDhcp(int32_t * result);
bool atDhcpIpRangeClear();
bool atDhcpIpRange(IpRange const & configure);
bool atDhcpIpRange(IpRange * configure);
bool atApAutoConnect(bool enable);
bool atApStartSmart(int32_t const & type = nullref);
bool atApStopSmart();
bool atWps(bool enable);
bool atStationHostName(Text const & name);
bool atStationHostName(Text * name);
bool atMdnsDisable();
bool atMdns(Text const & hostName, Text const & serviceName, int32_t port);

