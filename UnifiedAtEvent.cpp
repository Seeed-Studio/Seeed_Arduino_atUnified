#define private public
#include"Uart.h"
#include"UnifiedAtEvent.h"
#include"Utilities.h"
#include"UnifiedBackTask.h"

#define T_US            1000000.0
#define T_MS            1000.0
#define ESP_BIT_RATE    115200
#define ESP_BYTE_RATE   (1.0 * ESP_BIT_RATE / 11)
#define MAX_DESIRED     50.0
#define COST(bytes)     uint32_t(T_US / ESP_BYTE_RATE * (bytes))
#define ei              else if
#define es              else
#define self            (*(EspStateBar *)handle)

volatile bool isUseRtosMemory = false;

void * memoryAlloc(size_t size){
    void * ptr;
    if (isUseRtosMemory == false){
        if (ptr = malloc(size)){
            return ptr;
        }
    }
    Rtos::malloc(& ptr, size);
    return ptr;
}

void memoryFree(void * ptr){
    if (isUseRtosMemory == false){
        free(ptr);
    }
    else{
        Rtos::free(ptr);
    }
}

void * operator new(size_t size) {
    return memoryAlloc(size);
}

void * operator new[](size_t size) {
    return memoryAlloc(size);
}

void operator delete(void * ptr) {
    memoryFree(ptr);
}

void operator delete[](void * ptr) {
    memoryFree(ptr);
}

namespace{
    #ifdef USE_ESP32_AT_SERIAL
        #define PIN_RX
        auto & com = Serial;
    #elif defined USE_ESP32_AT_SERIAL2
        #define PIN_RX
        auto & com = Serial2;
    #elif defined USE_ESP32_AT_SERIAL1
        #define PIN_RX
        auto & com = Serial1;
    #endif
    volatile bool ipdMode = false;
}

#ifdef PIN_RX

    // void sendSignal(void * serial, char chr){
    //     if (serial != & com){
    //         return;
    //     }
    //     // debug("%c", chr);
    //     // static uint64_t word = 0;
    //     // uint64_t want = '+' << 32 | 'I' << 24 | 'P' << 16 | 'D' << 8 | ':';
    //     // auto flag = (want & word) == want && ipdMode == false;
    //     // word = word << 8 | chr;
    //     // Rtos::giveFromISR(esp.semaWaitRx);
    //     // com.rxBuffer.clear();
    //     // uint32_t size;
    //     // Rtos::queueSize(esp.rxQueue, & size);
    //     // Rtos::queueSendFromISR(esp.rxQueue, chr);
    // }

    int EspStateBar::available(){
        return com.available();
    }

    int EspStateBar::read(){
        return com.read();
    }

    void EspStateBar::begin(){
        com.begin(ESP_BIT_RATE);
    }

    void EspStateBar::write(Text value){
        com.print(value.c_str());
    }

    void EspStateBar::txBin(uint8_t const * buffer, size_t length){
        com.write(buffer, length);
    }

    // default:
    // - wifi multi-connection
    // - close echo
    // - show IPD remote ip:port
    void EspStateBar::reset(){
        wifi.reset();
        tx("ATE0");          flush(); waitFlag();
        tx("AT+CIPMUX=1");   flush(); waitFlag();
        tx("AT+CIPDINFO=1"); flush(); waitFlag();
    }
#else
    int EspStateBar::available(){ return -1; }
    int EspStateBar::read(){ return -1; }
    void EspStateBar::begin(){}
    void EspStateBar::write(Text value){}
    void EspStateBar::reset(){}
#endif

EspStateBar         esp;

struct TokenEventPair{
    typedef std::function<void(EspStateBar &, Text &)> Invoke;
    const char * token;
    Invoke       invoke;
};

std::initializer_list<TokenEventPair> responesMap = { 
    { 
        "ready", [](EspStateBar & esp, Text &){
            esp.reset();
            esp.whenReset();
        }
    }, { 
        "WIFI CONNECTED", [](EspStateBar & esp, Text &){
            esp.wifi.state = WifiConnected;
            esp.wifi.whenStateChanged();
        }
    }, { 
        "WIFI GOT IP", [](EspStateBar & esp, Text &){
            esp.wifi.state = WifiGotIp;
            esp.wifi.whenStateChanged();
        }
    }, { 
        "WIFI DISCONNECT", [](EspStateBar & esp, Text &){
            esp.wifi.state = WifiDisconnect;
            esp.wifi.whenStateChanged();
        }
    }, { 
        "smartconfig connected wifi", [](EspStateBar & esp, Text &){
            esp.smart.state = Success;
            esp.smart.whenRising();
        }
    }, {
        "smartconfig connect Fail", [](EspStateBar & esp, Text &) {
            esp.smart.state = Fail;
            esp.smart.whenRising();
        }
    }, {
        "OK", [](EspStateBar & esp, Text &) {
            if (esp.wait == WaitWifiScan){
                esp.wifi.whenScanFinished();
            }
            esp.wait = WaitNothing;
            esp.flag = Success;
            Rtos::give(esp.semaWaitCmd); // for waitFlag
            Rtos::give(esp.semaWaitCmd); // for tx
        }
    }, {
        "ERROR", [](EspStateBar & esp, Text &) {
            if (esp.wait == WaitWifiScan){
                esp.wifi.whenScanFailed();
            }
            esp.wait = WaitNothing;
            esp.flag = Fail;
            Rtos::give(esp.semaWaitCmd); // for waitFlag
            Rtos::give(esp.semaWaitCmd); // for tx
        }
    }, {
        "+IPD", [](EspStateBar & esp, Text & resp){
            int32_t id;
            Ipd     ipd;

            // +5 to skip "+IPD,"
            esp.rx(resp.c_str() + 5, & id, & ipd.length, & ipd.ip, & ipd.port);
            ipd.data = std::shared_ptr<uint8_t>(new uint8_t[ipd.length]);
            ipd.i = 0;

            for (int32_t i = 0; i < ipd.length; i++){
                if (esp.available()){
                    ipd.data.get()[i] = esp.read();
                }
                es{
                    Rtos::delayus(COST(ipd.length - i));
                }
            }
            ipdMode = false;
            esp.wifi.packet[id].push(ipd);
            esp.wifi.packId = id;
            esp.wifi.whenReceivePacket();
        }
    },
};

Text EspStateBar::readUntil(char * token){
    std::vector<char> buf;
    char        c;
    while(true){
        while (available() <= 0){
            Rtos::delayms(1);
        }

        c = read();
        buf.push_back(c);
        // debug("%c", c);

        if (strchr(token, c) != nullptr){
            buf.push_back('\0');
            return Text(& buf.front());
        }
    }
}

void EspStateBar::eventHandler(){
    while (true) {
        Text && resp = readUntil(':', '\n');
        if (resp.length() == 0){
            continue;
        }

        // first
        for (auto & event : responesMap) {
            if (resp.startsWith(event.token)) {
                event.invoke(this[0], resp);
                break;
            }
        }

        // second analysis
        analysis.invoke(resp);
    }
}

Result EspStateBar::waitFlag(uint32_t ms) {
    Rtos::take(semaWaitCmd, Rtos::msToTicks(ms));
    return flag;
}

// setup and loop code block
extern void body();

// FREE RTOS IDLE function
extern "C" void vApplicationIdleHook(){}

// ESP-LIB function
extern bool __attribute__((weak)) _start_network_event_task(){}

void fore(void * handle) {
    _start_network_event_task();
    self.begin();
    self.reset();
    body();
}

void back(void * handle){
    self.eventHandler();
}

void espAtShell(){
    esp.run();
    while(1);
}

void EspStateBar::run(){
    isUseRtosMemory = true;
    Rtos::createSemaphore(& semaWaitCmd, 2 /*max*/, 1 /*for send first cmd.*/);
    Rtos::createThread(& taskBack, "ESP-BG", back, this, 1024, 1 /*priority*/);
    Rtos::createThread(& taskFore, "ESP-FG", fore, this, 2048, 1);
    Rtos::scheduler();
}

//void atWifiScanSubfunction(Text current){
//    auto & list = esp.wifi.apList;
//    list.clear();
//
//    auto lastIndexOf = [](const char * line, char chr, size_t times){
//        for (int i = strlen(line); i-- > 0; ){
//            if (line[i] != chr){
//                continue;
//            }
//            if (--times == 0){
//                return i;
//            }
//        }
//        return -1;
//    };
//
//    while (current.startsWith("+CWLAP:")){
//        // char    line[] = "+CWLAP:(3,\"POT\",-AL00a\",-53,\"12:34:56:78:9a:bc\",1)";
//        char *  line = (char *)current.c_str();
//        char *  p = nullptr;
//        char *  t;
//        int32_t mac[6]; //need 32bit when use sscanf
//        int32_t ecn;
//        int32_t rssi;
//        int32_t channel;
//
//        p = line + lastIndexOf(line, ',', 3);
//
//        sscanf(line, "+CWLAP:" "(%d,", & ecn);
//        sscanf(p + 1, "%d,\"%x:%x:%x:%x:%x:%x\",%d", 
//            & rssi,
//            & mac[0],
//            & mac[1],
//            & mac[2],
//            & mac[3],
//            & mac[4],
//            & mac[5],
//            & channel
//        );
//
//        //replace '\"' to '\0'
//        p[0] = ',';
//        p[-1] = '\0';
//
//        WifiApItem info;
//        info.ecn = ecn;
//        info.ssid = strchr(line, ',') + 2; //skip ',' '\"'
//        info.rssi = rssi;
//        copy<uint8_t, int32_t>(info.bssid, mac, 6);
//        info.channel = channel;
//        list.push_back(info);
//        current = readLine();
//    }
//
//    if (esp.wifi.whenFinishScan){
//        esp.wifi.whenFinishScan();
//    }
//}
//
