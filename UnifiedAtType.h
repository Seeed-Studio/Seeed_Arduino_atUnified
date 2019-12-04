#pragma once
#ifndef ARDUINO
    namespace std{
        void __throw_bad_function_call();
        void __throw_bad_alloc();
        void __throw_length_error(char const*);
    }
#endif

#include<Arduino.h>

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

#include<ctype.h>
#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>


#define debug(...)      Serial.printf(__VA_ARGS__)
#define CMD(name,...)   bool name(__VA_ARGS__) {
#define $               return waitFlag(); }
#define END_LINE        "\r\n"
#define ANY_INI(r,t)        \
Any(r const & v){           \
    if (v == nullref){      \
        t = nullptr;        \
    }                       \
    else {                  \
        t = & tmp_ ## t;    \
        type = Type ## t;   \
        tmp_ ## t = v;      \
    }                       \
}                           \
Any(r * v){                 \
    if (v == nullref){      \
        t = nullptr;        \
    }                       \
    else {                  \
        type = Type ## t;   \
        t = (decltype(t))v; \
    }                       \
}

struct nullref_t{
    template<class type>
    operator type &() const{
        return *(type *)nullptr;
    }

    template<class type>
    friend bool operator == (type const & a, nullref_t b) {
        return & a == nullptr;
    }
    
    template<class type>
    friend bool operator == (nullref_t b, type const & a) {
        return & a == nullptr;
    }
    
    template<class type>
    friend bool operator != (type const & a, nullref_t b) {
        return & a != nullptr;
    }
    
    template<class type>
    friend bool operator != (nullref_t b, type const & a) {
        return & a != nullptr;
    }
};

template<class type>
struct nullable{
    nullable() : 
        isNull(true){
    }
    nullable(type v) : 
        isNull(false), value(v){
    }
    void operator= (type v){
        isNull = false;
        value = v;
    }
    operator type & (){
        return value;
    }
    operator type * (){
        if (isNull){
            return nullptr;
        }
        return & value;
    }
    type * operator & (){
        return this[0];
    }
    type value;
    bool isNull;
};

constexpr bool disable      = false;
constexpr bool enable       = true;
constexpr nullref_t nullref = {};

enum Result{
    fail,
    success,
    timeout,
};

enum TypeAny{
    Typenull, Typei08, Typei32, Typex32, Typestr, Typeip, Typemac, Typetime,
};

enum DayOfWeek{
    Sun, Mon, Tue, Wen, Thu, Fri, Sat, NotDayOfWeek
};

enum Month{
    Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec, NotMonth
};

struct DateTime{
    int16_t year;
    int8_t  month;
    int8_t  day;
    int8_t  hour;
    int8_t  minute;
    int8_t  second;
    int8_t  dayOfWeek;
};

struct X32 { int32_t x; X32(int32_t x = 0) :x(x) {} };

template<uint32_t length>
struct NetCode{
    NetCode(){ isNull = true; }

    template<class ... args>
    NetCode(args ... list) {
        auto items = { list... };
        auto i = 0;
        for (auto v : items) {
            value[i] = uint8_t(v);
            i += 1;
        }
        isNull = false;
    }
    operator uint8_t * (){
        if (isNull){
            return nullptr;
        }
        return value;
    }
    bool    isNull;
private:
    uint8_t value[length];
};

typedef NetCode<4> Ipv4;
typedef NetCode<6> Mac;
typedef nullable<int8_t>    ni8;
typedef nullable<int32_t>   ni32;
typedef nullable<bool>      nbool;
typedef nullable<uint8_t>   nu8;
typedef nullable<uint32_t>  nu32;


struct Text{
    Text(){}
    Text(char value) {
        char tmp [] = { value, '\0' };
        this[0] = Text(tmp);
    }
    Text(const char * value) {
        auto p = new char[strlen(value) + 1];
        data = std::shared_ptr<char>(p);
        strcpy(data.get(), value);
    }
    const char * c_str(){
        return data.get();
    }
    void operator=(const char * value){
        data = Text(value).data;
    }
    void operator=(decltype(nullptr) value){
        data = Text().data;
    }
    bool operator==(decltype(nullptr) value){
        return data == nullptr;
    }
    bool operator!=(decltype(nullptr) value){
        return data != nullptr;
    }
    Text operator + (Text value){
        size_t len = length() + value.length();
        Text tmp;
        auto p = new char[len];
        tmp.data = std::shared_ptr<char>(p);

        if (data != nullptr){
            strcpy(p, data.get());
            p += length();
        }
        if (value != nullptr){
            strcpy(p, value.data.get());
        }
        return tmp;
    }

    template<class type>
    Text operator += (type value){
        this[0] = this[0] + Text(value);
        return this[0];
    }
    bool operator == (Text value){
        if (value.data == nullptr){
            return data == nullptr;
        }
        if (data == nullptr){
            return value.data == nullptr;
        }
        return strcmp(value.data.get(), data.get()) == 0;
    }
    bool operator != (Text value){
        return !(this[0] == value);
    }
    size_t length(){
        if (data == nullptr){
            return 0;
        }
        return strlen(data.get());
    }
    bool startsWith(const char * str){
        if (data == nullptr){
            return false;
        }
        return strncmp(data.get(), str, strlen(str)) == 0;
    }
    char & operator [](size_t index){
        return data.get()[index];
    }
private:
    std::shared_ptr<char> data;
};

struct Any{
    Any(){
        str = nullptr;
        type = Typenull;
    }
    Any(decltype(nullptr)) {
        str = nullptr;
        type = Typenull;
    }

    template<class type>
    Any(nullable<type> * value) : 
        Any((type *)value[0]){
    }
    
    template<class type>
    Any(nullable<type> & value) : 
        Any((type *)value){
    }

    ANY_INI(Text,       str);
    ANY_INI(X32,        x32);
    ANY_INI(int32_t,    i32);
    ANY_INI(int8_t,     i08);
    ANY_INI(bool,       i08);
    ANY_INI(Ipv4,       ip);
    ANY_INI(Mac,        mac);
    ANY_INI(DateTime,   time);

    bool isEmpty() {
        return Typenull == type;
    }
    union{
        Text  *   str;
        X32     *   x32;
        int32_t *   i32;
        int8_t  *   i08;
        Ipv4    *   ip;
        Mac     *   mac;
        DateTime*   time;
    };
    TypeAny         type;
private:
    Text            tmp_str;
    union{
        X32         tmp_x32;
        int32_t     tmp_i32;
        int8_t      tmp_i08;
        Ipv4        tmp_ip;
        Mac         tmp_mac;
        DateTime    tmp_time;
    };
};

#undef ANY_INI

