#include"UnifiedAtEvent.h"

// enum EventType{
//     WifiConnected,
//     WifiGotIp,
//     WifiDisconnect,
//     WifiConnectionError,
// };

extern String readLine();
extern String peekPrefix();
extern void   clearPrefix();

void atDefaultConfig(){
    tx("ATE0") && waitFlag() == success &&          // close echo.
    tx("AT+CIPMUX=1") && waitFlag() == success &&   // enable multi-connection
    enable;
}

EspStateBar esp;

void atWifiScanSubfunction(String current){
    auto & list = esp.wifi.apList;
    list.clear();

    auto lastIndexOf = [](const char * line, char chr, size_t times){
        for (int i = strlen(line); i-- > 0; ){
            if (line[i] != chr){
                continue;
            }
            if (--times == 0){
                return i;
            }
        }
        return -1;
    };

    while (current.startsWith("+CWLAP:")){
        // char    line[] = "+CWLAP:(3,\"POT\",-AL00a\",-53,\"12:34:56:78:9a:bc\",1)";
        char *  line = (char *)current.c_str();
        char *  p = nullptr;
        char *  t;
        int32_t mac[6]; //need 32bit when use sscanf
        int32_t ecn;
        int32_t rssi;
        int32_t channel;

        p = line + lastIndexOf(line, ',', 3);

        sscanf(line, "+CWLAP:" "(%d,", & ecn);
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

        WifiApItem info;
        info.ecn = ecn;
        info.ssid = strchr(line, ',') + 2; //skip ',' '\"'
        info.rssi = rssi;
        copy<uint8_t, int32_t>(info.bssid, mac, 6);
        info.channel = channel;
        list.push_back(info);
        current = readLine();
    }

    if (esp.wifi.whenFinishScan){
        esp.wifi.whenFinishScan();
    }
}

void atEventHandler(){
    String resp = peekPrefix();
    String line;

    if (resp.startsWith("+IPD")){
        int32_t len;
        int32_t port = 0;
        int32_t ip[4] = { 0 };
        clearPrefix();
        sscanf(resp.c_str(), "+IPD,%d,%d.%d.%d.%d,%d", 
            & len, & ip[0], & ip[1], & ip[2], & ip[3], & port
        ); 
        return;
    }
    else{
        line = readLine();
    }
    if (line == "WIFI CONNECTED"){
        esp.wifi.connected = 1;
        return;
    }
    if (line == "WIFI GOT IP"){
        esp.wifi.gotIp = 1;
        return;
    }
    if (line == "WIFI DISCONNECT"){
        esp.wifi.connected = 0;
        esp.wifi.gotIp = 0;
        esp.wifi.disconnect = 1; 
        esp.wifi.whenWifiConnected();
        return;
    }

    // system reset
    if (line.startsWith("ready")){
        esp.resetFlag();
        return;
    }
    if (line.startsWith("+CWJAP")){
        sscanf(line.c_str(), "+CWJAP:%d", & esp.wifi.failCode);
        return;
    }
    if (line.startsWith("+CWLAP")){
        atWifiScanSubfunction(line);
        return;
    }
    if (line.startsWith("smartconfig connected wifi")){
        esp.smart.isFail = 1;
        esp.smart.isSuccess = 0;
        esp.smart.whenRising();
        return;
    }
    if (line.startsWith("smartconfig connect fail")){
        esp.smart.isFail = 0;
        esp.smart.isSuccess = 1;
        esp.smart.whenRising();
        return;
    }
    if (line.startsWith("+PING")){
        esp.ping.isTimeout = line == "+PING:TIMEOUT";
        if (esp.ping.isTimeout == 0){
            int latency;
            sscanf(line.c_str(), "+PING:%d", & latency);
            esp.ping.latency = latency;
        }
        esp.ping.whenRising();
        return;
    }
}

