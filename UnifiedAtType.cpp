#include"UnifiedAtType.h"
#define ATE             "ATE"
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
volatile bool idEndLine = true;

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

bool subrx(const char * prefix, any * list){
    String  resp = readLine();
    auto    str = (char *)resp.c_str();
    auto    len = strlen(prefix);

    if (resp.indexOf(prefix) != 0){
        return fail;
    }
    str += len;
    
    while(str[0]){
        if (str[0] == ','){
            str += 1;
            continue;
        }
        if (list->skip){
            str += list->skip;
            list += 1;
        }
        if (str[0] == '\"'){
            str += 1;
            auto len = strchr(str, '\"') - str;
            str[len] = '\0';
            list->set<String>(str);
            str += len + 1;
        }
        else if(list->type == string){
            list->set<String>(str);
            break;
        }
        else{
            auto end = str;
            for (; isxdigit(end[0]); end++){
                ;
            }
            sscanf(str, list->type == hex ? "%x" : "%d", list->v);
            str = end;
        }

        //DON't do this: for(;; list += 1)
        list += 1;
    }
    return success;
}

void write(int32_t value){
    char buf[sizeof(int32_t) * 8];
    sprintf(buf, "%d", value);
    write(buf);
}

void write(String const & value){
    write("\"");
    write(value.c_str());
    write("\"");
}

void write(ip_t * ip){
    write(ip[0]); write(".");
    write(ip[1]); write(".");
    write(ip[2]); write(".");
    write(ip[3]);
}

void write(mac_t one){
    const char * map = "0123456789abcdef";
    char buf[] = { 
        map[one & 0xf], 
        map[one >> 0xf], '\0' };
    write(buf);
}

void write(mac_t * mac){
    write(mac[0]); write(":");
    write(mac[1]); write(":");
    write(mac[2]); write(":");
    write(mac[3]); write(":");
    write(mac[4]); write(":");
    write(mac[5]); write(":");
    write(mac[6]); write(":");
    write(mac[7]);
}

void tx(){
    write("\r\n");
    idEndLine = true;
}

CMD(atTest)
    tx(AT);
$

//DON'T USE -> CMD(atEcho, bool enable)
//BUT -> bool atEcho(bool enable)
bool atEcho(bool enable){
    extern bool isNotWakeup();
    //echo will be enable when reset
    //THIS CMD NEED NOT 'SET' SUFIX
    //AND DON'T USE CMD(atEcho, bool enable) FORMAT, IT WILL RESULT RECURRENCE.
    if (isNotWakeup()) return fail;
    tx(enable ? ATE "1" : ATE "0"); 
    return waitFlag();
}
