#ifndef ARDUINO
    #include<string>
    typedef std::string String;
    namespace std{
        void __throw_bad_function_call();
        void __throw_bad_alloc();
        void __throw_length_error(char const*);
    }
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

#include<ctype.h>
#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

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

constexpr bool disable      = false;
constexpr bool enable       = true;
constexpr uint8_t fail      = 0;
constexpr uint8_t success   = 1;
constexpr uint8_t match     = 2;
constexpr nullref_t nullref = {};


enum TypeAny{
    Typei08, Typei32, Typex32, Typestr, Typeip, Typemac, Typetime,
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

struct X32 { int32_t x; X32(int32_t x = 0) :x(x) {} };

template<uint32_t length>
struct NetCode{
    template<class ... args>
    NetCode(args ... list) {
        auto items = { list... };
        auto i = 0;
        for (auto v : items) {
            code[i] = uint8_t(v);
            i += 1;
        }
        
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
typedef NetCode<4> Ipv4;
typedef NetCode<6> Mac;

#define CMD(name,...)   bool name(__VA_ARGS__)
#define $               }
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

struct Any{
    Any(){}
    Any(decltype(nullptr)) {
        str = nullptr;
    }

    ANY_INI(String,     str);
    ANY_INI(X32,        x32);
    ANY_INI(int32_t,    i32);
    ANY_INI(int8_t,     i08);
    ANY_INI(bool,       i08);
    ANY_INI(Ipv4,       ip);
    ANY_INI(Mac,        mac);
    ANY_INI(DateTime,   time);

    bool isEmpty() {
        return str == nullptr;
    }
    union{
        String  *   str;
        X32     *   x32;
        int32_t *   i32;
        int8_t  *   i08;
        Ipv4    *   ip;
        Mac     *   mac;
        DateTime*   time;
    };
    TypeAny         type;
private:
    String          tmp_str; // put to union will result memory leak.
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

