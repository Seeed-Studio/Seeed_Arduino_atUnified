#pragma once
#include"UnifiedAtWifi.h"
#include"UnifiedRingBuffer.h"
#include"Utilities.h"
#define MAX_PACKET_CHANNEL      5

enum WifiState : uint8_t{
    WifiDisconnect,
    WifiConnected,
    WifiGotIp,
    WifiConnectFail,
};

enum TypeAny{
    TypeI08, TypeI32, TypeX32, TypeStr, TypeIp, TypeMac, TypeTime,
};

enum DayOfWeek{
    Sun, Mon, Tue, Wen, Thu, Fri, Sat, NotDayOfWeek
};

enum Month{
    Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec, NotMonth
};

class DateTime{
public:
    //USE THE 32BIT int32_t FOR sscanf
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t dayOfWeek;
};

template<uint32_t length>
struct NetCode{
    operator uint8_t * (){
        return code;
    }
    bool isEmpty(){
        for (size_t i = 0; i < length; i++){
            if (code[i] != 0){
                return false;
            }
        }
        return true;
    }
    NetCode() : code { 0 }{}
private:
    uint8_t code[length];
};

struct X32{ int32_t x; X32(int32_t x = 0) :x(x){} };

typedef std::function<void ()>  handler;
typedef std::vector<WifiApItem> list_ap;
typedef NetCode<4> Ipv4;
typedef NetCode<6> Mac;

struct Any{
    Any(String str) : 
        str(& tmpStr), 
        tmpStr(str){
    }
    Any(int32_t v) : 
        i32(& tmpI32), 
        tmpI32(v){
    }
    Any(Ipv4 v) : 
        ip(& tmpIp), 
        tmpIp(v){
    }
    Any(Mac v) : 
        mac(& tmpMac),
        tmpMac(v){
    }
    Any(DateTime v) : 
        time(& tmpTime), 
        tmpTime(v){
    }
    
    Any(X32 * v) : 
        i32((int32_t *)v){
    }
    Any(int32_t * v) : 
        i32(v){
    }
    Any(int8_t * v) : 
        i8(v){
    }
    Any(bool * v) : 
        i8((int8_t *)v){
    }
    Any(Ipv4 * v) : 
        ip(v){
    }
    Any(Mac * v) : 
        mac(v){
    }
    Any(DateTime * v) : 
        time(v){
    }
    union{
        String  *   str;
        int32_t *   i32;
        int8_t  *   i8;
        Ipv4    *   ip;
        Mac     *   mac;
        DateTime*   time;
    };
private:
    String          tmpStr; // put to union will result memory leak.
    union{
        int32_t     tmpI32;
        Ipv4        tmpIp;
        Mac         tmpMac;
        DateTime    tmpTime;
    };
};

struct Ipd{
    int32_t     length;
    int32_t     port;
    Ipv4        ip;
    std::shared_ptr<uint8_t> 
                data;
};

struct EspStateBar{
private:
    struct AnalysisInvoke{
        template<class ... args>
        AnalysisInvoke(std::function<String &, Any *> && func, args & ... list) : 
            func(func), args{ list... }{
            }
        void invoke(String & resp){
            func(resp, args);
        }
    private:
        std::function<String &, Any *>  func;
        Any                             args[10];
    };
public:
    struct Wifi{
        WifiState state;
        uint32_t  failCode;
        list_ap   apList;
        handler   whenWifiStateChange;
        handler   whenFinishScan;
        std::queue<Ipd> 
                  packet[MAX_PACKET_CHANNEL];
        Wifi(){
            reset();
            whenWifiStateChange = EspStateBar::doNothing;
            whenFinishScan = EspStateBar::doNothing;
        }
        void reset(){
            state = WifiDisconnect;
            failCode = 0;
            apList.clear();

            for (int i = 0; i < MAX_PACKET_CHANNEL; i++){
                while(packet[i].empty() == false){
                    packet[i].pop();
                }
            }
        }
    } wifi;

    struct Smart{
        uint32_t  isSuccess  : 1;
        uint32_t  isFail     : 1;
        handler   whenRising;
        Smart(){
            reset();
            whenRising = EspStateBar::doNothing;
        }
        void reset(){
            isSuccess = 0;
            isFail = 0;
        }
    }smart;

    struct Ping{
        uint32_t  isTimeout  : 1;
        uint32_t  latency    : 15;
        handler   whenRising;
        Ping(){
            reset();
            whenRising = EspStateBar::doNothing;
        }
        void reset(){
            isTimeout = 0;
            latency = 0;
        }
    } ping;

    struct Analysis{
        Analysis(){
            reset();
        }
        void reset(){
            while(que.empty() == false){
                que.pop();
            }
        }
        void push(AnalysisInvoke const & item){
            que.push(item);
        }
        void pop(){
            que.pop();
        }
        void invoke(String & resp){
            que.front().invoke(resp);
        }
    private:
        std::queue<AnalysisInvoke> que;
    } analysis;

    struct Signal{
        void reset(){
            while(cmd.empty()){
                cmd.pop();
            }
        }
        void wait();
        void handleOne();
        void pushRequest(String cmd = String());
        bool containsCmd(){
            return cmd.size() != 0;
        }
        String popCmd(){
            String && v = cmd.front();
            cmd.pop();
            return v;
        }
    private:
        std::queue<String> cmd;
    } signal;

    void resetFlag(){
        wifi.resetFlag();
        smart.resetFlag();
        ping.resetFlag();
    }
    void eventHandler();

    handler      whenReset;
private:
    
    template<class ... args>
    String readUntil(char a, args ... list){
        char group[] = { a, list..., '\0' };
        return readUntil(group);
    }
    String readUntil(char * token);
    int read();
    int available();

    static void doNothing(){}
};

extern EspStateBar esp;

template<class ... args>
void rx(const char * token, args ... list){
    esp.analysis.push({
        [&](String & resp, Any * buf){
            if (resp.startsWith(token) == false){
                return;
            }
            rxMain(readUntil('\n'), buf);
            esp.analysis.pop();
        }, list..., nullptr // nullptr as end of args
    });
}

template<class ... args>
void tx(const char * token, args const & ... list){
    String  cmd = token;
    Any     param[] = { token, list..., nullptr }; // nullptr as end of args
    txMain(& cmd, param);
    esp.signal.pushRequest(cmd);
}

