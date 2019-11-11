#pragma once
#include"UnifiedAtType.h"

#define SET_CIPSTATUS       "AT+CIPSTATUS"
#define ASK_CIPDOMAIN       "AT+CIPDOMAIN=%s", domainName
#define GET_CIPDOMAIN       "+CIPDOMAIN:%i", ip
#define SET_CIPDNS          "AT+CIPDNS=%d%+i%+i", userDefinedDns, dns1, dns2
#define ASK_CIPDNS          "AT+CIPDNS?"
#define GET_CIPDNS          "+CIPDNS:%i\n+CIPDNS:%i", dns1, dns2

#define SET_CIPSTAMAC       "AT+CIPSTAMAC=%m", address
#define ASK_CIPSTAMAC       "AT+CIPSTAMAC?"
#define GET_CIPSTAMAC       "+CIPSTAMAC:%m", address

#define SET_CIPAPMAC        "AT+CIPAPMAC=%m", address
#define ASK_CIPAPMAC        "AT+CIPAPMAC?"
#define GET_CIPAPMAC        "+CIPAPMAC:%m", address

#define SET_CIPSTA          "AT+CIPSTA=%i%+i%+i", ip, gateway, netmask
#define ASK_CIPSTA          "AT+CIPSTA?"
#define GET_CIPSTA          "+CIPSTA:ip:%i\n"           \
                            "+CIPSTA:gateway:%i\n"      \
                            "+CIPSTA:netmask:%i\n",     \
                                ip, gateway, netmask

#define SET_CIPAP           "AT+CIPAP=%i%+i%+i", ip, gateway, netmask
#define ASK_CIPAP           "AT+CIPAP?"
#define GET_CIPAP           "+CIPAP:ip:%i\n"            \
                            "+CIPAP:gateway:%i\n"       \
                            "+CIPAP:netmask:%i",        \
                                ip, gateway, netmask

#define SET_CIPSTART_TCP    "AT+CIPSTART=\"TCP\",%s,%i,%d%+d", 

enum Transmissiontype{
    TCP, UDP, SSL,
};

class IpLink{
public:
    Transmissiontype
            transmissiontype;
    ipv4    remoteIp;
    int32_t remotePort;
    union{
        struct{
            int32_t secondOfKeepAlive;
        }tcp;

        struct{
            int32_t localPort;

            //meanings:
            //0 - After receiving the data, do not change the remote label [default]
            //1 - After receiving the data, change the remote label
            //2 - After receiving the data, change the remote beacon
            int32_t mode;
        }udp;

        struct{
            int32_t secondOfKeepAlive;
        }ssl;
    };
}

CMD(atTcpIpStatus)

$

CMD(atGetIpByDomainName, String const & domainName, ipv4 * ip)
    tx(ASK_CIPDOMAIN);
    rx(GET_CIPDOMAIN);
$

CMD(atDns, 
    bool userDefinedDns, 
    ipv4 const & dns1 = nullref, 
    ipv4 const & dns2 = nullref)
    tx(SET_CIPDNS);
$

CMD(atDns, bool userDefinedDns, ipv4 * dns1, ipv4 * dns2)
    tx(SET_CIPDNS);
$

CMD(atDns, ipv4 const & dns1, ipv4 const & dns2 = nullref)
    tx(ASK_CIPDNS);
    rx(GET_CIPDNS);
$

CMD(atStationMac, mac const & address)
    tx(SET_CIPSTAMAC);
$

CMD(atStationMac, mac * address)
    tx(ASK_CIPSTAMAC);
    rx(GET_CIPSTAMAC);
$

CMD(atApMac, mac const & address)
    tx(SET_CIPAPMAC);
$

CMD(atApMac, mac * address)
    tx(ASK_CIPAPMAC);
    rx(GET_CIPAPMAC);
$

CMD(atStationIp, 
    ipv4 const & ip, 
    ipv4 const & gateway = nullref, 
    ipv4 const & netmask = nullref)
    tx(SET_CIPSTA);
$

CMD(atStationIp, 
    ipv4 * ip, 
    ipv4 * gateway, 
    ipv4 * netmask)
    tx(ASK_CIPSTA);
    tx(GET_CIPSTA);
$

CMD(atApIp, 
    ipv4 const & ip, 
    ipv4 const & gateway = nullref, 
    ipv4 const & netmask = nullref)
    tx(SET_CIPAP);
$

CMD(atApIp, 
    ipv4 * ip, 
    ipv4 * gateway, 
    ipv4 * netmask)
    tx(ASK_CIPAP);
    tx(GET_CIPAP);
$

CMD(atTcpConnect, String const & address, int32_t port, int32_t secondOfKeepAlive = leaveOut)
    tx("AT+CIPSTART=\"TCP\",%s,%d%+d", address, port, secondOfKeepAlive);
$

CMD(atUdpConnect, String const & address, int32_t port, int32_t localPort = leaveOut, int32_t mode = leaveOut)
    tx("AT+CIPSTART=\"UDP\",%s,%d%+d%+d", address, port, localPort, mode);
$

CMD(atSslConnect, String const & address, int32_t port, int32_t secondOfKeepAlive = leaveOut)
    tx("AT+CIPSTART=\"SSL\",%s,%d%+d", address, port, secondOfKeepAlive);
$


