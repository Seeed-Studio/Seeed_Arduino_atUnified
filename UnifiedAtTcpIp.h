#pragma once
#include"UnifiedAtType.h"
#include"UnifiedRingBuffer.h"

constexpr const char * TypeTCP = "TCP";
constexpr const char * TypeUDP = "UDP";
constexpr const char * TypeSSL = "SSL";

struct TransmissionLink{
    // 0 ~ 4
    Ni32    id;

    // - UDP
    // - TCP
    Text    type;

    Ipv4    remoteIp;
    int32_t remotePort;
    int32_t localPort;

    // - 0 : as client
    // - 1 : as service
    int32_t asService;
};

struct ConnectionBaseInfo{
    Text    type;
    Text    ipOrDomain;
    Ni32    port;
};

struct TcpSslConnection : public ConnectionBaseInfo{
    Ni32    secondOfKeepAlive;

    operator TcpSslConnection &(){
        return *(TcpSslConnection *)this;
    }
};

struct TcpConnection : TcpSslConnection{
    TcpConnection(){
        type = TypeTCP;
    }
};

struct SslConnection : TcpSslConnection{
    SslConnection(){
        type = TypeSSL;
    }
};

struct UdpConnection : public ConnectionBaseInfo{
    UdpConnection() {
        type = TypeUDP;
    }
    Ni32    localPort;

    // - 0 : the destination peer entity of UDP will not change; this is the default setting.
    // - 1 : the destination peer entity of UDP will change once.
    // - 2 : the destination peer entity of UDP is allowed to change.
    Ni32    mode;
};

struct TimeSource{
    //- pos : west timezone
    //- neg : east timezone, -8 indicated the 8th timezone of east
    Ni32    timezone;
    Text    domain[3];
};

struct IpInfo{
    Ipv4 apIp;
    Ipv4 stationIp;
    Mac  apMac;
    Mac  stationMac;
};

bool atIpAvailableChannel(int32_t * value);
bool atIpStatus(int32_t * status, Array<TransmissionLink> * list = nullptr);
bool atGetIpByDomainName(Text const & domainName, Ipv4 * ip);
bool atDns(bool userDefinedDns,  Ipv4 const & dns1 = nullref, Ipv4 const & dns2 = nullref);
bool atDns(Ipv4 * dns1, Ipv4 * dns2 = nullptr);
bool atStationMac(Mac const & address);
bool atStationMac(Mac * address);
bool atApMac(Mac const & address);
bool atApMac(Mac * address);
bool atStationIp(Ipv4 const & ip, Ipv4 const & gateway = nullref, Ipv4 const & netmask = nullref);
bool atStationIp(Ipv4 * ip, Ipv4 * gateway = nullptr, Ipv4 * netmask = nullptr);
bool atApIp(Ipv4 const & ip, Ipv4 const & gateway = nullref, Ipv4 const & netmask = nullref);
bool atApIp(Ipv4 * ip, Ipv4 * gateway = nullptr, Ipv4 * netmask = nullptr);
bool atIpConnect(int32_t id, TcpSslConnection const & info);
bool atIpConnect(int32_t id, UdpConnection const & info);
bool atIpSslConfigure(int32_t id, int32_t type, int32_t certKeyId, int32_t CaId);
bool atIpSend(int32_t id, uint8_t const * buffer, int32_t length);
bool atIpSend(int32_t id, uint8_t const * buffer, int32_t length, Ipv4 ip, int32_t port);
bool atIpAvailable(int32_t id, int32_t * size);
bool atIpPeek(int32_t id, int32_t * buffer);
bool atIpReceive(int32_t id, uint8_t * buffer, int32_t length, int32_t * actuallyLength = nullptr);
bool atIpClose(int32_t id);
bool atIpInfo(IpInfo * info);
bool atIpMux(bool enable);
bool atIpMux(bool * enable);
bool atIpServer(bool enable, int32_t const & port = nullref);
bool atIpServer(bool enable, int32_t port, Text const & type, bool enableCA);
bool atIpServerMaxConnection(int32_t count);
bool atIpServiceMaxConnection(int32_t * count);
bool atIpMode(int32_t mode);
bool atIpMode(int32_t * mode);
bool atIpSaveConnection(bool passthrough, TcpSslConnection const & info);
bool atIpSaveConnection(bool passthrough, UdpConnection const & info);
bool atIpTimeout(int32_t second);
bool atIpTimeout(int32_t * second);
bool atIpTimeSource(bool enable, TimeSource src = TimeSource());
bool atIpTimeSource(bool * enable, TimeSource * src = nullptr);
bool atIpDateTime(DateTime * result);
bool atIpPing(Text const & ipOrDomain, int32_t * ms);

