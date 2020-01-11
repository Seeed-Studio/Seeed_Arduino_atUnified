
#include <stdlib.h>
#include "esp32-hal-log.h"
#include "esp_at_lib.h"

//used by hal log
const char * pathToFileName(const char * path)
{
    size_t i = 0;
    size_t pos = 0;
    char * p = (char *)path;
    while(*p){
        i++;
        if(*p == '/' || *p == '\\'){
            pos = i;
        }
        p++;
    }
    return path+pos;
}


int dump_tasks(void) {
    static char TaskListBuf[0x180];

    vTaskList(TaskListBuf);
    printf(TaskListBuf);
    return 0;
}
