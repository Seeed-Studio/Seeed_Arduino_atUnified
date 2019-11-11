#include"UnifiedAtType.h"
#define ATE             "ATE%d", enable

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

#ifdef USE_SERIAL_COM
    bool available(){
        return com.available() != 0;
    }
    String readLine(){
        String && line = com.readStringUntil('\n');
        if (line.length() == 0){
            return line;
        }
        line = line.substring(0, line.length() - 1);
        //debug(line.c_str());
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
        while(readLine().length());
        return atTest();
    }
#endif

bool waitFlag(){
    while(true){
        String ack = readLine();
        //debug("E:%s\n", ack.c_str());
        if (ack == AT_OK){
            return success;
        }
        if (ack == AT_ERROR){
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

int32_t parseInt(char ** p, char type){
    typedef int(* match_t)(int);
    auto end = p[0];
    auto match = type == 'x' ? match_t(& isxdigit): match_t(& isdigit);
    auto value = int32_t(0);
    char fmt[] = { '%', type, '\0' };
    if (end[0] == '-' || end[0] == '+'){
        end++;
    }
    while(match(end[0])){
        end += 1;
    }
    sscanf(p[0], fmt, & value);
    p[0] = end;
    return value;
};

//parse mac/ip
//mac format
//12:34:56:78:9a:bc
//ip format
//192.168.1.1
void parseNetCode(any * list, char ** p, char type, size_t length){
    for (size_t i = 0; i < length; i++){
        list->set<uint8_t>(i, parseInt(p, type));
        p[0] += 1; //skip ':' or '.'
    }
    if (p[0] == ','){
        p[0] += 1;
    }
};

bool rxMain(const char * fmt, any * list){
    char buf[32];
    auto p = (char *)readLine().c_str();
    //char line[] = "+AT:\"hello?\",12,-129,\"nico\",\"12:34:56:78:9a:bc\",\"192.168.1.1\"";
    //char * p = line;

    for(; fmt[0]; fmt++){
        if (fmt[0] == '\n' && p[0] == '\0'){
            fmt += 1;
            p = (char *)readLine().c_str();
        }
        if (fmt[0] != '%'){
            if (fmt[0] != p[0]){
                return fail;
            }
            p += 1;
            continue;
        }
        
        fmt += 1;

        switch(fmt[0]){
        case 's': {
                auto end = (char *)strchr(p + 1, '\"');
                end[0] = '\0';
                list->set<String>(String(p + 1));
                p = end + 1;
                break;
            }
        case 'd':{
                list->set<int32_t>(parseInt(& p, 'd'));
                break;   
            }
        case 'x':{
                list->set<int32_t>(parseInt(& p, 'x'));
                break;
            }
        case 'm': {
                parseNetCode(list, & p, 'x', 6);
                break;
            }
        case 'i':{
                parseNetCode(list, & p, 'd', 4);
                break;
            }
        case '%':{
                if (p[0] != '%'){
                    return false;
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


#define WRITE_NUM(fmt)          \
auto v = list->get<int32_t>();  \
if (mayHidden){                 \
    if (v == leaveOut){         \
        continue;               \
    }                           \
}                               \
sprintf(buf, fmt, v);           \
write(buf);                     \
break;

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
                    continue;
                }
                write('\"');
                write(list->get<String>().c_str());
                write('\"');
                break;
            }
        case 'd':{
                WRITE_NUM("%d");
            }
        case 'x':{
                WRITE_NUM("%x");
            }
        case 'm': {
                if (list->ptr<uint8_t>() == nullptr){
                    continue;
                }
                auto mac = list->ptr<uint8_t>();
                sprintf(buf, "\"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\"",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
                );
                write(buf);
                break;
            }
        case 'i':{
                if (list->ptr<uint8_t>() == nullptr){
                    continue;
                }
                auto ip = list->ptr<uint8_t>();
                sprintf(buf, "\"%d.%d.%d.%d\"",
                    ip[0], ip[1], ip[2], ip[3]
                );
                write(buf);
                break;
            }
        case '%':{
                write('%');
                continue;
            }
        }

        // DON'T DO THIS 
        // for(;; list += 1)
        list += 1;
    }
    write(END_LINE);
}

void tx(const char * cmd){
    write(cmd);
    write(END_LINE);
}

CMD(atTest)
    tx(AT);
$

//DON'T USE -> CMD(atEcho, bool enable)
//BUT -> bool atEcho(bool enable)
bool atEcho(bool enable){
    //echo will be enable when reset
    //THIS CMD NEED NOT 'SET' SUFIX
    //AND DON'T USE CMD(atEcho, bool enable) FORMAT, IT WILL RESULT RECURRENCE.
    if (isNotWakeup()) return fail;
    tx(ATE); 
    return waitFlag();
}
