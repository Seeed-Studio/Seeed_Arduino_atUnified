/*
    The MIT License (MIT)
    Copyright (C) 2019  Seeed Technology Co.,Ltd.
*/
#ifndef _SEED_ATUNIFIED_H__
#define _SEED_ATUNIFIED_H__

#include <Arduino.h>

//#define LWIP_PROVIDE_ERRNO 1
#define LWIP_ERRNO_INCLUDE "sys/errno.h"
#include "esp_at_lib.h"
#include "lwip/err.h"
#include "lwip/errno.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"
#include "atu_sockets.h"
#include "atu_netdb.h"
#undef IPADDR_NONE
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
//#include "esp_ipc.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "tcpip_adapter.h"
#include "esp_hal_log.h"

#endif//_SEED_ATUNIFIED_H__
