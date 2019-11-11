#pragma once
#include<Arduino.h>
#include<WString.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>

#define AT              "AT"
#define END_LINE        "\r\n"
#define TOKEN           ":"
#define AT_ERROR        "ERROR"
#define AT_OK           "OK"
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

constexpr bool disable     = false;
constexpr bool enable      = true;
constexpr bool fail        = false;
constexpr bool success     = true;
constexpr int32_t leaveOut = int32_t(0x80000000);

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
    any(type const & v) : data((type *) & v){}

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

template<class ... arg>
bool rx(const char * fmt, arg ... list){
    bool rxMain(const char * fmt, any * list);
    any   ls[] = { list... };
    any * v = ls;
    return rxMain(fmt, v);
}

template<class ... arg>
void tx(const char * fmt, arg ... list){
    void txMain(const char * fmt, any * list);
    any     ls[] = { list... };
    any   * v = ls;
    txMain(fmt, v);
}

void tx(const char * cmd);

