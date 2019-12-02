#pragma once
#include"UnifiedAtType.h"
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


typedef std::vector<int> ListAp;
//typedef std::vector<WifiApItem> ListAp;
typedef std::queue<Ipd> IpdQue;

struct EspStateBar{
private:
    struct AnalysisInvoke{
        template<class ... args>
        AnalysisInvoke(std::function<void (String &, Any *)> && func, args ... list) : 
            func(func){
                Any tmp[] = { list... };
                for (int i = 0; i < sizeof...(args); i++){
                    arg[i] = tmp[i];
                }
            }
        void invoke(String & resp){
            func(resp, arg);
        }
    private:
        std::function<void (String &, Any *)>  func;
        Any                                    arg[10];
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
            if (que.size()) {
                que.front().invoke(resp);
            }
        }
    private:
        std::queue<AnalysisInvoke> que;
    } analysis;

    void reset(){
        wifi.reset();
        analysis.reset();
    }
    void eventHandler();
    void whenReset();
private:    
    template<class ... args>
    String readUntil(char a, args ... list){
        char group[] = { a, list..., '\0' };
        return readUntil(group);
    }
    String readUntil(char * token);
    int read();
    int available();
    void writeLine(String value);

    bool   flag;
    void * semaWaitFlag;
    void * semaWaitRx;
    friend struct Background;
public:
    template<class ... args>
    void rx(const char * token, args ... list){
        void rxMain(String & resp, Any * buf);
        analysis.push(AnalysisInvoke{
            [&](String & resp, Any * buf){
                if (resp.startsWith(token) == false){
                    return;
                }
                auto && line = readUntil('\n');
                rxMain(line, buf);
                analysis.pop();
            }, list..., nullptr // nullptr as end of args
        });
    }

    bool waitFlag();

    template<class ... args>
    void tx(const char * token, args const & ... list){
        void txMain(String * resp, Any * buf);
        String resp = token;
        Any arg[] = { list..., nullptr }; // nullptr as the end of args.
        txMain(resp, arg);
        writeLine(resp);
    }

};

extern EspStateBar esp;
void espBegin();


