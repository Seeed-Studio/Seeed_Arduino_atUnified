#include"UnifiedAtType.h"

namespace std{
    void __throw_bad_function_call(){
        debug("FUNC:ERROR\n");
    }
    void __throw_bad_alloc() {
        debug("MEM:ERROR\n");
    }
    void __throw_length_error(char const*){
        debug("LEN:ERROR\n");
    }
}
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
        return Fail;
    }
    p[0] = end;
    return Success;
};

// parse mac/ip
// Mac format
// 12:34:56:78:9a:bc
// ip format
// 192.168.1.1
bool parseNetCode(char ** p, char type, uint8_t * buf, size_t length){

    // NOTICE:
    // some at response line use the "12:34:56:78:9a:bc" format, 
    // but some use 12:34:56:78:9a:bc
    // shit design!!!
    if (p[0][0] == '\"'){
        p[0] += 1;
    }
    for (int32_t i = 0, v; i < length; i++){
        if (parseInt(p, type, & v) == Fail){
            return Fail;
        }
        buf[i] = int32_t(v);

        switch(p[0][0]){
        case ':':
        case '.':
        case '\"':
            p[0] += 1;
            break;
        }
    }
};

// Format:
// Mon Dec 12 02:33:32 2016
bool parseDateTime(char ** p, DateTime * time){
    auto     skipEmpty = [](char ** strp){
        char * str = strp[0];
        while(str[0] == ' ' || str[0] == '\t'){
            str++;
        }
        strp[0] = str;
    };

    char *   str = p[0];
    auto     dayOfWeek = 
    !strncmp(str, "Sun", 3) ? Sun :
    !strncmp(str, "Mon", 3) ? Mon :
    !strncmp(str, "Tue", 3) ? Tue :
    !strncmp(str, "Wen", 3) ? Wen :
    !strncmp(str, "Thu", 3) ? Thu :
    !strncmp(str, "Fri", 3) ? Fri :
    !strncmp(str, "Sat", 3) ? Sat : NotDayOfWeek;

    if (dayOfWeek == NotDayOfWeek){
        return Fail;
    }

    str += 4;
    skipEmpty(& str);

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
        return Fail;
    }

    //1 01:18:52 1970
    //Thu Jan  1 01:18:52 1970
    str += 4;
    skipEmpty(& str);
    time->dayOfWeek = dayOfWeek;
    time->month     = month;
    int32_t tmp;
    parseInt(& str, 'd', & tmp); time->day    = tmp; str += 1; // skip ' '
    parseInt(& str, 'd', & tmp); time->hour   = tmp; str += 1; // skip ':'
    parseInt(& str, 'd', & tmp); time->minute = tmp; str += 1; // skip ':'
    parseInt(& str, 'd', & tmp); time->second = tmp; str += 1; // skip ' '
    parseInt(& str, 'd', & tmp); time->year   = tmp;
    p[0] = str;
    return Success;
}

size_t rxMain(Text resp, Any * buf){
    char *  str = (char *)resp.c_str();
    char *  end;
    char    chr;
    size_t  len;
    int32_t i32;
    size_t  c = 0;

    if (str[0] == '('){
        str += 1;
    }

    for(; buf->isEmpty() == false && str[0] != '\r' && str[0] != '\n'; buf += 1, str += 1, c++){ // skip ,
        // debug("%d:%p:%s", buf->type, buf->str, str);
        switch(buf->type){
        case Typei08: 
            parseInt(& str, 'd', & i32); 
            buf->i08[0] = i32;
            // debug("b:%d\n", buf->i08[0]);
            break;
        case Typei32: 
            parseInt(& str, 'd', buf->i32);
            //debug("i:%d\n", buf->i32[0]);
            break;
        case Typex32: 
            parseInt(& str, 'x', buf->i32); 
            //debug("x:%d\n", buf->i32[0]);
            break;
        case Typestr: 
            if (str[0] == '\"') {
                str += 1;
                end = (char *)strchr(str, '\"');
                chr = end[0];
                end[0] = '\0';
                buf->str[0] = Text(str);
                end[0] = chr;
                str = end + 1;
                // debug("[%s]\n", buf->str[0].c_str());
            }
            else {
                buf->str[0] = str;
            }
            break;
        case Typeip:
            parseNetCode(& str, 'd', buf->ip[0], 4);
            // debug("ip:%d.%d.%d.%d\n", buf->ip[0][0], buf->ip[0][1], buf->ip[0][2], buf->ip[0][3]);
            break;
        case Typemac:
            parseNetCode(& str, 'x', buf->mac[0], 6);
            // debug("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", buf->mac[0][0], buf->mac[0][1], buf->mac[0][2], buf->mac[0][3], buf->mac[0][4], buf->mac[0][5]);
            break;
        case Typetime:
            parseDateTime(& str, buf->time);
            break;
        case Typeskip:
            len = strlen(buf->skip);
            str += len;
            break;
        default: 
            break;
        }
    }
    return c;
}

void txMain(Text * resp, Any * buf){
    char tmp[32];
    bool goon = true;
    Ipv4 ip;
    Mac  mac;
    while(buf->isEmpty() == false){ // maybe the first param of buf is empty
        switch(buf->type){
        case Typei08: 
        case Typei32: 
            sprintf(tmp, "%d", buf->i32[0]);
            resp[0] += tmp;
            break;
        case Typestr: 
            resp[0] += '\"';
            resp[0] += buf->str[0];
            resp[0] += '\"';
            break;
        case Typeip:
            ip = buf->ip[0];
            sprintf(tmp, "\"%d.%d.%d.%d\"", ip[0], ip[1], ip[2], ip[3]);
            resp[0] += tmp;
            break;
        case Typemac:
            mac = buf->mac[0];
            sprintf(tmp, "\"%02X:%02X:%02X:%02X:%02X:%02X\"", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            resp[0] += tmp;
            break;
        default: break;
        }
        buf += 1;

        if (buf->isEmpty() || (buf->type == Typestr && buf->str[0] == nullptr)){
            break;
        }
        else {
            resp[0] += ",";
        }
    }
    resp[0] += END_LINE;
}
