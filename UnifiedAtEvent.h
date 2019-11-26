#pragma once
#include"UnifiedAtWifi.h"
#include"UnifiedRingBuffer.h"
#include"Utilities.h"

typedef std::function<void ()>  handler;
typedef std::vector<WifiApItem> list_ap;

struct EspStateBar{
private:
    typedef struct UdpContext : UnifiedRingBuffer<uint8_t>{
        ipv4    remoteIp;
        int32_t remotePort;
    } link;
    
public:
    struct Wifi{
        uint32_t connected  : 1;
        uint32_t gotIp      : 1;
        uint32_t disconnect : 1;
        list_ap  apList;
        handler  whenWifiConnected;
        handler  whenFinishScan;

        void resetFlag(){
            connected = 0;
            gotIp = 0;
            disconnect = 0;
            apList.clear();
            whenWifiConnected = EspStateBar::doNothing;
            whenFinishScan = EspStateBar::doNothing;
        }
    } wifi;

    struct Smart{
        uint32_t isSuccess  : 1;
        uint32_t isFail     : 1;
        handler  whenRising;

        void resetFlag(){
            isSuccess = 0;
            isFail = 0;
            whenRising = EspStateBar::doNothing;
        }
    }smart;

    struct Ping{
        uint32_t isTimeout  : 1;
        uint32_t latency    : 15;
        handler  whenRising;

        void resetFlag(){
            isTimeout = 0;
            latency = 0;
            whenRising = EspStateBar::doNothing;
        }
    } ping;

    void resetFlag(){
        wifi.resetFlag();
        smart.resetFlag();
        ping.resetFlag();
    }
private:
    static void doNothing(){}
};

extern EspStateBar esp;

