#pragma once
#include"UnifiedAtType.h"
#ifdef max
    #pragma push(max)
    #pragma push(min)
    #undef max
    #undef min
    #include<functional>
    #pragma pop(min)
    #pragma pop(max)
#else
    #include<functional>
#endif
extern String readLine();

class WifiLinkInfo{
public:
    int8_t  ecn;
    String  ssid;
    int8_t  rssi;
    uint8_t mac[6];
    uint8_t channel;
};

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

class WifiUserList{
public:
    uint8_t ip[4];
    uint8_t mac[6];
};

class IpRange{
public:
    int32_t leaseMinute;
    uint8_t startIp[4];
    uint8_t endIp[4];
};

bool atWifiMode(int32_t mode);
bool atWifiMode(int32_t * mode);
bool atWifiConnect(String ssid, String pwd, String bssid = "");
bool atWifiScan(std::function<void (WifiLinkInfo &)> && call);
bool atWifiDisconnect();
bool atWifiApConfigure(WifiApConfigure * configure);
bool atWifiApConfigure(WifiApConfigure const & configure);
bool atWifiUserList(std::function<void (WifiUserList &)> && call);
bool atDhcp(bool enable, int32_t mask);
bool atDhcp(int32_t * result);
bool atDhcpIpRangeClear();
bool atDhcpIpRange(IpRange const & configure);
bool atDhcpIpRange(IpRange * configure);
bool atApAutoConnect(bool enable);
bool atApStartSmart(int32_t type);
bool atApStopSmart(int32_t type);
bool atWps(bool enable);
bool atHostNameTemp(String const & name);
bool atHostNameTemp(String * name);
bool atMdnsDisable();
bool atMdns(String hostName, String serviceName, int32_t port);
