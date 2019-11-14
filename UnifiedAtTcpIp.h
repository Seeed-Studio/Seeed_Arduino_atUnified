#pragma once
#include"UnifiedAtType.h"

constexpr const char * TCP = "TCP";
constexpr const char * UDP = "UDP";
constexpr const char * SSL = "SSL";

class ConnectionBaseInfo{
public:
    ConnectionBaseInfo(){}
    ConnectionBaseInfo(
        String const & type,
        String const & ipOrDomain, 
        int32_t port = leaveOut) : 
            type(type),
            ipOrDomain(ipOrDomain),
            port(port){
        }
    
    String  type;
    String  ipOrDomain;
    int32_t port;
};

class TcpSslConnection : public ConnectionBaseInfo{
public:
    TcpSslConnection(){}
    TcpSslConnection(
        String const & type,
        String const & ipOrDomain, 
        int32_t port = leaveOut, 
        int32_t secondOfKeepAlive = leaveOut) : 
            ConnectionBaseInfo(type, ipOrDomain, port),
            secondOfKeepAlive(secondOfKeepAlive){
        }
    
    int32_t secondOfKeepAlive;
};

class UdpConnection : public ConnectionBaseInfo{
public:
    UdpConnection(){}
    UdpConnection(
        String const & ipOrDomain, 
        int32_t port = leaveOut, 
        int32_t localPort = leaveOut,
        int32_t mode = leaveOut) : 
            ConnectionBaseInfo(UDP, ipOrDomain, port),
            localPort(localPort),
            mode(mode){
        }
    int32_t localPort;
    int32_t mode;
};

//timezone
//- neg : east timezone, -8 indicated the 8th timezone of east
//- pos : west timezone

class TimeSource{
public:
    TimeSource() : 
        timezone(leaveOut){}
    int32_t timezone;
    String  domain[3];
};

class IpInfo{
public:
    ipv4 apIp;
    mac  apMac;
    ipv4 stationIp;
    mac  stationMac;
};

bool atIpStatus(int32_t * status);
bool atGetIpByDomainName(String const & domainName, ipv4 * ip);
bool atDns(bool userDefinedDns,  ipv4 const & dns1 = nullref,  ipv4 const & dns2 = nullref);
bool atDns(ipv4 * dns1, ipv4 * dns2 = nullptr);
bool atStationMac(mac const & address);
bool atStationMac(mac * address);
bool atApMac(mac const & address);
bool atApMac(mac * address);
bool atStationIp(ipv4 const & ip, ipv4 const & gateway = nullref, ipv4 const & netmask = nullref);
bool atStationIp(ipv4 * ip, ipv4 * gateway, ipv4 * netmask);
bool atApIp(ipv4 const & ip, ipv4 const & gateway = nullref, ipv4 const & netmask = nullref);
bool atApIp(ipv4 * ip, ipv4 * gateway, ipv4 * netmask);
bool atConnect(TcpSslConnection const & info, int32_t id = leaveOut);
bool atConnect(UdpConnection const & info, int32_t id = leaveOut);
bool atSslConfigure(int32_t type, int32_t certKeyId, int32_t CaId, int32_t id = leaveOut);
bool atIpSend(uint8_t const * buffer, int32_t length, int32_t id = leaveOut);
bool atIpSend(uint8_t const * buffer, int32_t length, ipv4 ip, int32_t port, int32_t id = leaveOut);
bool atIpClose(int32_t id = leaveOut);
bool atIpInfo(IpInfo * info);
bool atIpMux(bool enable);
bool atIpMux(bool * enable);
bool atIpServer(bool enable, int32_t port = leaveOut, bool sslCaEnable = true);
bool atIpServer(bool * enable, int32_t * port = nullptr, bool * sslCaEnable = nullptr);
bool atServerMaxConnection(int32_t count);
bool atServiceMaxConnection(int32_t * count);
bool atIpMode(int32_t mode);
bool atIpMode(int32_t * mode);
bool atSaveConnection(bool passthrough, TcpSslConnection const & info);
bool atSaveConnection(bool passthrough, UdpConnection const & info);
bool atIpTimeout(int32_t second);
bool atIpTimeout(int32_t * second);
bool atTimeSource(bool enable, TimeSource const & src = nullref);
bool atTimeSource(bool * enable, TimeSource * src = nullptr);
bool atDateTime(DateTime * result);
bool atPing(String const & ipOrDomain, int32_t * ms);

