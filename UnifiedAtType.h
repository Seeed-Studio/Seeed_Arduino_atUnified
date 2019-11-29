#pragma once
#ifndef ARDUINO
    #include<Arduino.h>
    #include<string>
    typedef std::string String;
#else
    #include<WString.h>
#endif

#ifdef max
    #pragma push(max)
    #pragma push(min)
    #undef max
    #undef min
    #include<functional>
    #include<vector>
    #include<queue>
    #include<memory>
    #pragma pop(min)
    #pragma pop(max)
#else
    #include<functional>
    #include<vector>
    #include<queue>
    #include<memory>
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
bool name(__VA_ARGS__) {

#define $                return waitFlag(); }
#define debug(...)       Serial.printf(__VA_ARGS__);

bool atTest();
bool atBegin();

template<class a, class b>
void copy(a * des, b const * src, size_t length){
    while (length--) {
        des[length] = (a)src[length];
    }
}

struct any{
    template<class type>
    any(type * v) : data(v){} // not [type const *]

    template<class type>
    any(type const & v) : data(& (type &)v){}

    template<class type>
    void set(type const & value){
        debug("\nset:%p\n", data);
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


struct nullref_t{
    template<class type>
    operator type &() const{
        return *(type *)nullptr;
    }
};


constexpr bool disable      = false;
constexpr bool enable       = true;
constexpr uint8_t fail      = 0;
constexpr uint8_t success   = 1;
constexpr uint8_t match     = 2;
constexpr int32_t leaveOut  = int32_t(0x80000000);
constexpr nullref_t nullref = {};


bool ta(const char * cmd);
bool tb(uint8_t const * buffer, size_t length);
bool tx(const char * cmd);

template<class ... arg>
bool rx(const char * fmt, arg & ... list){
    bool rxMain(const char * fmt, any * list);
    any   ls[] = { list... };
    return rxMain(fmt, ls);
}

template<class ... arg>
bool ta(const char * fmt, arg const & ... list){
    void txMain(const char * fmt, any * list);
    any     ls[] = { list... };
    txMain(fmt, ls);
    return true;
}

template<class ... arg>
bool tx(const char * fmt, arg const & ... list){
    void txMain(const char * fmt, any * list);
    any     ls[] = { list... };
    txMain(fmt, ls);
    tx("");
    return true;
}


uint8_t waitFlag(const char * externFlag = nullptr);

