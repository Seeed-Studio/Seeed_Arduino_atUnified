#include"UnifiedAtType.h"
#define EOL             ((char *)"")

#ifdef USE_AT_SERIAL
    #define USE_SERIAL_COM
    auto & com = Serial;
#elif defined USE_AT_SERIAL2
    #define USE_SERIAL_COM
    auto & com = Serial2;
#else
    #define USE_SERIAL_COM
    auto & com = Serial1;
#endif

volatile bool needWaitWeekup = false;
volatile bool isEndLine = true;
String peekValue = "";

#ifdef USE_SERIAL_COM
    int available(){
        return com.available();
    }
    int read(){
        return com.read();
    }
    String peekPrefix(){
        char buf[48] = { 0 };
        if (peekValue.length() == 0){
            for (size_t i = 0; i < sizeof(buf); i++) {
                buf[i] = com.peek();
                if (buf[i] == '\r'){
                    break;
                }

                com.read();

                if (buf[i] == ':'){
                    break;
                }
            }
            peekValue = buf;
        }
        return peekValue;
    }
    void clearPrefix(){
        peekValue = "";
    }
    String readLine(){
        while(available() == 0);
        String line = peekValue + com.readStringUntil('\n');
        if (line.length()){
            line = line.substring(0, line.length() - 1);
        }
        return line;
    }
    void write(char value){
        com.print(value);
        debug("%c", value);
    }
    void write(const char * value){
        com.print(value);
        debug(value);
    }
    bool atBegin(){
        com.begin(115200);
        while(available());
        tx("ATE0");
        return waitFlag();
    }
#endif

uint8_t waitFlag(const char * externFlag){
    while(true){
        String ack = readLine();
        if (externFlag && ack.indexOf(externFlag) >= 0){
            return match;
        }
        if (ack.indexOf(AT_OK) >= 0){
            return success;
        }
        if (ack.indexOf(AT_ERROR) >= 0 || ack.indexOf(AT_FAIL) >= 0){
            return fail;
        }
    }
}
void waitReady(){
    while(readLine() != AT_READY){
        ;
    }
}
bool isNotWakeup(){
    if (needWaitWeekup == false){
        return false;
    }
    if (available()){
        waitReady();
        needWaitWeekup = false;
        return false;
    }
    return true;
}

String readLine();

bool parseInt(char ** p, char type, int32_t * v){
    typedef int(* match_t)(int);
    auto end = p[0];
    auto match = type == 'x' ? match_t(& isxdigit): match_t(& isdigit);
    char fmt[] = { '%', type, '\0' };
    if (end[0] == '-' || end[0] == '+'){
        end++;
    }
    while(match(end[0])){
        end += 1;
    }
    if (sscanf(p[0], fmt, v) == 0){
        return fail;
    }
    p[0] = end;
    return success;
};

//parse mac/ip
//Mac format
//12:34:56:78:9a:bc
//ip format
//192.168.1.1
bool parseNetCode(any * list, char ** p, char type, size_t length){

    // NOTICE:
    // some at response line use the "12:34:56:78:9a:bc" format, 
    // but some use 12:34:56:78:9a:bc
    // shit design!!!
    if (p[0][0] == '\"'){
        p[0] += 1;
    }
    for (int32_t i = 0, v; i < length; i++){
        if (parseInt(p, type, & v) == fail){
            return fail;
        }

        list->set<uint8_t>(i, v);

        if (p[0][0] != '\0'){
            p[0] += 1;  //skip ':' or '.' or '\"'(the end) or 
                        //','(when value without double quotes)
        }
    }

    // when last character is '\"'
    // p[0][0] maybe ',' or '\0'
    if (p[0][0] == ','){
        p[0] += 1;
    }
};

bool parseDateTime(any * list, char ** p){
    // Format:
    // Mon Dec 12 02:33:32 2016
    char * str = p[0];
    auto * time = list->ptr<DateTime>();
    auto dayOfWeek = 
    !strncmp(str, "Sun", 3) ? Sun :
    !strncmp(str, "Mon", 3) ? Mon :
    !strncmp(str, "Tue", 3) ? Tue :
    !strncmp(str, "Wen", 3) ? Wen :
    !strncmp(str, "Thu", 3) ? Thu :
    !strncmp(str, "Fri", 3) ? Fri :
    !strncmp(str, "Sat", 3) ? Sat : NotDayOfWeek;

    if (dayOfWeek == NotDayOfWeek){
        return fail;
    }

    str += 4;

    auto month = 
    !strncmp(str, "Jan", 3) ? Jan :
    !strncmp(str, "Feb", 3) ? Feb :
    !strncmp(str, "Mar", 3) ? Mar :
    !strncmp(str, "Apr", 3) ? Apr :
    !strncmp(str, "May", 3) ? May :
    !strncmp(str, "Jun", 3) ? Jun :
    !strncmp(str, "Jul", 3) ? Jul :
    !strncmp(str, "Aug", 3) ? Aug :
    !strncmp(str, "Sep", 3) ? Sep :
    !strncmp(str, "Oct", 3) ? Oct :
    !strncmp(str, "Nov", 3) ? Nov :
    !strncmp(str, "Dec", 3) ? Dec : NotMonth;

    if (month == NotMonth){
        return fail;
    }

    str += 4;
    time->dayOfWeek = dayOfWeek;
    time->month     = month;
    parseInt(& str, 'd', & time->day   ); str += 1; // skip ' '
    parseInt(& str, 'd', & time->hour  ); str += 1; // skip ':'
    parseInt(& str, 'd', & time->minute); str += 1; // skip ':'
    parseInt(& str, 'd', & time->second); str += 1; // skip ' '
    parseInt(& str, 'd', & time->year  );
    p[0] = str;
    return success;
}

#define assert(func,...)            \
if (func(__VA_ARGS__) == fail) {    \
    return fail;                    \
}


bool rxMain(const char * fmt, any * list){
    char *        p = (char *)readLine().c_str();
    char          buf[32];
    union{
        int32_t   i32;
        int8_t    i8;
    };
    //char line[] = "+AT:\"hello?\",12,-129,\"nico\",\"12:34:56:78:9a:bc\",\"192.168.1.1\"";
    //char * p = line;
    debug("%s\n", p);
    
    while(fmt[0]){
        if (p[0] == '\0'){
            if (fmt[0] == '\n'){
                fmt += 1;
                p = (char *)readLine().c_str();
            }
            else{
                return success;
            }
        }
        if (fmt[0] == '%'){
            fmt += 1;
        }
        else {
            if (fmt[0] == p[0]){
                fmt += 1;
                p += 1;
                continue;
            }
            else{
                return fail;
            }
        }

        debug("\nC:%c%c", fmt[0], p[0]);

        switch(fmt[0]){
        case 's': {
                auto end = (char *)strchr(p + 1, '\"');
                end[0] = '\0';
                list->set<String>(String(p + 1));
                p = end + 1;
                break;
            }
        case 'd':{
                assert(parseInt, & p, 'd', & i32);
                debug("\nget:%d\n", i32);
                list->set<int32_t>(i32);
                break;
            }
        case 'b':{
                assert(parseInt, & p, 'd', & i32);
                list->set<int8_t>(i8);
                break;   
            }
        case 'x':{
                assert(parseInt, & p, 'x', & i32);
                list->set<int32_t>(i32);
                break;
            }
        case 'm': {
                assert(parseNetCode, list, & p, 'x', 6);
                break;
            }
        case 'i':{
                assert(parseNetCode, list, & p, 'd', 4);
                break;
            }
        case 't':{
                assert(parseDateTime, list, & p);
                break;
            }
        case '%':{
                if (p[0] != '%'){
                    p = EOL;
                    return fail;
                }
                p += 1;
                continue;
            }
        }

        // DON'T DO THIS 
        // for(;; list += 1)
        list += 1;
    }
    return success;
}


#define WRITE_NUM(type,fmt)     \
auto v = list->get<type>();     \
if (mayHidden){                 \
    if (v == leaveOut){         \
        continue;               \
    }                           \
}                               \
sprintf(buf, fmt, v);           \
SPLIT_CHAR()                    \
write(buf);                     \
break;

#define SPLIT_CHAR()    if (mayHidden){ write(","); }

void txMain(const char * fmt, any * list){
    char buf[32];
    
    for(; fmt[0]; fmt++){
        if (fmt[0] != '%'){
            write(fmt[0]);
            continue;
        }
        
        bool mayHidden = fmt[1] == '+';
        fmt += mayHidden ? 2 : 1;

        switch(fmt[0]){
        case 's': {
                if (list->get<String>() == nullptr){
                    return;
                }
                SPLIT_CHAR();
                write('\"');
                write(list->get<String>().c_str());
                write('\"');
                break;
            }
        case 'b':{
                WRITE_NUM(int8_t, "%d");
            }
        case 'd':{
                WRITE_NUM(int32_t, "%d");
            }
        case 'x':{
                WRITE_NUM(int32_t, "%x");
            }
        case 'm': {
                if (list->ptr<uint8_t>() == nullptr ||
                    list->get<mac>().isEmpty()){
                    return;
                }
                auto Mac = list->ptr<uint8_t>();
                sprintf(buf, "\"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\"",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
                );
                SPLIT_CHAR();
                write(buf);
                break;
            }
        case 'i':{
                if (list->ptr<uint8_t>() == nullptr ||
                    list->get<Ipv4>().isEmpty()){
                    return;
                }
                auto ip = list->ptr<uint8_t>();
                sprintf(buf, "\"%d.%d.%d.%d\"",
                    ip[0], ip[1], ip[2], ip[3]
                );
                SPLIT_CHAR();
                write(buf);
                break;
            }
        case '%':{
                write('%');
                continue; // skip list += 1;
            }
        }

        // DON'T DO THIS 
        // for(;; list += 1)
        list += 1;
    }
}

bool tx(const char * cmd){
    write(cmd);
    write(END_LINE);
    return true;
}
bool ta(const char * cmd){
    write(cmd);
    return true;
}
bool tb(uint8_t const * buffer, size_t length){
    for (size_t i = 0; i < length; i++){
        write(buffer[i]);
    }
    return true;
}

CMD(atTest)
    tx("AT");
$
