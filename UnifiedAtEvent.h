#pragma once
#include"UnifiedAtType.h"
#include"UnifiedAtWifi.h"
#include"UnifiedBackTask.h"
#define MAX_PACKET_CHANNEL      5

enum WifiState : uint8_t{
    WifiDisconnect,
    WifiConnected,
    WifiGotIp,
    WifiConnectFail,
};

struct Ipd{
    int32_t         length;
    int32_t         port;
    Ipv4            ip;
    std::shared_ptr<uint8_t> 
                    data;
};

typedef std::vector<WifiApItem> ListAp;
typedef std::queue<Ipd> IpdQue;

struct EspStateBar{
private:
    template<class first, class ... args>
    static void setAny(Any * any, first a, args ... list){
        any[0] = Any(a);
        setAny(any + 1, list...);
    }
    static void setAny(Any * any){}

    template<class ... args>
    static Any * newAny(args ... list){
        Any * ptr = new Any[sizeof...(args)];
        setAny(ptr, list...);
        return ptr;
    }

    struct AnalysisInvoke;
    typedef std::function<void (AnalysisInvoke &, Text &, Any *)> Invoke;
    typedef std::function<void ()> CallBack;
    struct AnalysisInvoke{
        AnalysisInvoke(){
            arg = nullptr;
            token = "";
        }

        template<class ... args>
        AnalysisInvoke(Invoke func, const char * token, CallBack const & callback, args ... list) : 
            func(func), token(token), callback(callback){
                arg = newAny(list...);
            }
        
        void invoke(Text & resp){
            if (func){
                func(this[0], resp, arg);
            }
        }
        void relase(){
            if (arg){
                delete [] arg; arg = nullptr;
            }
        }
        const char *    token;
        CallBack        callback;
    private:
        template<class first, class ... args> void set(first a, args ... list){
            arg[0] = Any(a);
            arg += 1;
            set(list...);
        }
        void set(){}
        Invoke          func;
        Any        *    arg;
    };
public:
    struct Wifi{
        WifiState   state;
        ListAp      list;
        IpdQue      packet[MAX_PACKET_CHANNEL];
        bool        scanFinished;
        void whenReceivePacket(int32_t id);
        void whenScanFinished(ListAp & list);
        void whenScanFailed(int32_t failCode);
        void whenStateChanged(WifiState state);
        void reset(){
            state = WifiDisconnect;
            scanFinished = false;
            list.clear();

            for (int i = 0; i < MAX_PACKET_CHANNEL; i++){
                while(packet[i].empty() == false){
                    packet[i].pop();
                }
            }
        }
    } wifi;

    struct Smart{
        void whenRising(bool isSuccess);
    }smart;

    struct Ping{
        void whenRising(bool isTimeOut, uint32_t latency);
    } ping;

    // struct Analysis{
    //     Analysis(){}
    //     void reset(){
    //         while(size()){
    //             pop();
    //         }
    //     }
    //     void push(AnalysisInvoke const & item){
    //         Rtos::queueSend(handle, item);
    //     }
    //     uint32_t size(){
    //         uint32_t size;
    //         Rtos::queueSize(handle, & size);
    //         return size;
    //     }
    //     void pop(){
    //         AnalysisInvoke dummy;
    //         Rtos::queueReceive(handle, & dummy);
    //         dummy.relase();
    //     }
    //     void invoke(Text & resp){
    //         AnalysisInvoke func;
    //         Rtos::queuePeek(handle, & func);
    //         func.invoke(resp);
    //     }
    //     static constexpr int itemSize = sizeof(AnalysisInvoke);
    // private:
    //     void * handle;
    // } analysis;

    AnalysisInvoke analysis;

    void reset();
    void eventHandler();
    void whenReset();
private:
    template<class ... args>
    Text readUntil(char a, args ... list){
        char group[] = { a, list..., '\0' };
        return readUntil(group);
    }
    Text readUntil(char * token);
    int  read();
    int  available();
    void begin();
    void write(Text value);
    volatile Result flag;
    volatile int 
           semaWaitFlag;
    void * semaWaitRx;
    void * rxQueue;
    void * taskFore;
    void * taskBack;
    friend struct Background;

    #define TEMP_RX                                         \
    size_t len = strlen(an.token);                          \
    if (strncmp(resp.c_str(), an.token, len)){              \
        return;                                             \
    }                                                       \
    Text v = readUntil('\n');                               \
    rxMain(v, buf);                                         \

public:
    Result waitFlag(uint32_t ms = uint32_t(-1));
    void run();

    template<class ... args>
    void rx(std::function<void ()> const & callback, const char * token, args ... list){
        extern void rxMain(Text resp, Any * buf);
        analysis = AnalysisInvoke{
            [this](AnalysisInvoke & an, Text & resp, Any * buf){
                TEMP_RX;
                an.callback();
            }, token, callback, list..., nullptr // nullptr as end of args
        };
    }

    template<class ... args>
    void rx(const char * token, args ... list){
        rx([](){}, token, list...);
    }

    template<class ... args>
    void tx(const char * token, args ... list){
        extern void txMain(Text * resp, Any * buf);
        Text resp = token;
        Any * arg = newAny(list..., nullptr);
        txMain(& resp, arg);
        write(resp);
        delete [] arg;
    }
};

extern EspStateBar esp;

template<class ... args>
void rx(const char * token, args ... list){
    esp.rx(token, list...);
}

template<class ... args>
void rx(std::function<void ()> const & callback, const char * token, args ... list){
    esp.rx(callback, token, list...);
}

template<class ... args>
void tx(const char * token, args ... list){
    esp.tx(token, list...);
}

inline Result waitFlag(uint32_t ms = uint32_t(-1)){
    return esp.waitFlag(ms);
}

