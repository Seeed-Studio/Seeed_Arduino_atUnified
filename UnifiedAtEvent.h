#pragma once
#include"UnifiedAtType.h"
#include"UnifiedAtWifi.h"
#include"UnifiedAtTcpIp.h"
#include"UnifiedBackTask.h"
#define MAX_PACKET_CHANNEL      5
#define MAX_AP_LIST             86
#define MAX_STA_LIST            32

enum WifiState : uint8_t{
    WifiDisconnect,
    WifiConnected,
    WifiGotIp,
    WifiConnectFail,
};

enum Wait : uint8_t{
    WaitNothing,
    WaitWifiScan,
};

struct Ipd{
    int32_t         i;
    int32_t         length;
    int32_t         port;
    Ipv4            ip;
    std::shared_ptr<uint8_t> 
                    data;
};

typedef std::queue<Ipd>         IpdQue;

struct EspStateBar{
private:
    typedef std::function<void (Text &, Any *)> Invoke;


    struct AnalysisInvoke{
        void invoke(Text & resp){
            if (func){
                func(resp, arg);
            }
        }

        template<class first, class ... args>
        void setArg(first const & a, args const & ... list){
            set(arg, a, list...);
        }
        Event           whenResolutionOneLine;

    private:
        friend EspStateBar;
        uint32_t     i;
        const char*  token[16];
        Invoke       func;
        Any *        arg;
        Any          argList[16];

        template<class first, class ... args>
        static void set(Any * any, first const & a, args const & ... list){
            new(any) Any(a);
            set(any + 1, list...);
        }
        static void set(Any * any){}
    };


public:
    struct Wifi{
        typedef TransmissionLink TranLink;
        WifiState   state;
        WifiApItem  apList[MAX_AP_LIST];
        WifiApItem* apListEnd;
        WifiUser    apUser[MAX_STA_LIST];
        WifiUser  * apUserEnd;
        TranLink    tranLink[MAX_PACKET_CHANNEL];
        TranLink  * tranLinkEnd;
        IpdQue      packet[MAX_PACKET_CHANNEL];
        uint8_t     packId;
        Event       whenReceivePacket;
        Event       whenScanFinished;
        Event       whenScanFailed;
        Event       whenStateChanged;
        Wifi(){
            whenReceivePacket = EspStateBar::doNothing;
            whenScanFinished = EspStateBar::doNothing;
            whenScanFailed = EspStateBar::doNothing;
            whenStateChanged = EspStateBar::doNothing;
        }
        void reset(){
            state = WifiDisconnect;

            for (int i = 0; i < MAX_PACKET_CHANNEL; i++){
                while(packet[i].empty() == false){
                    packet[i].pop();
                }
            }
        }
    } wifi;

    struct Smart{
        bool        state;
        Event       whenRising;
        Smart(){
            whenRising = EspStateBar::doNothing;
        }
    } smart;

    struct Ping{
        bool        isTimeout;
        uint32_t    latency;
        Event       whenRising;
        Ping(){
            whenRising = EspStateBar::doNothing;
        }
    } ping;

    AnalysisInvoke  analysis;
    Event           whenReset;
    volatile Wait   wait;

    EspStateBar(){
        wait = WaitNothing;
        whenReset = EspStateBar::doNothing;
    }
    void reset();
    void eventHandler();
    void run();
    Result waitFlag(uint32_t ms = uint32_t(-1));

    static void doNothing(){}

    template<class ... args>
    void rx(std::function<void (Text &, Any *)> const & token, args const & ... list){
        analysis.func = token;
        analysis.set(list..., nullptr);
        analysis.arg += sizeof...(args);
    }

    template<class ... args>
    void rx(const char * token, args const & ... list){
        analysis.token[analysis.i++] = token;

        analysis.func = [this](Text & resp, Any * arg){
            auto token = analysis.token[analysis.i++];
            auto arg_skip = 0;

            if (resp.startsWith(token)){
                auto && v = readUntil("\n");
                arg_skip = rxMain(v, analysis.arg);
                analysis.arg += arg_skip;
                analysis.whenResolutionOneLine();
            }
        };

        analysis.setArg(list..., nullptr);
        analysis.arg += sizeof...(args);
    }

    void resetArg() {
        analysis.i = 0;
        analysis.arg = analysis.argList;
    }

    void flush(){
        analysis.token[analysis.i] = nullptr;
        resetArg();
        // debug(">%s", cmd.c_str());
        write(cmd);
    }

    template<class ... args>
    void tx(const char * token, args const & ... list){
        Rtos::take(semaWaitCmd);
        resetArg();
        analysis.whenResolutionOneLine = EspStateBar::doNothing;
        analysis.setArg(list..., nullptr);
        flag = Timeout;
        cmd = token;
        txMain(& cmd, analysis.arg);
    }

    void txBin(uint8_t const * buffer, size_t length);
private:
    Text readUntil(char * token);

    /* Com operations */
    void begin();
    int  available();
    int  read();
    void write(Text value);

    volatile Result flag;
    Text             cmd;
    void *  semaWaitCmd;
    void *  taskFore;
    void *  taskBack;
};

extern EspStateBar esp;

template<class ... args>
void rx(const char * token, args const & ... list){
    esp.rx(token, list...);
}

template<class ... args>
void rx(std::function<void (Text &, Any *)> const & token){
    esp.rx(token);
}

inline void flush(){
    esp.flush();
}

template<class ... args>
void tx(const char * token, args const & ... list){
    esp.tx(token, list...);
}

inline Result waitFlag(uint32_t ms = uint32_t(-1)){
    return esp.waitFlag(ms);
}

inline void txBin(uint8_t const * buffer, size_t length){
    esp.txBin(buffer, length);
}

