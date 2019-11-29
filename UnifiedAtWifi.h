#pragma once
#include"UnifiedAtType.h"

// when current esp32 as station
// this structure is a connected ap item base infomation. 
struct WifiLinkedAp{
    String  ssid;
    Mac     bssid;
    int8_t  channel;
    int8_t  rssi;
};

// when current esp32 as station
// this structure is a connectable ap item which from scan list.
struct WifiApItem{
    String  ssid;
    Mac     bssid;
    int8_t  ecn;
    int8_t  rssi;
    int8_t  channel;
};

// when current esp32 as ap
// this structure is configure infomation for itself.
struct WifiApConfigure{
    String  ssid;
    String  pwd;
    int32_t channel;
    int32_t ecn;
    int32_t maxConnect;
    int32_t isSsidHidden;
    WifiApConfigure() : 
        maxConnect(leaveOut),
        isSsidHidden(leaveOut){
    }
};

// when current esp32 as ap
// this structure is a station base infomation which connected this ap.
struct WifiUser{
    Ipv4    ip;
    Mac     bssid;
};

struct IpRange{
    // 
    int32_t leaseMinute;
    Ipv4    startIp;
    Ipv4    endIp;
};

bool atWifiMode(int32_t mode);
bool atWifiMode(int32_t * mode);
bool atWifiConnect(String const & ssid, String const & pwd, Mac bssid = mac());
bool atWifiConnect(WifiLinkedAp * ap);
bool atWifiScan(std::vector<WifiApItem> & list);
bool atWifiScanAsync(std::function<void ()> const & callback);
bool atWifiDisconnect();
bool atWifiApConfigure(WifiApConfigure const & configure);
bool atWifiApConfigure(WifiApConfigure * configure);
bool atWifiUser(std::vector<WifiUser> & list);
bool atDhcp(bool enable, int32_t mask);
bool atDhcp(int32_t * result);
bool atDhcpIpRangeClear();
bool atDhcpIpRange(IpRange const & configure);
bool atDhcpIpRange(IpRange * configure);
bool atApAutoConnect(bool enable);
bool atApStartSmart(int32_t type = leaveOut);
bool atApStopSmart();
bool atWps(bool enable);
bool atStationHostName(String const & name);
bool atStationHostName(String * name);
bool atMdnsDisable();
bool atMdns(String hostName, String serviceName, int32_t port);

