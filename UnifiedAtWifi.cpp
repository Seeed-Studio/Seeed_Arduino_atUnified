#include"UnifiedAtWifi.h"

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

CMD(atWifiConnect, 
    String ssid, 
    String pwd, 
    String bssid)
    if (bssid == ""){
        tx(AT CWJAP SET, ssid, pwd);
    }
    else{
        tx(AT CWJAP SET, ssid, pwd, bssid);
    }
$

CMD(atWifiScan, std::function<void (WifiLinkInfo &)> && call)
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

CMD(atWifiUserList, std::function<void (WifiUserList &)> && call)
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
