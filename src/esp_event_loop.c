/*
 *  The MIT License (MIT)
 *  Copyright (C) 2019  Seeed Technology Co.,Ltd.
 */
#include "esp_at_lib.h"
#include "esp_event_loop.h"
#include "esp_hal_log.h"

/**
 * \brief           Save callback function for high level (AtWiFi) event
 */
static system_event_cb_t sys_evt_callback = NULL;

static void esp_sta_getip_cb(espr_t res, void* arg) {
	system_event_t sys_evt;
	tcpip_adapter_ip_info_t* ip_info = (tcpip_adapter_ip_info_t*)arg;

	sys_evt.event_id = SYSTEM_EVENT_STA_GOT_IP;
	sys_evt.event_info.got_ip.ip_info = *ip_info;

	if (sys_evt_callback) {
		(*sys_evt_callback)(NULL, &sys_evt);
	}
	return;
}

/**
 * \brief           Callback function for low level  (esp-at-lib) operation
 */
static espr_t at_unified_cb(esp_evt_t* evt) {
	system_event_t sys_evt;
	int eid = -1, r;

	switch (esp_evt_get_type(evt)) {
	/*!< Station listed APs event */
	case ESP_EVT_STA_LIST_AP:
		eid = SYSTEM_EVENT_SCAN_DONE; break;

	/*!< Join to access point */
	case ESP_EVT_STA_JOIN_AP:
		break;

	/*!< Station AP info (name, mac, channel, rssi) */
	case ESP_EVT_STA_INFO_AP:
		break;

	/*!< Station just connected to AP */
	case ESP_EVT_WIFI_CONNECTED:
		eid = SYSTEM_EVENT_STA_CONNECTED;break;

	/*!< Station just disconnected from AP */
	case ESP_EVT_WIFI_DISCONNECTED:
		eid = SYSTEM_EVENT_STA_DISCONNECTED; break;

	/*!< Station has valid IP.
	 * When this event is received to application, no IP has been read from device.
	 * Stack will proceed with IP read from device and 
	 * will later send \ref ESP_EVT_WIFI_IP_ACQUIRED event
	 */
	case ESP_EVT_WIFI_GOT_IP:
		{
		static tcpip_adapter_ip_info_t ip_info[1];
		r = esp_sta_getip((esp_ip_t*)&ip_info->ip,
		                  (esp_ip_t*)&ip_info->gw,
		                  (esp_ip_t*)&ip_info->netmask, esp_sta_getip_cb, ip_info, 0);
		// eid = SYSTEM_EVENT_STA_GOT_IP;
		// when esp_sta_getip() complete, esp_sta_getip_cb() will be called.
		}
		return espOK;

	/*!< Station IP address acquired.
	 * At this point, valid IP address has been received from device.
	 * Application may use \ref esp_sta_copy_ip function to read it
	 */
	case ESP_EVT_WIFI_IP_ACQUIRED:

	/*!< Server status changed */
	case ESP_EVT_SERVER:
		return espOK;

	/*!< New station just connected to ESP's access point */
	case ESP_EVT_AP_CONNECTED_STA:
		eid = SYSTEM_EVENT_AP_STACONNECTED; break;

	/*!< New station just disconnected from ESP's access point */
	case ESP_EVT_AP_DISCONNECTED_STA:
		eid = SYSTEM_EVENT_AP_STADISCONNECTED; break;

	/*!< New station just received IP from ESP's access point */
	case ESP_EVT_AP_IP_STA:
		eid = SYSTEM_EVENT_AP_STAIPASSIGNED; break;

	/*!< DNS domain service finished */
	case ESP_EVT_DNS_HOSTBYNAME:
		return espOK;

	/*!< DNS domain service finished */
	default:
		log_e("at_unified evt: %d\r\n", esp_evt_get_type(evt));
		return espERR;
	}

	if (!sys_evt_callback || eid < 0) {
		return espOK;
	}

	sys_evt.event_id = eid;
	r = (*sys_evt_callback)(NULL, &sys_evt);
	if (r != ESP_OK) {
		return espERR;
	}
	return espOK;
}

/**
  * @brief  Initialize event loop
  *         Create the event handler and task
  *
  * @param  system_event_cb_t cb : application specified event callback, it can be modified by call esp_event_set_cb
  * @param  void *ctx : reserved for user
  *
  * @return ESP_OK : succeed
  * @return others : fail
  */
esp_err_t esp_event_loop_init(system_event_cb_t cb, void *ctx) {
	sys_evt_callback = cb;
	esp_evt_register(at_unified_cb);
	return ESP_OK;
}

/**
  * @brief  Set application specified event callback function
  *
  * @attention 1. If cb is NULL, means application don't need to handle
  *               If cb is not NULL, it will be call when an event is received, after the default event callback is completed
  *
  * @param  system_event_cb_t cb : callback
  * @param  void *ctx : reserved for user
  *
  * @return system_event_cb_t : old callback
  */
system_event_cb_t esp_event_loop_set_cb(system_event_cb_t cb, void *ctx)
{
	system_event_cb_t old_cb;

	old_cb = sys_evt_callback;
	sys_evt_callback = cb;
	return old_cb;
}

/**
  * @brief  Get the queue used by event loop
  *
  * @attention : currently this API is used to initialize "q" parameter
  * of wifi_init structure.
  *
  * @return QueueHandle_t : event queue handle
  */
QueueHandle_t esp_event_loop_get_queue(void)
{ return 0; }
