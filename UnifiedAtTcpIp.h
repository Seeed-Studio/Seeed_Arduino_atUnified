#pragma once
#include"UnifiedAtType.h"
#include"UnifiedRingBuffer.h"

constexpr const char * TCP = "TCP";
constexpr const char * UDP = "UDP";
constexpr const char * SSL = "SSL";

struct ConnectedItem{
    // 0 ~ 4
    int32_t id;

    // - UDP
    // - TCP
    String  type;

    Ipv4    remoteIp;
    int32_t remotePort;
    int32_t localPort;

    // - 0 : as client
    // - 1 : as service
    int32_t asService;
};

struct ConnectionBaseInfo{
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

struct TcpSslConnection : public ConnectionBaseInfo{
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

struct UdpConnection : public ConnectionBaseInfo{
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

    // - 0 : the destination peer entity of UDP will not change; this is the default setting.
    // - 1 : the destination peer entity of UDP will change once.
    // - 2 : the destination peer entity of UDP is allowed to change.
    int32_t mode;
};

//timezone
//- neg : east timezone, -8 indicated the 8th timezone of east
//- pos : west timezone

struct TimeSource{
    TimeSource() : 
        timezone(leaveOut){}
    int32_t timezone;
    String  domain[3];
};

struct IpInfo{
    Ipv4 apIp;
    Mac  apMac;
    Ipv4 stationIp;
    Mac  stationMac;
};

bool atIpAvailableChannel(int32_t * value);
bool atIpStatus(int32_t * status, std::vector<ConnectedItem> & list = nullref);
bool atGetIpByDomainName(String const & domainName, Ipv4 * ip);
bool atDns(bool userDefinedDns,  Ipv4 dns1 = Ipv4(), Ipv4 dns2 = Ipv4());
bool atDns(Ipv4 * dns1, Ipv4 * dns2 = nullptr);
bool atStationMac(Mac const & address);
bool atStationMac(Mac * address);
bool atApMac(Mac const & address);
bool atApMac(Mac * address);
bool atStationIp(Ipv4 const & ip, Ipv4 gateway = Ipv4(), Ipv4 netmask = Ipv4());
bool atStationIp(Ipv4 * ip, Ipv4 * gateway, Ipv4 * netmask);
bool atApIp(Ipv4 const & ip, Ipv4 gateway = Ipv4(), Ipv4 netmask = Ipv4());
bool atApIp(Ipv4 * ip, Ipv4 * gateway = nullptr, Ipv4 * netmask = nullptr);
bool atIpConnect(TcpSslConnection const & info, int32_t id = leaveOut);
bool atIpConnect(UdpConnection const & info, int32_t rxBufferSize, int32_t id = leaveOut);
bool atIpSslConfigure(int32_t type, int32_t certKeyId, int32_t CaId, int32_t id = leaveOut);
bool atIpSend(uint8_t const * buffer, int32_t length, int32_t id = leaveOut);
bool atIpSend(uint8_t const * buffer, int32_t length, Ipv4 ip, int32_t port, int32_t id = leaveOut);
bool atIpAvailable(int32_t * size, int32_t id = leaveOut);
bool atIpPeek(int32_t * buffer, int32_t id = leaveOut);
bool atIpReceive(uint8_t * buffer, int32_t length, int32_t * actuallyLength = nullptr, int32_t id = leaveOut);
bool atIpClose(int32_t id = leaveOut);
bool atIpInfo(IpInfo * info);
bool atIpMux(bool enable);
bool atIpMux(bool * enable);
bool atIpServer(bool enable, int32_t port = leaveOut, bool sslCaEnable = true);
bool atIpServer(bool * enable, int32_t * port = nullptr, bool * sslCaEnable = nullptr);
bool atIpServerMaxConnection(int32_t count);
bool atIpServiceMaxConnection(int32_t * count);
bool atIpMode(int32_t mode);
bool atIpMode(int32_t * mode);
bool atIpSaveConnection(bool passthrough, TcpSslConnection const & info);
bool atIpSaveConnection(bool passthrough, UdpConnection const & info);
bool atIpTimeout(int32_t second);
bool atIpTimeout(int32_t * second);
bool atIpTimeSource(bool enable, TimeSource const & src = nullref);
bool atIpTimeSource(bool * enable, TimeSource * src = nullptr);
bool atIpDateTime(DateTime * result);
bool atIpPing(String const & ipOrDomain, int32_t * ms);

