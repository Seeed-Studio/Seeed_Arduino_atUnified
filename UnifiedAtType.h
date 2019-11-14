#pragma once
#ifndef ARDUINO
    #include<string>
    typedef std::string String;
#else
    #include<Arduino.h>
    #include<WString.h>
#endif

#ifdef max
    #pragma push(max)
    #pragma push(min)
    #undef max
    #undef min
    #include<functional>
    #pragma pop(min)
    #pragma pop(max)
#else
    #include<functional>
#endif

#include<stdio.h>
#include<string.h>
#include<stdint.h>

#define AT              "AT"
#define END_LINE        "\r\n"
#define TOKEN           ":"
#define AT_ERROR        "ERROR"
#define AT_OK           "OK"
#define AT_FAIL         "FAIL"
#define AT_READY        "ready"
#define SET             "="
#define ASK             "?"
#define CMD(name,...)                       \
inline bool name(__VA_ARGS__) {             \
    extern bool atEcho(bool);               \
    extern bool waitFlag();                 \
    if (atEcho(false) == fail) return fail;
#define $                return waitFlag(); }
#define debug(...)       Serial.printf(__VA_ARGS__);

bool atTest();
bool atBegin();

enum class actionMode{
    waitReady,
    async,
};

template<class a, class b>
void copy(a * des, b const * src, size_t length){
    while (length--) {
        des[length] = (a)src[length];
    }
}

struct any{
    template<class type>
    any(const type * v) : data((type *)v){}

    template<class type>
    any(type const & v) : data(& (type &)v){}

    template<class type>
    void set(type const & value){
        *(type *)data = value;
    }
    template<class type>
    void set(size_t i, type const & value){
        ((type *)data)[i] = value;
    }

    template<class type>
    type get(){
        return *(type *)data;
    }

    template<class type>
    type * ptr(){
        return (type *)data;
    }
private:
    void * data;
};

template<uint32_t length>
struct NetCode{
    uint8_t * operator &(){
        return code;
    }
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

struct nullref_t{
    template<class type>
    operator type &() const{
        return *(type *)nullptr;
    }
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

constexpr bool disable      = false;
constexpr bool enable       = true;
constexpr bool fail         = false;
constexpr bool success      = true;
constexpr int32_t leaveOut  = int32_t(0x80000000);
constexpr nullref_t nullref = {};

typedef NetCode<4> ipv4;
typedef NetCode<6> mac;

template<class ... arg>
bool rx(const char * fmt, arg & ... list){
    bool rxMain(const char * fmt, any * list);
    any   ls[] = { list... };
    return rxMain(fmt, ls);
}

template<class ... arg>
bool tx(const char * fmt, arg ... list){
    void txMain(const char * fmt, any * list);
    any     ls[] = { list... };
    txMain(fmt, ls);
    return true;
}

bool tx(const char * cmd);

