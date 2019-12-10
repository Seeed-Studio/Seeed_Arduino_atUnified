#pragma once
#include<Arduino.h>
#include<String.h>

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

// FOR ESP-LIB
#define log_e(...)
#define log_i(...)
#define log_d(...)
#define log_w(...)

// FOR SELF-DEBUG
#define debug(...)      if (Serial) { Serial.printf(__VA_ARGS__); }
#define CMD(name,...)   bool name(__VA_ARGS__) {
#define $               flush(); return waitFlag(); }
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
Any(r const * v){           \
    if (v == nullptr){      \
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

// this array is meet for hold static memory pointer.
template<class type>
struct Array{
    Array(){
        len = 0;
        ptr = nullptr;
    }
    Array(type * ptr, size_t len) : ptr(ptr), len(len){}
    type & operator [](size_t index){
        return ptr[index];
    }
    size_t size(){
        return len;
    }
private:
    size_t len;
    type * ptr;
};

constexpr bool disable      = false;
constexpr bool enable       = true;
constexpr nullref_t nullref = {};

enum Result{
    Fail,
    Success,
    Timeout,
};

enum TypeAny{
    Typenull, Typei08, Typei32, Typex32, Typestr, Typeip, Typemac, Typetime, Typeskip,
};

enum DayOfWeek{
    Sun, Mon, Tue, Wen, Thu, Fri, Sat, NotDayOfWeek
};

enum Month{
    NotMonth, Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec
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
    NetCode(const uint8_t * v){
        for (size_t i = 0; i < length; i++){
            value[i] = v[i];
        }
        isNull = false;
    }

    template<class ... args>
    NetCode(uint8_t first, args ... list) {
        uint8_t items[] = { first, list... };
        this[0] = NetCode(items);
    }
    operator uint8_t * (){
        if (isNull){
            return nullptr;
        }
        return value;
    }
    explicit operator char * (){
        return (char *)(uint8_t *)this[0];
    }
    auto * operator &(){
        isNull = false;
        return this;
    }
    bool    isNull;
private:
    uint8_t value[length];
};

template<class type>
struct Nullable{
    Nullable() : 
        isNull(true){
    }
    Nullable(type v) : 
        isNull(false), value(v){
    }
    void operator= (type v) {
        isNull = false;
        value = v;
    }
    operator type & () const {
        return value;
    }
    operator type * () const {
        if (isNull){
            return nullptr;
        }
        return & value;
    }
    type * operator & () const {
        isNull = false;
        return this[0];
    }
    mutable type value;
    mutable bool isNull;
};

typedef NetCode<4>              Ipv4;
typedef NetCode<6>              Mac;
typedef Nullable<int8_t>        Ni8;
typedef Nullable<int32_t>       Ni32;
typedef Nullable<bool>          nbool;
typedef Nullable<uint8_t>       nu8;
typedef Nullable<uint32_t>      nu32;
typedef std::function<void()>   Event;

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
    const char * c_str() const {
        return data.get();
    }
    operator String(){
        return String(c_str());
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
    Text sub(size_t index){
        if (index >= length()){
            return Text();
        }
        return Text(data.get() + index);
    }
    Text operator + (Text value){
        size_t lenA = length();
        size_t lenB = value.length();
        size_t len = lenA + lenB + 1; // 1 for '\0'
        Text tmp;
        auto p = new char[len];
        tmp.data = std::shared_ptr<char>(p);

        if (data != nullptr){
            strcpy(p, data.get());
            p += lenA;
        }
        if (value != nullptr){
            strcpy(p, value.data.get());
            p += lenB;
        }
        p[0] = '\0';
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
    bool startsWith(Text const & str){
        return startsWith(str.c_str());
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

    Any(const char * skip) : 
        skip(skip), type(Typeskip){
    }

    template<class type>
    Any(Nullable<type> const & v) : 
        Any((const type *)v){
    }

    template<class type>
    Any(Nullable<type> const * v) : 
        Any(& v->value){
    }

    ANY_INI(Text,       str);
    ANY_INI(X32,        x32);
    ANY_INI(int32_t,    i32);
    ANY_INI(int8_t,     i08);
    ANY_INI(bool,       i08);
    ANY_INI(Ipv4,       ip);
    ANY_INI(Mac,        mac);
    ANY_INI(DateTime,   time);
    #undef ANY_INI

    bool isEmpty() {
        return Typenull == type || str == nullptr;
    }
    union{
        Text        *   str;
        X32         *   x32;
        int32_t     *   i32;
        int8_t      *   i08;
        Ipv4        *   ip;
        Mac         *   mac;
        DateTime    *   time;
        const char  *   skip;
    };
    TypeAny             type;
private:
    Text                tmp_str;
    union{
        X32             tmp_x32;
        int32_t         tmp_i32;
        int8_t          tmp_i08;
        Ipv4            tmp_ip;
        Mac             tmp_mac;
        DateTime        tmp_time;
    };
};

template<class a, class b>
void copy(a * des, const b * src, size_t length){
    for (size_t i = 0; i < length; i++){
        des[i] = src[i];
    }
}

void   txMain(Text * resp, Any * buf);
size_t rxMain(Text resp, Any * buf);

