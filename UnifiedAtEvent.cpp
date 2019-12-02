#define private public
#include"UnifiedAtEvent.h"
#include"Utilities.h"
#include"Seeed_Arduino_FreeRTOS.h"

#define T_US            1000000.0
#define T_MS            1000.0
#define ESP_BIT_RATE    115200
#define ESP_BYTE_RATE   (1.0 * ESP_BIT_RATE / 11)
#define MAX_DESIRED     50.0
#define COST_US(bytes)  uint32_t(T_US / ESP_BYTE_RATE * (bytes))
#define RT_SLEEP_US(us) vTaskDelay((us / portTICK_PERIOD_US));
#define ei              else if
#define es              else
#define debug(...)      Serial.printf(__VA_ARGS__)

#ifdef USE_AT_SERIAL
    #define PIN_RX
    auto & com = Serial;
#elif defined USE_AT_SERIAL2
    #define PIN_RX      PIN_SERIAL2_RX
    auto & com = Serial2;
#else
    #define PIN_RX      PIN_SERIAL1_RX
    auto & com = Serial1;
#endif

#ifdef PIN_RX
    int EspStateBar::available(){
        return com.available();
    }

    int EspStateBar::read(){
        return com.read();
    }

    void atBegin() {
        com.begin(ESP_BIT_RATE);
    }

    // default:
    // - wifi multi-connection
    // - close echo
    // - show IPD remote ip:port
    void atDefault() {
        auto skip = [](){
            while (true){
                auto line = com.readStringUntil('\n');
                if (line.length() == 0){
                    break;
                }
                debug("%s", line.c_str());
            }
        };
        skip(); com.println("ATE0");
        skip(); com.println("AT+CIPMUX=1");
        skip(); com.println("AT+CIPDINFO=1");
        skip();
        
    }
#endif

TaskHandle_t        espTask;
TaskHandle_t        espFore;
EspStateBar         esp;
int                 itrRxPin = digitalPinToInterrupt(PIN_RX);

void take(void * sema){
    xSemaphoreTake((SemaphoreHandle_t)sema, portMAX_DELAY);
}
void give(void * sema) {
    xSemaphoreGive((SemaphoreHandle_t)sema);
}

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
    atDefault();
}

struct TokenEventPair{
    const char *                    token;
    std::function<void(String &)>   invoke;
};

std::initializer_list<TokenEventPair> responesMap = { 
    { 
        "ready", [](String &){
            esp.reset();
            esp.whenReset();
        }
    }, { 
        "WIFI CONNECTED", [](String &){
            esp.wifi.whenStateChanged(WifiConnected);
        }
    }, { 
        "WIFI GOT IP", [](String &){
            esp.wifi.whenStateChanged(WifiGotIp);
        }
    }, { 
        "WIFI DISCONNECT", [](String &){
            esp.wifi.whenStateChanged(WifiDisconnect);
        }
    }, { 
        "smartconfig connected wifi", [](String &){
            esp.smart.whenRising(success);
        }
    }, {
        "smartconfig connect fail", [](String &) {
            esp.smart.whenRising(fail);
        }
    }, {
        "OK", [](String &) {
            esp.flag = success;
            give(esp.semaWaitFlag);
        }
    }, {
        "ERROR", [](String &) {
            esp.flag = fail;
            give(esp.semaWaitFlag);
        }
    }, {
        "+IPD", [](String & resp){
            int32_t id;
            Ipd     ipd;
            esp.rx(resp.c_str() + 5, & id, & ipd.length, & ipd.ip, & ipd.port);
            ipd.data = std::shared_ptr<uint8_t>(new uint8_t[ipd.length]);

            for (int32_t i = 0; i < ipd.length; i++){
                if (esp.available()){
                    ipd.data.get()[i] = esp.read();
                }
                es{
                    RT_SLEEP_US(COST_US(ipd.length - i));
                }
            }
            esp.wifi.packet[id].push(ipd);
            esp.wifi.whenReceivePacket(id);
        }
    },
};

void atRxFalling() {
    auto flag = pdFALSE;
    detachInterrupt(itrRxPin);
    xSemaphoreGiveFromISR((SemaphoreHandle_t)esp.semaWaitRx, &flag);
    debug("start bit\n");
}

void atSetRxInterrupt() {
    attachInterrupt(itrRxPin, atRxFalling, FALLING);
}

String EspStateBar::readUntil(char * token){
    for(std::vector<char> buf;;){
        while (available() <= 0){
            RT_SLEEP_US(COST_US(MAX_DESIRED));
        }

        char c = read();
        buf.push_back(c);
        debug("%c", c);

        if (strchr(token, c) != nullptr){
            buf.push_back(0);
            return String(& buf.front());
        }
    }
}

void EspStateBar::eventHandler(){
    while(true){
        while(!Serial);
        take(semaWaitRx);
        debug("wait\n");
        RT_SLEEP_US(COST_US(MAX_DESIRED));
        debug("wait\n");
        while (true) {
            String && resp = readUntil(':', '\n');
            Serial.print(resp);
            for (auto & i : responesMap) {
                if (resp.startsWith(i.token)) {
                    i.invoke(resp);
                    resp = "";
                }
            }
            if (resp.length()) {
                analysis.invoke(resp);
            }
            if (available() <= 0) {
                atSetRxInterrupt();
                break;
            }
        }
    }
}

bool EspStateBar::waitFlag() {
    take(semaWaitFlag);
    return flag;
}

void listen(void *){
    esp.eventHandler();
}

void foreground(void *) {
    extern void setup();
    extern void loop();
    extern int  main();
    atSetRxInterrupt();
    Serial.begin(115200);
    while(1){
        while (Serial.available() > 0){
            char c = Serial.read();
            Serial1.print(c);
        }
        RT_SLEEP_US(500);
    }
}


void espBegin(){
    atBegin();
    atDefault();
    esp.semaWaitRx = xSemaphoreCreateCounting(2, 0);
    esp.semaWaitFlag = xSemaphoreCreateCounting(2, 0);
    xTaskCreate(listen, "ESP-BG", 512, NULL, tskIDLE_PRIORITY + 1, & espTask);
    xTaskCreate(foreground, "ESP-FG", 1024, NULL, tskIDLE_PRIORITY + 1, & espFore);
    vTaskStartScheduler();
}

//
//void atWifiScanSubfunction(String current){
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
