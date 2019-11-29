#include"UnifiedAtEvent.h"
#include"UnifiedRingBuffer.h"
#include"Utilities.h"

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
#endif

#define T_US            1000000.0
#define T_MS            1000.0
#define ESP_BYTE_RATE   (115200.0 / 11)
#define MAX_DESIRED     50.0
#define COST_US(bytes)  uint32_t(T_US / ESP_BIT_RATE * (bytes))
#define RT_SLEEP_US(us) 
#define ei              else if
#define es              else

bool parseInt(char ** p, char type, int32_t * v){
    typedef int(* match_t)(int);
    auto end = p[0];
    auto match = type == 'x' ? match_t(& isxdigit): match_t(& isdigit);
    char fmt[] = { '%', type, '\0' };
    if (end[0] == '-' || end[0] == '+'){
        end++;
    }
    while(match(end[0])){
        end += 1;
    }
    if (sscanf(p[0], fmt, v) == 0){
        return fail;
    }
    p[0] = end;
    return success;
};

//parse mac/ip
//Mac format
//12:34:56:78:9a:bc
//ip format
//192.168.1.1
bool parseNetCode(char ** p, char type, uint8_t * buf, size_t length){

    // NOTICE:
    // some at response line use the "12:34:56:78:9a:bc" format, 
    // but some use 12:34:56:78:9a:bc
    // shit design!!!
    if (p[0][0] == '\"'){
        p[0] += 1;
    }
    for (int32_t i = 0, v; i < length; i++){
        if (parseInt(p, type, & v) == fail){
            return fail;
        }

        buf[i] = int32_t(v);

        switch(p[0][0]){
        case ':':
        case '.':
        case '\"':
            p[0] += 1;
            break;
        }
    }
};

bool parseDateTime(char ** p, DateTime * time){
    // Format:
    // Mon Dec 12 02:33:32 2016
    char *   str = p[0];
    auto     dayOfWeek = 
    !strncmp(str, "Sun", 3) ? Sun :
    !strncmp(str, "Mon", 3) ? Mon :
    !strncmp(str, "Tue", 3) ? Tue :
    !strncmp(str, "Wen", 3) ? Wen :
    !strncmp(str, "Thu", 3) ? Thu :
    !strncmp(str, "Fri", 3) ? Fri :
    !strncmp(str, "Sat", 3) ? Sat : NotDayOfWeek;

    if (dayOfWeek == NotDayOfWeek){
        return fail;
    }

    str += 4;

    auto month = 
    !strncmp(str, "Jan", 3) ? Jan :
    !strncmp(str, "Feb", 3) ? Feb :
    !strncmp(str, "Mar", 3) ? Mar :
    !strncmp(str, "Apr", 3) ? Apr :
    !strncmp(str, "May", 3) ? May :
    !strncmp(str, "Jun", 3) ? Jun :
    !strncmp(str, "Jul", 3) ? Jul :
    !strncmp(str, "Aug", 3) ? Aug :
    !strncmp(str, "Sep", 3) ? Sep :
    !strncmp(str, "Oct", 3) ? Oct :
    !strncmp(str, "Nov", 3) ? Nov :
    !strncmp(str, "Dec", 3) ? Dec : NotMonth;

    if (month == NotMonth){
        return fail;
    }

    str += 4;
    time->dayOfWeek = dayOfWeek;
    time->month     = month;
    parseInt(& str, 'd', & time->day   ); str += 1; // skip ' '
    parseInt(& str, 'd', & time->hour  ); str += 1; // skip ':'
    parseInt(& str, 'd', & time->minute); str += 1; // skip ':'
    parseInt(& str, 'd', & time->second); str += 1; // skip ' '
    parseInt(& str, 'd', & time->year  );
    p[0] = str;
    return success;
}

void rxMain(String & resp, Any * buf){
    for(char * str = resp.c_str(); buf->isEmpty() == false; buf += 1, str += 1){ // skip ,
        switch(buf->type){
        case TypeI08: 
            int32_t i32;
            parseInt(& str, 'd', & i32); 
            buf->i8[0] = i32;
            break;
        case TypeI32: 
            parseInt(& str, 'd', & buf->i32); 
            break;
        case TypeX32: 
            parseInt(& str, 'x', & buf->i32); 
            break;
        case TypeStr: 
            parseStr(& str, & buf->str);
            break;
        case TypeIp:
            parseNetCode(& str, 'd', & buf->ip);
            break;
        case TypeMac:
            parseNetCode(& str, 'x', & buf->mac);
            break;
        case TypeTime:
            parseTime(& str, & buf->time);
            break;
        default: debug("ERROR TYPE %d in parse.\n", buf->type); break;
        }
    }
}

void txMain(String * resp, Any * buf){
    char buf[32];
    resp[0] = "";
    while(true){
        switch(buf->type){
        case TypeI08: 
        case TypeI32: 
            itoa(buf, 10, buf->i32[0]);
            resp[0] += buf;
            break;
        case TypeStr: 
            resp[0] += '\"';
            resp[0] += str[0];
            resp[0] += '\"';
            break;
        case TypeIp:
            parseNetCode(& str, 'd', buf->ip[0]);
            break;
        case TypeMac:
            parseNetCode(& str, 'x', buf->mac[0]);
            break;
        case TypeTime:
            parseTime(& str, buf->time[0]);
            break;
        default: debug("ERROR TYPE %d in parse.\n", buf->type); break;
        }
        buf += 1;

        if (buf->isEmpty() == false){
            resp[0] += ",";
        }
    }
    resp[0] += END_LINE;
}

EspStateBar esp;
int         itrRxPin = digitalPinToInterrupt(PIN_RX);

void detectRxFalling(){
    detachInterrupt(itrRxPin);
    esp.signal.pushRequest();
}

String EspStateBar::readUntil(char * token){
    for(std::vector<char> buf;;){
        if (available() <= 0){
            RT_SLEEP_US(COST_US(MAX_DESIRED));
        }

        char c = read();
        buf.push_back(c);

        if (strchr(token, c) != nullptr){
            buf.push_back(0);
            return & buf.front();
        }
    }
}

void EspStateBar::Signal::wait(){

}
void EspStateBar::Signal::handleOne(){

}
void EspStateBar::Signal::pushRequest(String cmd){

}

// default:
// - wifi multi-connection
// - close echo
// - show IPD remote ip:port
void EspStateBar::eventHandler(){
    while(true){
        signal.wait();
        signal.handleOne();

        if (signal.containsCmd()){
            writeLine(signal.popCmd());
        }
        es{
            RT_SLEEP_US(COST_US(MAX_DESIRED));
        }

        resp = readUntil(':', '\n');

        // async signal
        // system reset
        if (resp.startsWith("ready")){
            resetFlag();
            whenReset();
        }
        ei (resp.startsWith("WIFI CONNECTED")){
            wifi.state = WifiConnected;
            wifi.whenWifiStateChange();
        }
        ei (resp.startsWith("WIFI GOT IP")){
            wifi.state = WifiGotIp;
            wifi.whenWifiStateChange();
        }
        ei (resp.startsWith("WIFI DISCONNECT")){
            wifi.state = WifiDisconnect;
            wifi.whenWifiStateChange();
        }
        ei (resp.startsWith("smartconfig connected wifi")){
            smart.isFail = 1;
            smart.isSuccess = 0;
            smart.whenRising();
        }
        ei (resp.startsWith("smartconfig connect fail")){
            smart.isFail = 0;
            smart.isSuccess = 1;
            smart.whenRising();
        }
        ei (resp.startsWith("+IPD")) {
            int32_t id;
            Ipd     ipd;
            sscanf(resp.c_str(), "+IPD,%d,%d,%d.%d.%d.%d,%d", 
                & id, 
                & ipd.length, 
                & ipd.ip[0], 
                & ipd.ip[1], 
                & ipd.ip[2], 
                & ipd.ip[3],
                & ipd.port
            );
            ipd.data = new uint8_t[ipd.length];

            for (int32_t i = 0; i < ipd.length; i++){
                if (available()){
                    ipd.data.get()[i] = read();
                }
                es{
                    RT_SLEEP_US(COST_US(ipd.length - i));
                }
            }
            wifi.packet[id].push(ipd);
            wifi.whenRxPacket();
        }

        // rx invoke
        ei (analysis.size()){
            analysis.invoke(resp);
        }
    }
}



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

// #include <Seeed_Arduino_FreeRTOS.h> 

// TaskHandle_t Handle_aTask;

// void delayus(int us){
//     vTaskDelay(us / portTICK_PERIOD_US);
// }

// void delayms(int ms){
//     vTaskDelay((ms * 1000) / portTICK_PERIOD_US);  
// }

// void listen(void *){
//     while(true){
//         if (available()){
            
//         }
//         esp.eventHandler();
//         delayms(1);
//     }
// }

// struct Background{
//     Listen(){
//         xTaskCreate(listen, "Task A", 1024, NULL, tskIDLE_PRIORITY + 3, & Handle_aTask);
//         vTaskStartScheduler();
//     }
// }background;
