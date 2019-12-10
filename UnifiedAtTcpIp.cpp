#include"UnifiedAtTcpIp.h"
#include"UnifiedRingBuffer.h"
#include"UnifiedAtEvent.h"
#include"Utilities.h"
#define MAX_PACKET_CHANNEL      5

bool atAvailableChannel(int32_t * value){
    int32_t  status;
    uint32_t freeChannel = (1 << MAX_PACKET_CHANNEL) - 1;
    Array<TransmissionLink> items;

    if (atIpStatus(& status, & items) == Fail || items.size() == 0){
        return Fail;
    }

    for (auto item = esp.wifi.tranLink; item < esp.wifi.tranLinkEnd; item++){
        freeChannel ^= 1 << item->id; // mark the channel in use.
    }

    value[0] = indexOfSetBit(freeChannel);
    return Success;
}

bool atIpStatus(int32_t * status, Array<TransmissionLink> * list){
    static TransmissionLink item;
    tx("AT+CIPSTATUS");
    rx("STATUS:", status);
    rx("+CIPSTATUS:", 
        & item.id, 
        & item.type, 
        & item.remoteIp, 
        & item.remotePort, 
        & item.localPort, 
        & item.asService
    );

    // reset buffer
    esp.wifi.tranLinkEnd = esp.wifi.tranLink;
    esp.analysis.whenResolutionOneLine = [&](){
        esp.wifi.tranLinkEnd[0] = item;
        esp.wifi.tranLinkEnd += 1;
    };
    
    flush();
    Result r = waitFlag();

    if (r == Success){
        list[0] = Array<TransmissionLink>(esp.wifi.tranLink, esp.wifi.tranLinkEnd - esp.wifi.tranLink);
    }
    return r;
}

CMD(atGetIpByDomainName, Text const & domainName, Ipv4 * ip)
    tx("AT+CIPDOMAIN=", domainName);
    rx("+CIPDOMAIN:", ip);
$

CMD(atDns, bool userDefinedDns, Ipv4 const & dns1, Ipv4 const & dns2)
    tx("AT+CIPDNS=", userDefinedDns, dns1, dns2);
$

CMD(atDns, bool userDefinedDns, Ipv4 * dns1, Ipv4 * dns2)
    tx("AT+CIPDNS=", userDefinedDns, dns1, dns2);
$

CMD(atDns, Ipv4 * dns1, Ipv4 * dns2)
    int32_t i = 0;
    Ipv4 *  dns[] = { dns1, dns2 };
    Ipv4 ** ptr = dns;
    Ipv4    item;

    tx("AT+CIPDNS?");
    rx("+CIPDNS:", & item);

    esp.analysis.whenResolutionOneLine = [&](){
        if (ptr[0]) {
            ptr[0][0] = item;
        }
    };

$

CMD(atStationMac, Mac const & address)
    tx("AT+CIPSTAMAC=", address);
$

CMD(atStationMac, Mac * address)
    tx("AT+CIPSTAMAC?");
    rx("+CIPSTAMAC:", address);
$

CMD(atApMac, Mac const & address)
    tx("AT+CIPAPMAC=", address);
$

CMD(atApMac, Mac * address)
    tx("AT+CIPAPMAC?");
    rx("+CIPAPMAC:", address);
$

CMD(atStationIp, 
    Ipv4 const & ip, 
    Ipv4 const & gateway, 
    Ipv4 const & netmask)
    tx("AT+CIPSTA=", ip, gateway, netmask);
$

CMD(atStationIp, 
    Ipv4 * ip, 
    Ipv4 * gateway, 
    Ipv4 * netmask)
    tx("AT+CIPSTA?");
    rx("+CIPSTA:", "ip", ip);
    rx("+CIPSTA:", "gateway", gateway);
    rx("+CIPSTA:", "netmask", netmask);
$

CMD(atApIp, 
    Ipv4 const & ip, 
    Ipv4 const & gateway, 
    Ipv4 const & netmask)
    tx("AT+CIPAP=", ip, gateway, netmask);
$

CMD(atApIp, 
    Ipv4 * ip, 
    Ipv4 * gateway, 
    Ipv4 * netmask)
    tx("AT+CIPAP?");
    rx("+CIPAP:", "ip", ip);
    rx("+CIPAP:", "gateway", gateway);
    rx("+CIPAP:", "netmask", netmask);
$

CMD(atIpConnect, int32_t id, TcpSslConnection const & info)
    // Format:
    // AT+CIPSTART="TCP","192.168.101.110",1000
    // when use multiple connections
    // then the first param is connection id
    // AT+CIPSTART=1,"TCP","192.168.101.110",1000
    // MUST IN MULTIPLE CONNECTIONS
    tx("AT+CIPSTART=", 
        id, 
        info.type,
        info.ipOrDomain, 
        info.port, 
        info.secondOfKeepAlive
    );
$

CMD(atIpConnect, int32_t id, UdpConnection const & info)
    // Format:
    // AT+CIPSTART="UDP","192.168.101.110",1000,1002,2
    // when use multiple connections
    // then the first param is connection id
    // AT+CIPSTART=1,"TCP","192.168.101.110",1000
    tx("AT+CIPSTART=",
        id,
        info.type,
        info.ipOrDomain, 
        info.port, 
        info.localPort,
        info.mode
    );
$

// type:
// - 0: Non-certification
// - 1: Load cert and private key, for server authentication
// - 2: Loading cert and private key of CA, Certified server
// - 3: Two-way authentication, SSL client and server to authenticate each otherundefineds certificate
CMD(atIpSslConfigure, int32_t id, int32_t type, int32_t certKeyId, int32_t CaId)
    tx("AT+CIPSSLCCONF=", id, type, certKeyId, CaId);
$

CMD(atIpSendCore, 
    int32_t id, 
    uint8_t const * buffer, 
    int32_t length, 
    Ipv4 const & ip = nullref, 
    int32_t const & port = nullref)
    tx("AT+CIPSEND=", id, ip, port);
    rx([=](Text & resp, Any *){
        if (resp != ">"){
            return;
        }
        txBin(buffer, length);
    });
$

CMD(atIpSend, int32_t id, uint8_t const * buffer, int32_t length)
    return atIpSendCore(id, buffer, length);
$

CMD(atIpSend, int32_t id, uint8_t const * buffer, int32_t length, Ipv4 ip, int32_t port)
    return atIpSendCore(id, buffer, length, ip, port);
$

bool atIpAvailable(int32_t id, int32_t * size){
    auto & packet = esp.wifi.packet[id];
    size[0] = packet.empty() ? 0 : packet.front().length;
    return Success;
}

bool atIpPeek(int32_t id, int32_t * buffer){
    auto & packet = esp.wifi.packet[id];
    if (packet.empty()){
        buffer[0] = -1;
        return Success;
    }
    
    auto & pack = packet.front();
    buffer[0] = pack.data.get()[pack.i];
    return Success;
}

bool atIpReceive(int32_t id, uint8_t * buffer, int32_t length, int32_t * actuallyLength){
    size_t i = 0;
    auto & packet = esp.wifi.packet[id];

    while (packet.empty() == false){
        for(auto & pack = packet.front(); pack.i < pack.length; pack.i++, i++){
            if (i == length){
                goto end;
            }
            buffer[i] = pack.data.get()[pack.i];
        }
        packet.pop();
    }
end:
    if (actuallyLength){
        actuallyLength[0] = i;
    }
    return Success;
}

// id is not needed when at single connection mode
// when at multiconnection mode
// id range 0~4
// - 5: Close all transmission
CMD(atIpClose, int32_t id)
    tx("AT+CIPCLOSE=", id);
$

CMD(atIpInfo, IpInfo * info)
    tx("AT+CIFSR");
    rx("+CIFSR:", "APIP",   & info->apIp);
    rx("+CIFSR:", "APMAC",  & info->apMac);
    rx("+CIFSR:", "STAIP",  & info->stationIp);
    rx("+CIFSR:", "STAMAC", & info->stationMac);
$

CMD(atIpMux, bool enable)
    tx("AT+CIPMUX=", enable);
$

CMD(atIpMux, bool * enable)
    tx("AT+CIPMUX?");
    rx("+CIPMUX:", enable);
$

CMD(atIpServer, bool enable, int32_t const & port)
    tx("AT+CIPSERVER=", enable, port);
$

// NOTICE
// ONLY ONE SERVICE CAN BE CREATED
// type
// - "SSL" : to set SSL server
CMD(atIpServer, bool enable, int32_t port, Text const & type, bool enableCA)
    tx("AT+CIPSERVER=", enable, port, type, enableCA);
$

CMD(atIpServerMaxConnection, int32_t count)
    tx("AT+CIPSERVERMAXCONN=", count);
$

CMD(atIpServiceMaxConnection, int32_t * count)
    tx("AT+CIPSERVERMAXCONN?");
    rx("+CIPSERVERMAXCONN:", count);
$

// mode
// - 0 : Normal mode.
// - 1 : UART-Wi-Fi passthrough mode (transparent transmission), 
//      which can only be enabled in TCP/SSL single connection mode or 
//      in UDP mode when the remote IP and port do not change.
CMD(atIpMode, int32_t mode)
    tx("AT+CIPMODE=", mode);
$

CMD(atIpMode, int32_t * mode)
    tx("AT+CIPMODE?");
    rx("+CIPMODE=", mode);
$

// passthrough
// - 0 : Normal mode, ESP32 will NOT enter UART-Wi-Fi passthrough mode on power-up.
// - 1 : ESP32 will enter UART-Wi-Fi passthrough mode on power-up.
CMD(atIpSaveConnection, bool passthrough, TcpSslConnection const & info)
    tx("AT+SAVETRANSLINK=",
        passthrough,
        info.ipOrDomain, 
        info.port, 
        info.type,
        info.secondOfKeepAlive
    );
$

CMD(atIpSaveConnection, bool passthrough, UdpConnection const & info)
    tx("AT+SAVETRANSLINK=", 
        passthrough,
        info.ipOrDomain, 
        info.port, 
        info.type,
        info.localPort
    );
$

// second
//   0 : TCP server will never Timeout.
// > 0 : TCP server will disconnect from the TCP client that does not communicate with it until Timeout.
CMD(atIpTimeout, int32_t second)
    tx("AT+CIPSTO=", second);
$

CMD(atIpTimeout, int32_t * second)
    tx("AT+CIPSTO?");
    rx("+CIPSTO:", second);
$

CMD(atIpTimeSource, bool enable, TimeSource src)
    tx("AT+CIPSNTPCFG=", 
        enable,
        src.timezone,
        src.domain[0],
        src.domain[1],
        src.domain[2]);
$

CMD(atIpTimeSource, bool * enable, TimeSource * src)
    tx("AT+CIPSNTPCFG?");
    rx("+CIPSNTPCFG:",
        & enable[0],
        & src->timezone, 
        & src->domain[0],
        & src->domain[1],
        & src->domain[2]
    );
$

CMD(atIpDateTime, DateTime * result)
    // Format:
    // +CIPSNTPTIME:Mon Dec 12 02:33:32 2016
    tx("AT+CIPSNTPTIME?");
    rx("+CIPSNTPTIME:", result);
$

// // AT+CIUPDATE
// CMD(atUpdateByWifi, int32_t mode)

CMD(atIpPing, Text const & ipOrDomain, int32_t * ms)
    Text item;
    tx("AT+PING=", ipOrDomain);
    rx("+PING:", & item);

    esp.analysis.whenResolutionOneLine = [&](){
        if (item.startsWith("TIMEOUT")){
            ms[0] = -1;
        }
        else{
            Any any[] = { ms };
            rxMain(item, any);
        }
    };
$

