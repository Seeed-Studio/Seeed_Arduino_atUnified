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
    extern bool isNotWakeup();              \
    extern bool waitFlag();                 \
    if (isNotWakeup()) return fail;         \
    if (atEcho(false) == fail) return fail;
#define $                return waitFlag(); }
#define debug(...)       Serial.printf(__VA_ARGS__);

constexpr bool disable = false;
constexpr bool enable  = true;
constexpr bool fail    = false;
constexpr bool success = true;

bool atTest();
bool atBegin();

enum any_type{
    none,
    digital,
    hex,
    string,
    ip,
};

enum class actionMode{
    waitReady,
    async,
};

enum hex_t : int32_t {};
enum mac_t : uint8_t {};
enum ip_t : uint8_t {};

enum class txMore{ };
constexpr txMore more = txMore(0);

struct any{
    any(ip_t * v)    : v(v), type(ip),      skip(0){}
    any(hex_t * v)   : v(v), type(hex),     skip(0){}
    any(int32_t * v) : v(v), type(digital), skip(0){}
    any(String * v)  : v(v), type(string),  skip(0){}
    any(const char * v) :    type(none),    skip(strlen(v)){}
    any(char v) :            type(none),    skip(1){}
    template<class type> 
    void set(type const & value){
        *(type *)v = value;
    }
    void *   v;
    uint8_t  skip;
    any_type type;
};

template<class ... arg>
bool rx(const char * prefix, arg ... list){
    extern bool subrx(const char * prefix, any * list);
    any     ls[] = { list... };
    return subrx(prefix, ls);
}

inline void tx(txMore){}
void tx();

template<class first, class ... arg>
void tx(first a, arg ... list){
    extern void write(int32_t value);
    extern void write(String const & value);
    extern void write(ip_t * ip);
    extern void write(mac_t one);
    extern void write(mac_t * mac);
    extern volatile bool idEndLine;

    write(a);
    if (idEndLine){
        idEndLine = false;
    }
    else if (sizeof...(list)){
        write(",");
    }
    tx(list...);
}

template<class a, class b>
void copy(a * des, b const * src, size_t length){
    while (length--) {
        des[length] = (a)src[length];
    }
}

