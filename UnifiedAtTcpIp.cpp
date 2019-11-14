#include"UnifiedAtTcpIp.h"

CMD(atIpStatus, int32_t * status)
    tx("AT+CIPSTATUS");
    rx("STATUS:%d", status);
    // more-----------------------------------------------------
    // rx("+CIPSTATUS:<link	ID>,<type>,<remote	IP>,<remote	port>,<localport>,<tetype>");
$

CMD(atGetIpByDomainName, String const & domainName, ipv4 * ip)
    tx("AT+CIPDOMAIN=%s", domainName);
    rx("+CIPDOMAIN:%i", ip);
$

CMD(atDns, bool userDefinedDns, ipv4 const & dns1, ipv4 const & dns2)
    tx("AT+CIPDNS=%d%+i%+i", userDefinedDns, dns1, dns2);
$

CMD(atDns, bool userDefinedDns, ipv4 * dns1, ipv4 * dns2)
    tx("AT+CIPDNS=%d%+i%+i", userDefinedDns, dns1, dns2);
$

CMD(atDns, ipv4 * dns1, ipv4 * dns2)
    tx("AT+CIPDNS?");
    rx("+CIPDNS:%i\n+CIPDNS:%i", dns1, dns2);
$

CMD(atStationMac, mac const & address)
    tx("AT+CIPSTAMAC=%m", address);
$

CMD(atStationMac, mac * address)
    tx("AT+CIPSTAMAC?");
    rx("+CIPSTAMAC:%m", address);
$

CMD(atApMac, mac const & address)
    tx("AT+CIPAPMAC=%m", address);
$

CMD(atApMac, mac * address)
    tx("AT+CIPAPMAC?");
    rx("+CIPAPMAC:%m", address);
$

CMD(atStationIp, 
    ipv4 const & ip, 
    ipv4 const & gateway, 
    ipv4 const & netmask)
    tx("AT+CIPSTA=%i%+i%+i", ip, gateway, netmask);
$

CMD(atStationIp, 
    ipv4 * ip, 
    ipv4 * gateway, 
    ipv4 * netmask)
    tx("AT+CIPSTA?");
    rx(
        "+CIPSTA:ip:%i\n"
        "+CIPSTA:gateway:%i\n"
        "+CIPSTA:netmask:%i\n",
        ip, gateway, netmask);
$

CMD(atApIp, 
    ipv4 const & ip, 
    ipv4 const & gateway, 
    ipv4 const & netmask)
    tx("AT+CIPAP=%i%+i%+i", ip, gateway, netmask);
$

CMD(atApIp, 
    ipv4 * ip, 
    ipv4 * gateway, 
    ipv4 * netmask)
    tx("AT+CIPAP?");
    rx(
        "+CIPAP:ip:%i\n"
        "+CIPAP:gateway:%i\n"
        "+CIPAP:netmask:%i",
        ip, gateway, netmask);
$

CMD(atConnect, TcpSslConnection const & info, int32_t id)
    // Format:
    // AT+CIPSTART="TCP","192.168.101.110",1000
    // when use multiple connections
    // then the first param is connection id
    // AT+CIPSTART=1,"TCP","192.168.101.110",1000
    tx("AT+CIPSTART=");
    id != leaveOut && tx("%d,", id); // skip when id is leaveOut
    tx("%s,%s,%d%+d", 
        info.type,
        info.ipOrDomain, 
        info.port, 
        info.secondOfKeepAlive);
$

CMD(atConnect, UdpConnection const & info, int32_t id)
    // Format:
    // AT+CIPSTART="UDP","192.168.101.110",1000,1002,2
    // when use multiple connections
    // then the first param is connection id
    // AT+CIPSTART=1,"TCP","192.168.101.110",1000
    tx("AT+CIPSTART=");
    id != leaveOut && tx("%d,", id); // skip when id is leaveOut
    tx("%s,%s,%d%+d%+d", 
        info.type,
        info.ipOrDomain, 
        info.port, 
        info.localPort,
        info.mode);
$

// type:
// - 0: Non-certification
// - 1: Load cert and private key, for server authentication
// - 2: Loading cert and private key of CA, Certified server
// - 3: Two-way authentication, SSL client and server to authenticate each otherundefineds certificate
CMD(atSslConfigure, int32_t type, int32_t certKeyId, int32_t CaId, int32_t id)
    tx("AT+CIPSSLCCONF=");
    id != leaveOut && tx("%d,", id);
    tx("%d,%d,%d", type, certKeyId, CaId);
$

CMD(atIpSend, uint8_t const * buffer, int32_t length, int32_t id)
    tx("AT+CIPSEND=");
    id != leaveOut && tx("%d,", id);
    tx("%d", length);
    ///--------------------------------- buffer
$

CMD(atIpSend, uint8_t const * buffer, int32_t length, ipv4 ip, int32_t port, int32_t id)
    tx("AT+CIPSEND=");
    id != leaveOut && tx("%d,", id);
    tx("%d,%i,%d", length, ip, port);
$

// id is not needed when at single connection mode
// when at multiconnection mode
// id range 0~4
// - 5: Close all transmission
CMD(atIpClose, int32_t id)
    tx("AT+CIPCLOSE") && id != leaveOut && 
    tx("=%d", id);
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
    tx("AT+CIPMUX=%d", enable);
$

CMD(atIpMux, bool * enable)
    tx("AT+CIPMUX?");
    rx("+CIPMUX:%b", enable);
$

CMD(atIpServer, bool enable, int32_t port, bool sslCaEnable)
    tx("AT+CIPSERVER=%d%+d%+d", 
        enable, 
        port, 
        port == leaveOut ? leaveOut : int32_t(sslCaEnable));
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

CMD(atServerMaxConnection, int32_t count)
    tx("AT+CIPSERVERMAXCONN=%d", count);
$

CMD(atServiceMaxConnection, int32_t * count)
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
CMD(atSaveConnection, bool passthrough, TcpSslConnection const & info)
    tx("AT+SAVETRANSLINK=%d,%s,%d%+s%+d", 
        passthrough,
        info.ipOrDomain, 
        info.port, 
        info.type,
        info.secondOfKeepAlive);
$

CMD(atSaveConnection, bool passthrough, UdpConnection const & info)
    tx("AT+SAVETRANSLINK=%d,%s,%d%+s%+d", 
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
    tx("AT+CIPSTO", second);
$

CMD(atIpTimeout, int32_t * second)
    tx("AT+CIPSTO?");
    rx("+CIPSTO:%d", second);
$

CMD(atTimeSource, bool enable, TimeSource const & src)
    tx("AT+CIPSNTPCFG=%d%+d%+s%+s%+s", 
        enable, 
        src.timezone,
        src.domain[0],
        src.domain[1],
        src.domain[2]);
$

CMD(atTimeSource, bool * enable, TimeSource * src)
    tx("AT+CIPSNTPCFG?");
    rx("+CIPSNTPCFG:%b", enable) && enable[0] && src &&
    rx(",%d,%s,%s,%s", 
        src->timezone, 
        src->domain[0],
        src->domain[1],
        src->domain[2]);
$

CMD(atDateTime, DateTime * result)
    // Format:
    // +CIPSNTPTIME:Mon Dec 12 02:33:32 2016
    tx("AT+CIPSNTPTIME?");
    rx("+CIPSNTPTIME:%t", result);
$

// // AT+CIUPDATE
// CMD(atUpdateByWifi, int32_t mode)

CMD(atPing, String const & ipOrDomain, int32_t * ms)
    tx("AT+PING=%s", ipOrDomain);
    rx("+PING:%d", ms);
$

