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

volatile bool change = false;

void * memoryAlloc(size_t size){
    void * ptr;
    if (change == false){
        if (ptr = malloc(size)){
            return ptr;
        }
    }
    Rtos::malloc(& ptr, size); 
    return ptr;
}

void memoryFree(void * ptr){
    if (change == false){
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
    volatile bool       inIPD = false;
}

#ifdef PIN_RX

    void sendSignal(void * serial, char chr){
        if (serial != & com){
            return;
        }

        static std::vector<char> tmp;
        tmp.push_back(chr);
        tmp.push_back('\0');
        bool matchIPD = Text(& tmp[0]).startsWith("+IPD") && inIPD == false;

        // if (chr == '\n' || matchIPD){
        // debug("%c", chr);

        if (chr == '\n' || (matchIPD && chr == ':')){
            inIPD = true;
            tmp.clear();
            Rtos::giveFromISR(esp.semaWaitRx);
        }
        else{
            tmp.pop_back();
        }
    }
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

    // default:
    // - wifi multi-connection
    // - close echo
    // - show IPD remote ip:port
    void EspStateBar::reset(){
        wifi.reset();
    }
#else
    int EspStateBar::available(){
        return -1;
    }

    int EspStateBar::read(){
        return -1;
    }

    void EspStateBar::begin(){}

    void EspStateBar::write(Text value){}

    // default:
    // - wifi multi-connection
    // - close echo
    // - show IPD remote ip:port
    void EspStateBar::reset(){}
#endif

EspStateBar         esp;

// EVENT --------------------------------------------------------------------
void EspStateBar::Wifi::whenReceivePacket(int32_t id) {

}
void EspStateBar::Wifi::whenScanFinished(ListAp & list) {

}
void EspStateBar::Wifi::whenScanFailed(int32_t failCode) {

}
void EspStateBar::Wifi::whenStateChanged(WifiState state) {

}
void EspStateBar::Smart::whenRising(bool isSuccess) {

}
void EspStateBar::Ping::whenRising(bool isTimeOut, uint32_t latency) {

}
void EspStateBar::whenReset() {
    
}

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
            esp.wifi.whenStateChanged(WifiConnected);
        }
    }, { 
        "WIFI GOT IP", [](EspStateBar & esp, Text &){
            esp.wifi.whenStateChanged(WifiGotIp);
        }
    }, { 
        "WIFI DISCONNECT", [](EspStateBar & esp, Text &){
            esp.wifi.whenStateChanged(WifiDisconnect);
        }
    }, { 
        "smartconfig connected wifi", [](EspStateBar & esp, Text &){
            esp.smart.whenRising(success);
        }
    }, {
        "smartconfig connect fail", [](EspStateBar & esp, Text &) {
            esp.smart.whenRising(fail);
        }
    }, {
        "OK", [](EspStateBar & esp, Text &) {
            // Rtos::queuePushBack(esp.semaWaitFlag, success);
            esp.flag = success;
            esp.semaWaitFlag = 1;
        }
    }, {
        "ERROR", [](EspStateBar & esp, Text &) {
            // Rtos::queuePushBack(esp.semaWaitFlag, fail);
            esp.flag = fail;
            esp.semaWaitFlag = 1;
        }
    }, {
        "+IPD", [](EspStateBar & esp, Text & resp){
            int32_t id;
            Ipd     ipd;

            // +5 to skip "+IPD,"
            esp.rx(resp.c_str() + 5, & id, & ipd.length, & ipd.ip, & ipd.port);
            ipd.data = std::shared_ptr<uint8_t>(new uint8_t[ipd.length]);

            for (int32_t i = 0; i < ipd.length; i++){
                if (esp.available()){
                    ipd.data.get()[i] = esp.read();
                }
                es{
                    Rtos::delayus(COST(ipd.length - i));
                }
            }
            esp.wifi.packet[id].push(ipd);
            esp.wifi.whenReceivePacket(id);
        }
    },
};

Text EspStateBar::readUntil(char * token){
    char              c;
    std::vector<char> buf;
    while(true){
        while (available() <= 0){
            Rtos::take(semaWaitRx);
        }
        c = read();
        buf.push_back(c);

        if (strchr(token, c) != nullptr){
            buf.push_back(0);
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
    // bool flag;
    // Rtos::queueReceive(semaWaitFlag, & flag);
    // return flag;
    while (ms) {
        Rtos::delayus(500); if (semaWaitFlag) break;
        Rtos::delayus(500); if (semaWaitFlag) break;
        if (~ms){
            ms -= 1;
        }
        if (ms == 0){
            return timeout;
        }
    }
    // debug("exit\n");
    semaWaitFlag = 0;
    return flag;
}

void fore(void * handle) {
    extern void setup();
    extern void loop();
    
    self.begin();
    tx("ATE0");         self.waitFlag();
    tx("AT+CIPMUX=1");  self.waitFlag();
    tx("AT+CIPDINFO=1");self.waitFlag();

    while(true){
        loop();
        yield(); // yield run usb background task

        if (serialEventRun) {
            serialEventRun();
        }
    }
}

void back(void * handle){
    self.eventHandler();
}

void EspStateBar::run(){
    change = true;
    // Rtos::createQueue(& semaWaitFlag, 2, sizeof(bool));
    Rtos::createSemaphore(& semaWaitRx, 1);
    // Rtos::createQueue(& analysis.handle, 5, analysis.itemSize);
    Rtos::createThread(& taskBack, "ESP-BG", back, this, 1024, 1);
    Rtos::createThread(& taskFore, "ESP-FG", fore, this, 1024, 1);
    Rtos::scheduler();
}

//
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
