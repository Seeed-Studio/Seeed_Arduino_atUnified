#include"UnifiedAtTcpIp.h"
#include"UnifiedRingBuffer.h"
#include"Utilities.h"
#define MAX_PACKET_CHANNEL      5

UnifiedRingBuffer<uint8_t> packetBuffers[MAX_PACKET_CHANNEL];

bool atAvailableChannel(int32_t * value){
    int32_t  status;
    uint32_t freeChannel = (1 << MAX_PACKET_CHANNEL) - 1;
    std::vector<ConnectedItem> items;
    if (atIpStatus(& status, items) == fail || items.size() == 0){
        return fail;
    }

    for (auto & item : items){
        freeChannel ^= 1 << item.id; // mark the channel in use.
    }

    auto i = indexOfSetBit(freeChannel);

    if (i == -1){
        return fail;
    }

    value[0] = i;
    return success;
}

CMD(atIpStatus, int32_t * status, std::vector<ConnectedItem> & list)
    tx("AT+CIPSTATUS");
    rx("STATUS:%d", status);

    std::vector<ConnectedItem> tmp;
    ConnectedItem item;

    while(
        rx("+CIPSTATUS:%d,%s,%i,%d,%d,%d", 
            item.id,
            item.type,
            item.remoteIp,
            item.remotePort,
            item.localPort,
            item.asService
        ) != fail){
        tmp.push_back(item);
    }
    if (& list != nullptr){
        list = tmp;
    }
$

CMD(atGetIpByDomainName, String const & domainName, Ipv4 * ip)
    tx("AT+CIPDOMAIN=%s", domainName);
    rx("+CIPDOMAIN:%i", ip);
$

CMD(atDns, bool userDefinedDns, Ipv4 dns1, Ipv4 dns2)
    tx("AT+CIPDNS=%d%+i%+i", userDefinedDns, dns1, dns2);
$

CMD(atDns, bool userDefinedDns, Ipv4 * dns1, Ipv4 * dns2)
    tx("AT+CIPDNS=%d%+i%+i", userDefinedDns, dns1, dns2);
$

CMD(atDns, Ipv4 * dns1, Ipv4 * dns2)
    tx("AT+CIPDNS?");
    rx("+CIPDNS:%i\n+CIPDNS:%i", dns1, dns2);
$

CMD(atStationMac, Mac const & address)
    tx("AT+CIPSTAMAC=%m", address);
$

CMD(atStationMac, Mac * address)
    tx("AT+CIPSTAMAC?");
    rx("+CIPSTAMAC:%m", address);
$

CMD(atApMac, Mac const & address)
    tx("AT+CIPAPMAC=%m", address);
$

CMD(atApMac, Mac * address)
    tx("AT+CIPAPMAC?");
    rx("+CIPAPMAC:%m", address);
$

CMD(atStationIp, 
    Ipv4 const & ip, 
    Ipv4 const & gateway, 
    Ipv4 const & netmask)
    tx("AT+CIPSTA=%i%+i%+i", ip, gateway, netmask);
$

CMD(atStationIp, 
    Ipv4 * ip, 
    Ipv4 * gateway, 
    Ipv4 * netmask)
    tx("AT+CIPSTA?");
    rx(
        "+CIPSTA:ip:%i\n"
        "+CIPSTA:gateway:%i\n"
        "+CIPSTA:netmask:%i\n",
        ip, gateway, netmask);
$

CMD(atApIp, 
    Ipv4 const & ip, 
    Ipv4 const & gateway, 
    Ipv4 const & netmask)
    tx("AT+CIPAP=%i%+i%+i", ip, gateway, netmask);
$

CMD(atApIp, 
    Ipv4 * ip, 
    Ipv4 * gateway, 
    Ipv4 * netmask)
    tx("AT+CIPAP?");
    rx(
        "+CIPAP:ip:%i\n"
        "+CIPAP:gateway:%i\n"
        "+CIPAP:netmask:%i",
        ip, gateway, netmask);
$

CMD(atIpConnect, TcpSslConnection const & info, int32_t id)
    // Format:
    // AT+CIPSTART="TCP","192.168.101.110",1000
    // when use multiple connections
    // then the first param is connection id
    // AT+CIPSTART=1,"TCP","192.168.101.110",1000
    ta("AT+CIPSTART=");
    id != leaveOut && ta("%d,", id);
    tx("%s,%s,%d%+d",
        info.type,
        info.ipOrDomain, 
        info.port, 
        info.secondOfKeepAlive
    );
$

CMD(atIpConnectCore, UdpConnection const & info, int32_t id)
    // Format:
    // AT+CIPSTART="UDP","192.168.101.110",1000,1002,2
    // when use multiple connections
    // then the first param is connection id
    // AT+CIPSTART=1,"TCP","192.168.101.110",1000
    ta("AT+CIPSTART=");
    id != leaveOut && ta("%d,", id); // skip when id is leaveOut
    tx("%s,%s,%d%+d%+d", 
        info.type,
        info.ipOrDomain, 
        info.port, 
        info.localPort,
        info.mode);
$

bool atIpConnect(UdpConnection const & info, int32_t rxBufferSize, int32_t id){
    if (atIpConnectCore(info, id) == fail){
        return fail;
    }
    if (id == leaveOut) id = 0;
    packetBuffers[id].setup(rxBufferSize);
    return packetBuffers[id];
}

namespace{
    bool send(uint8_t const * buffer, size_t length){
        if (waitFlag(">") != fail){
            return tb(buffer, length);
        }
        else{
            return fail;
        }
    }
}

// type:
// - 0: Non-certification
// - 1: Load cert and private key, for server authentication
// - 2: Loading cert and private key of CA, Certified server
// - 3: Two-way authentication, SSL client and server to authenticate each otherundefineds certificate
CMD(atIpSslConfigure, int32_t type, int32_t certKeyId, int32_t CaId, int32_t id)
    ta("AT+CIPSSLCCONF=");
    id != leaveOut && ta("%d,", id);
    tx("%d,%d,%d", type, certKeyId, CaId);
$

CMD(atIpSend, uint8_t const * buffer, int32_t length, int32_t id)
    ta("AT+CIPSEND=");
    id != leaveOut && 
    ta("%d,");
    tx("%d", length);
    send(buffer, length);
$

CMD(atIpSend, uint8_t const * buffer, int32_t length, Ipv4 ip, int32_t port, int32_t id)
    ta("AT+CIPSEND=");
    id != leaveOut && 
    ta("%d,", id);
    tx("%d,%i,%d", length, ip, port);
    send(buffer, length);
$

CMD(atIpAvailable, int32_t * size, int32_t id)
    if (id == leaveOut) id = 0;
    size[0] = packetBuffers[id].available();
    return success;
$

bool atIpPeek(int32_t * buffer, int32_t id){
    if (id == leaveOut) id = 0;
    buffer[0] = packetBuffers[id].peek();
    return success;
}
bool atIpReceive(uint8_t * buffer, int32_t length, int32_t * actuallyLength, int32_t id){
    if (id == leaveOut) id = 0;
    size_t i = 0;
    auto & packet = packetBuffers[id];

    for (; i < length && packet.available(); i++) {
        buffer[i] = packet.read_char();
    }
    if (actuallyLength){
        actuallyLength[0] = i;
    }
    return success;
}

// id is not needed when at single connection mode
// when at multiconnection mode
// id range 0~4
// - 5: Close all transmission
CMD(atIpClose, int32_t id)
    ta("AT+CIPCLOSE");
    id != leaveOut &&
    ta("=%d");
    tx(""); //END LINE

    if (id == leaveOut) id = 0;
    packetBuffers[id].clear();
$

CMD(atIpInfo, IpInfo * info)
    tx("AT+CIFSR");
    rx(
        "+CIFSR:APIP,%i\n"
        "+CIFSR:APMAC,%m\n"
        "+CIFSR:STAIP,%i\n"
        "+CIFSR:STAMAC,%m", 
        info->apIp,
        info->apMac,
        info->stationIp,
        info->stationMac
    );
$

CMD(atIpMux, bool enable)
    tx("AT+CIPMUX=%b", enable);
$

CMD(atIpMux, bool * enable)
    tx("AT+CIPMUX?");
    rx("+CIPMUX:%b", enable);
$

CMD(atIpServer, bool enable, int32_t port, bool sslCaEnable)
    tx("AT+CIPSERVER=%b%+d%+b", enable, port, sslCaEnable);
$

CMD(atIpServer, bool * enable, int32_t * port, bool * sslCaEnable)
    // Response format
    // +CIPSERVER:0
    // +CIPSERVER:1,443,"SSL",1
    tx("AT+CIPSERVER?");
    rx("+CIPSERVER:%b", enable) != fail && enable[0] && port && 
    rx(",%d", port) && sslCaEnable && 
    rx(",\"SSL\",%b", sslCaEnable);
$

CMD(atIpServerMaxConnection, int32_t count)
    tx("AT+CIPSERVERMAXCONN=%d", count);
$

CMD(atIpServiceMaxConnection, int32_t * count)
    tx("AT+CIPSERVERMAXCONN?");
    rx("+CIPSERVERMAXCONN:%d", count);
$

//mode
//- 0 : Normal mode.
//- 1 : UART-Wi-Fi passthrough mode (transparent transmission), 
//      which can only be enabled in TCP/SSL single connection mode or 
//      in UDP mode when the remote IP and port do not change.
CMD(atIpMode, int32_t mode)
    tx("AT+CIPMODE=%d", mode);
$

CMD(atIpMode, int32_t * mode)
    tx("AT+CIPMODE?");
    rx("+CIPMODE=%d", mode);
$

//passthrough
//- 0 : Normal mode, ESP32 will NOT enter UART-Wi-Fi passthrough mode on power-up.
//- 1 : ESP32 will enter UART-Wi-Fi passthrough mode on power-up.
CMD(atIpSaveConnection, bool passthrough, TcpSslConnection const & info)
    tx("AT+SAVETRANSLINK=%b,%s,%d%+s%+d", 
        passthrough,
        info.ipOrDomain, 
        info.port, 
        info.type,
        info.secondOfKeepAlive);
$

CMD(atIpSaveConnection, bool passthrough, UdpConnection const & info)
    tx("AT+SAVETRANSLINK=%n,%s,%d%+s%+d", 
        passthrough,
        info.ipOrDomain, 
        info.port, 
        info.type,
        info.localPort);
$

// second
//   0 : TCP server will never timeout.
// > 0 : TCP server will disconnect from the TCP client that does not communicate with it until timeout.
CMD(atIpTimeout, int32_t second)
    tx("AT+CIPSTO=%d", second);
$

CMD(atIpTimeout, int32_t * second)
    tx("AT+CIPSTO?");
    rx("+CIPSTO:%d", second);
$

CMD(atIpTimeSource, bool enable, TimeSource const & src)
    tx("AT+CIPSNTPCFG=%d%+d%+s%+s%+s", 
        enable,
        src.timezone,
        src.domain[0],
        src.domain[1],
        src.domain[2]);
$

CMD(atIpTimeSource, bool * enable, TimeSource * src)
    tx("AT+CIPSNTPCFG?");
    rx("+CIPSNTPCFG:%b", enable) && enable[0] && src &&
    rx(",%d,%s,%s,%s", 
        src->timezone, 
        src->domain[0],
        src->domain[1],
        src->domain[2]);
$

CMD(atIpDateTime, DateTime * result)
    // Format:
    // +CIPSNTPTIME:Mon Dec 12 02:33:32 2016
    tx("AT+CIPSNTPTIME?");
    rx("+CIPSNTPTIME:%t", result);
$

// // AT+CIUPDATE
// CMD(atUpdateByWifi, int32_t mode)

CMD(atIpPing, String const & ipOrDomain, int32_t * ms)
    tx("AT+PING=%s", ipOrDomain);
    rx("+PING:%d", ms);
$

