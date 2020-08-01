// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License") {}
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


/*               Notes about WiFi Programming
 *
 *  The esp32 WiFi programming model can be depicted as following picture:
 *
 *
 *                            default handler              user handler
 *  -------------             ---------------             ---------------
 *  |           |   event     |             | callback or |             |
 *  |   tcpip   | --------->  |    event    | ----------> | application |
 *  |   stack   |             |     task    |  event      |    task     |
 *  |-----------|             |-------------|             |-------------|
 *                                  /|\                          |
 *                                   |                           |
 *                            event  |                           |
 *                                   |                           |
 *                                   |                           |
 *                             ---------------                   |
 *                             |             |                   |
 *                             | WiFi Driver |/__________________|
 *                             |             |\     API call
 *                             |             |
 *                             |-------------|
 *
 * The WiFi driver can be consider as black box, it knows nothing about the high layer code, such as
 * TCPIP stack, application task, event task etc, all it can do is to receive API call from high layer
 * or post event queue to a specified Queue, which is initialized by API esp_wifi_init().
 *
 * The event task is a daemon task, which receives events from WiFi driver or from other subsystem, such
 * as TCPIP stack, event task will call the default callback function on receiving the event. For example,
 * on receiving event SYSTEM_EVENT_STA_CONNECTED, it will call tcpip_adapter_start() to start the DHCP
 * client in it's default handler.
 *
 * Application can register it's own event callback function by API esp_event_init, then the application callback
 * function will be called after the default callback. Also, if application doesn't want to execute the callback
 * in the event task, what it needs to do is to post the related event to application task in the application callback function.
 *
 * The application task (code) generally mixes all these thing together, it calls APIs to init the system/WiFi and
 * handle the events when necessary.
 *
 */
#include <stdlib.h>
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_at_lib.h"
#include "esp_smartconfig.h"
#include "esp_hal_log.h"

/**
  * @brief  Init WiFi
  *         Alloc resource for WiFi driver, such as WiFi control structure, RX/TX buffer,
  *         WiFi NVS structure etc, this WiFi also start WiFi task
  *
  * @attention 1. This API must be called before all other WiFi API can be called
  * @attention 2. Always use WIFI_INIT_CONFIG_DEFAULT macro to init the config to default values, this can
  *               guarantee all the fields got correct value when more fields are added into wifi_init_config_t
  *               in future release. If you want to set your owner initial values, overwrite the default values
  *               which are set by WIFI_INIT_CONFIG_DEFAULT, please be notified that the field 'magic' of 
  *               wifi_init_config_t should always be WIFI_INIT_CONFIG_MAGIC!
  *
  * @param  config pointer to WiFi init configuration structure; can point to a temporary variable.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_NO_MEM: out of memory
  *    - others: refer to error code esp_err.h
  */
esp_err_t esp_wifi_init(const wifi_init_config_t *config) {
	config = config;
	return ESP_OK;
}

/**
  * @brief  Deinit WiFi
  *         Free all resource allocated in esp_wifi_init and stop WiFi task
  *
  * @attention 1. This API should be called if you want to remove WiFi driver from the system
  *
  * @return 
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_deinit(void) {
	return ESP_OK;
}

/**
  * @brief     Set the WiFi operating mode
  *
  *            Set the WiFi operating mode as station, soft-AP or station+soft-AP,
  *            The default mode is soft-AP mode.
  *
  * @param     mode  WiFi operating mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - others: refer to error code in esp_err.h
  */
esp_err_t esp_wifi_set_mode(wifi_mode_t mode) {
	return ESPR_TO_ESP_ERR(esp_set_wifi_mode(mode, NULL, NULL, true));
}

/**
  * @brief  Get current operating mode of WiFi
  *
  * @param[out]  mode  store current WiFi mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_mode(wifi_mode_t *mode) {
	return ESPR_TO_ESP_ERR(esp_get_wifi_mode(mode, NULL, NULL, true));
}

/**
  * @brief  Start WiFi according to current configuration
  *         If mode is WIFI_MODE_STA, it create station control block and start station
  *         If mode is WIFI_MODE_AP, it create soft-AP control block and start soft-AP
  *         If mode is WIFI_MODE_APSTA, it create soft-AP and station control block and start soft-AP and station
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_NO_MEM: out of memory
  *    - ESP_ERR_WIFI_CONN: WiFi internal error, station or soft-AP control block wrong
  *    - ESP_FAIL: other WiFi internal errors
  */
esp_err_t esp_wifi_start(void) {
	return ESP_OK;
}

/**
  * @brief  Stop WiFi
  *         If mode is WIFI_MODE_STA, it stop station and free station control block
  *         If mode is WIFI_MODE_AP, it stop soft-AP and free soft-AP control block
  *         If mode is WIFI_MODE_APSTA, it stop station/soft-AP and free station/soft-AP control block
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_stop(void) {
	return ESP_OK;
}

/**
 * @brief  Restore WiFi stack persistent settings to default values
 *
 * This function will reset settings made using the following APIs:
 * - esp_wifi_get_auto_connect,
 * - esp_wifi_set_protocol,
 * - esp_wifi_set_config related
 * - esp_wifi_set_mode
 *
 * @return
 *    - ESP_OK: succeed
 *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
 */
esp_err_t esp_wifi_restore(void) {
	return ESPR_TO_ESP_ERR(esp_restore(NULL, NULL, true));
}

/**
  * @brief     Connect the ESP32 WiFi station to the AP.
  *
  * @attention 1. This API only impact WIFI_MODE_STA or WIFI_MODE_APSTA mode
  * @attention 2. If the ESP32 is connected to an AP, call esp_wifi_disconnect to disconnect.
  * @attention 3. The scanning triggered by esp_wifi_start_scan() will not be effective until connection between ESP32 and the AP is established.
  *               If ESP32 is scanning and connecting at the same time, ESP32 will abort scanning and return a warning message and error
  *               number ESP_ERR_WIFI_STATE.
  *               If you want to do reconnection after ESP32 received disconnect event, remember to add the maximum retry time, otherwise the called	
  *               scan will not work. This is especially true when the AP doesn't exist, and you still try reconnection after ESP32 received disconnect
  *               event with the reason code WIFI_REASON_NO_AP_FOUND.
  *   
  * @return 
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_WIFI_CONN: WiFi internal error, station or soft-AP control block wrong
  *    - ESP_ERR_WIFI_SSID: SSID of AP which station connects is invalid
  */
static wifi_sta_config_t sta_config[1], *sta_to_connect = NULL;

esp_err_t esp_wifi_connect(void) {
	wifi_sta_config_t* sta = sta_to_connect;
	const esp_mac_t* mac;
	espr_t r;

	log_v(" +++\r\n");

	if (!sta) {
		return ESP_ERR_WIFI_CONN;
	}
	mac = sta->bssid_set? (const esp_mac_t*)sta->bssid: NULL;
	r = esp_sta_join(sta->ssid, sta->password, mac, NULL, NULL, 0);
	return ESPR_TO_ESP_ERR(r);
}

/**
  * @brief     Disconnect the ESP32 WiFi station from the AP.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi was not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
  *    - ESP_FAIL: other WiFi internal errors
  */
esp_err_t esp_wifi_disconnect(void) {
	return ESPR_TO_ESP_ERR(esp_sta_quit(NULL, NULL, 1));
}

/**
  * @brief     Currently this API is just an stub API
  *

  * @return
  *    - ESP_OK: succeed
  *    - others: fail
  */
esp_err_t esp_wifi_clear_fast_connect(void) {
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     deauthenticate all stations or associated id equals to aid
  *
  * @param     aid  when aid is 0, deauthenticate all stations, otherwise deauthenticate station whose associated id is aid
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_MODE: WiFi mode is wrong
  */
esp_err_t esp_wifi_deauth_sta(uint16_t aid) {
	aid = aid;
	log_v(" +++\r\n");
	return ESP_OK;
}


/**
 * \brief           List of access points found by ESP device
 */
static
esp_ap_t aps[100];

/**
 * \brief           Number of valid access points in \ref aps array
 */
static
size_t apf;

/**
  * @brief     Scan all available APs.
  *
  * @attention If this API is called, the found APs are stored in WiFi driver dynamic allocated memory and the
  *            will be freed in esp_wifi_scan_get_ap_records, so generally, call esp_wifi_scan_get_ap_records to cause
  *            the memory to be freed once the scan is done
  * @attention The values of maximum active scan time and passive scan time per channel are limited to 1500 milliseconds.
  *            Values above 1500ms may cause station to disconnect from AP and are not recommended.
  *
  * @param     config  configuration of scanning
  * @param     block if block is true, this API will block the caller until the scan is done, otherwise
  *                         it will return immediately
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
  *    - ESP_ERR_WIFI_TIMEOUT: blocking scan is timeout
  *    - ESP_ERR_WIFI_STATE: wifi still connecting when invoke esp_wifi_scan_start
  *    - others: refer to error code in esp_err.h
  */
esp_err_t esp_wifi_scan_start(const wifi_scan_config_t *config, bool block) {
	espr_t r;

	r = esp_sta_list_ap(NULL, aps, ESP_ARRAYSIZE(aps), &apf, NULL, NULL, block);
	return ESPR_TO_ESP_ERR(r);
}

/**
  * @brief     Stop the scan in process
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi is not started by esp_wifi_start
  */
esp_err_t esp_wifi_scan_stop(void) {
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get number of APs found in last scan
  *
  * @param[out] number  store number of APIs found in last scan
  *
  * @attention This API can only be called when the scan is completed, otherwise it may get wrong value.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_scan_get_ap_num(uint16_t *number) {
	*number = apf;
	return ESP_OK;
}

/**
  * @brief     Get AP list found in last scan
  *
  * @param[inout]  number As input param, it stores max AP number ap_records can hold. 
  *                As output param, it receives the actual AP number this API returns.
  * @param         ap_records  wifi_ap_record_t array to hold the found APs
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_NO_MEM: out of memory
  */
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
esp_err_t esp_wifi_scan_get_ap_records(uint16_t *number, wifi_ap_record_t *ap_records) {
	int i;
	wifi_ap_record_t * ap;

	for (i = 0; i < *number; i++) {
		ap = &ap_records[i];

		memcpy(ap->bssid, aps[i].mac.mac, sizeof aps[i].mac);

		strncpy(ap->ssid, aps[i].ssid, sizeof aps[i].ssid);
		ap->ssid[min(sizeof ap->ssid, sizeof aps[i].ssid) - 1] = '\0';

		ap->primary = aps[i].ch;

		ap->rssi = aps[i].rssi;
		
		ap->authmode = aps[i].ecn;

		ap->phy_11b = esp_sta_is_ap_802_11b(&aps[i]);
		ap->phy_11g = esp_sta_is_ap_802_11g(&aps[i]);
		ap->phy_11n = esp_sta_is_ap_802_11n(&aps[i]);
	}
	return ESP_OK;
}


/**
  * @brief     Get information of AP which the ESP32 station is associated with
  *
  * @param     ap_info  the wifi_ap_record_t to hold AP information
  *            sta can get the connected ap's phy mode info through the struct member
  *            phy_11b，phy_11g，phy_11n，phy_lr in the wifi_ap_record_t struct.
  *            For example, phy_11b = 1 imply that ap support 802.11b mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_CONN: The station interface don't initialized
  *    - ESP_ERR_WIFI_NOT_CONNECT: The station is in disconnect status 
  */
esp_err_t esp_wifi_sta_get_ap_info(wifi_ap_record_t *ap_info) {
	esp_sta_info_ap_t info[1];
	espr_t r;

	r = esp_sta_get_ap_info(info, NULL, NULL, 1);
	if (r == espOK) {
		memcpy(ap_info->bssid, info->mac.mac, sizeof ap_info->bssid);
		strcpy(ap_info->ssid,  info->ssid);
		ap_info->rssi    = info->rssi;
		ap_info->primary = info->ch;
	}
	return ESPR_TO_ESP_ERR(r);
}

/**
  * @brief     Set current WiFi power save type
  *
  * @attention Default power save type is WIFI_PS_MIN_MODEM.
  *
  * @param     type  power save type
  *
  * @return    ESP_OK: succeed
  */
esp_err_t esp_wifi_set_ps(wifi_ps_type_t type) {
	type = type;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get current WiFi power save type
  *
  * @attention Default power save type is WIFI_PS_MIN_MODEM.
  *
  * @param[out]  type: store current power save type
  *
  * @return    ESP_OK: succeed
  */
esp_err_t esp_wifi_get_ps(wifi_ps_type_t *type) {
	type = type;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set protocol type of specified interface
  *            The default protocol is (WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)
  *
  * @attention Currently we only support 802.11b or 802.11bg or 802.11bgn mode
  *
  * @param     ifx  interfaces
  * @param     protocol_bitmap  WiFi protocol bitmap
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_set_protocol(wifi_interface_t ifx, uint8_t protocol_bitmap) {
	ifx = ifx;
	protocol_bitmap = protocol_bitmap;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get the current protocol bitmap of the specified interface
  *
  * @param     ifx  interface
  * @param[out] protocol_bitmap  store current WiFi protocol bitmap of interface ifx
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_get_protocol(wifi_interface_t ifx, uint8_t *protocol_bitmap) {
	ifx = ifx;
	protocol_bitmap = protocol_bitmap;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set the bandwidth of ESP32 specified interface
  *
  * @attention 1. API return false if try to configure an interface that is not enabled
  * @attention 2. WIFI_BW_HT40 is supported only when the interface support 11N
  *
  * @param     ifx  interface to be configured
  * @param     bw  bandwidth
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_set_bandwidth(wifi_interface_t ifx, wifi_bandwidth_t bw) {
	ifx = ifx;
	bw = bw;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get the bandwidth of ESP32 specified interface
  *
  * @attention 1. API return false if try to get a interface that is not enable
  *
  * @param     ifx interface to be configured
  * @param[out] bw  store bandwidth of interface ifx
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_bandwidth(wifi_interface_t ifx, wifi_bandwidth_t *bw) {
	ifx = ifx;
	bw = bw;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set primary/secondary channel of ESP32
  *
  * @attention 1. This is a special API for sniffer
  * @attention 2. This API should be called after esp_wifi_start() or esp_wifi_set_promiscuous()
  *
  * @param     primary  for HT20, primary is the channel number, for HT40, primary is the primary channel
  * @param     second   for HT20, second is ignored, for HT40, second is the second channel
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second) {
	primary = primary;
	second = second;
	log_v(" +++\r\n");

	// Not implemented.
	return ESP_OK;
}

/**
  * @brief     Get the primary/secondary channel of ESP32
  *
  * @attention 1. API return false if try to get a interface that is not enable
  *
  * @param     primary   store current primary channel
  * @param[out]  second  store current second channel
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_channel(uint8_t *primary, wifi_second_chan_t *second) {
	second = second;
	log_v(" +++\r\n");

	// Not implemented.
	*primary = 0;
	return ESP_OK;
}

/**
  * @brief     configure country info
  *
  * @attention 1. The default country is {.cc="CN", .schan=1, .nchan=13, policy=WIFI_COUNTRY_POLICY_AUTO}
  * @attention 2. When the country policy is WIFI_COUNTRY_POLICY_AUTO, the country info of the AP to which
  *               the station is connected is used. E.g. if the configured country info is {.cc="USA", .schan=1, .nchan=11}
  *               and the country info of the AP to which the station is connected is {.cc="JP", .schan=1, .nchan=14}
  *               then the country info that will be used is {.cc="JP", .schan=1, .nchan=14}. If the station disconnected
  *               from the AP the country info is set back back to the country info of the station automatically,
  *               {.cc="US", .schan=1, .nchan=11} in the example.
  * @attention 3. When the country policy is WIFI_COUNTRY_POLICY_MANUAL, always use the configured country info.
  * @attention 4. When the country info is changed because of configuration or because the station connects to a different
  *               external AP, the country IE in probe response/beacon of the soft-AP is changed also.
  * @attention 5. The country configuration is not stored into flash
  * @attention 6. This API doesn't validate the per-country rules, it's up to the user to fill in all fields according to 
  *               local regulations.
  *
  * @param     country   the configured country info
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_country(const wifi_country_t *country) {
	esp_country_t ec;

	log_v(" +++\r\n");

	ec.country = country->cnty_rtl;
	ec.channel_plan = country->channel_plan;
	return ESPR_TO_ESP_ERR(esp_set_wifi_country(ec, NULL, NULL, true));
}


/**
  * @brief     get the current country info
  *
  * @param     country  country info
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_country(wifi_country_t *country) {
	esp_country_t ec[1];
	espr_t r;

	log_v(" +++\r\n");
	r = esp_get_wifi_country(ec, NULL, NULL, true);

	if (espOK == r) {
		country->cnty_rtl = ec->country;
		country->channel_plan = ec->channel_plan;
	}
	return ESPR_TO_ESP_ERR(r);
}


/**
  * @brief     Set MAC address of the ESP32 WiFi station or the soft-AP interface.
  *
  * @attention 1. This API can only be called when the interface is disabled
  * @attention 2. ESP32 soft-AP and station have different MAC addresses, do not set them to be the same.
  * @attention 3. The bit 0 of the first byte of ESP32 MAC address can not be 1. For example, the MAC address
  *      can set to be "1a:XX:XX:XX:XX:XX", but can not be "15:XX:XX:XX:XX:XX".
  *
  * @param     ifx  interface
  * @param     mac  the MAC address
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_WIFI_MAC: invalid mac address
  *    - ESP_ERR_WIFI_MODE: WiFi mode is wrong
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_set_mac(wifi_interface_t ifx, const uint8_t mac[6]) {
	ifx = ifx;
	log_v(" +++\r\n");
	// esp_sta_setmac
	return ESP_OK;
}

/**
  * @brief     Get mac of specified interface
  *
  * @param      ifx  interface
  * @param[out] mac  store mac of the interface ifx
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  */
esp_err_t esp_wifi_get_mac(wifi_interface_t ifx, uint8_t mac[6]) {
	ifx = ifx;
	mac[0] = mac[0];
	log_v(" +++\r\n");
	// esp_sta_getmac
	return ESP_OK;
}

/**
  * @brief Register the RX callback function in the promiscuous mode.
  *
  * Each time a packet is received, the registered callback function will be called.
  *
  * @param cb  callback
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb) {
	cb = cb;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Enable the promiscuous mode.
  *
  * @param     en  false - disable, true - enable
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous(bool en) {
	en = en;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get the promiscuous mode.
  *
  * @param[out] en  store the current status of promiscuous mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_promiscuous(bool *en) {
	en = en;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief Enable the promiscuous mode packet type filter.
  *
  * @note The default filter is to filter all packets except WIFI_PKT_MISC
  *
  * @param filter the packet type filtered in promiscuous mode.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous_filter(const wifi_promiscuous_filter_t *filter) {
	filter = filter;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get the promiscuous filter.
  *
  * @param[out] filter  store the current status of promiscuous filter
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_promiscuous_filter(wifi_promiscuous_filter_t *filter) {
	filter = filter;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief Enable subtype filter of the control packet in promiscuous mode.
  *
  * @note The default filter is to filter none control packet.
  *
  * @param filter the subtype of the control packet filtered in promiscuous mode.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous_ctrl_filter(const wifi_promiscuous_filter_t *filter) {
	filter = filter;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get the subtype filter of the control packet in promiscuous mode.
  *
  * @param[out] filter  store the current status of subtype filter of the control packet in promiscuous mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_ARG: invalid argument
  */
esp_err_t esp_wifi_get_promiscuous_ctrl_filter(wifi_promiscuous_filter_t *filter) {
	filter = filter;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set the configuration of the ESP32 STA or AP
  *
  * @attention 1. This API can be called only when specified interface is enabled, otherwise, API fail
  * @attention 2. For station configuration, bssid_set needs to be 0; and it needs to be 1 only when users need to check the MAC address of the AP.
  * @attention 3. ESP32 is limited to only one channel, so when in the soft-AP+station mode, the soft-AP will adjust its channel automatically to be the same as
  *               the channel of the ESP32 station.
  *
  * @param     interface  interface, WIFI_IF_STA or WIFI_IF_AP
  * @param     conf  station or soft-AP configuration
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_WIFI_MODE: invalid mode
  *    - ESP_ERR_WIFI_PASSWORD: invalid password
  *    - ESP_ERR_WIFI_NVS: WiFi internal NVS error
  *    - others: refer to the erro code in esp_err.h
  */
esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t *conf) {
	log_v(" +++\r\n");
	if (interface == WIFI_IF_STA) {
		// a trick connecting by esp_wifi_connect() late
		wifi_sta_config_t* sta = &conf->sta;
		*(sta_to_connect = sta_config) = *sta;
		return ESP_OK;
	} else if (interface == WIFI_IF_AP) {
		wifi_ap_config_t * ap  = &conf->ap;
		espr_t r;

		r = esp_ap_configure(ap->ssid, ap->password,
		                     ap->channel, ap->authmode,
		                     ap->max_connection, ap->ssid_hidden,
		                     NULL, NULL, 1);
		return ESPR_TO_ESP_ERR(r);
	}
	return ESP_ERR_INVALID_ARG;
}

/**
  * @brief     Get configuration of specified interface
  *
  * @param     interface  interface, WIFI_IF_STA or WIFI_IF_AP
  * @param[out]  conf  station or soft-AP configuration
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  */
esp_err_t esp_wifi_get_config(wifi_interface_t interface, wifi_config_t *conf) {
	espr_t r;

	log_v(" +++\r\n");
	if (interface == WIFI_IF_STA) {
		wifi_sta_config_t* sta = &conf->sta;
		esp_sta_info_ap_t info[1];

		r = esp_sta_get_ap_info(info, NULL, NULL, 1);
		if (r == espOK) {
			strcpy(sta->ssid, info->ssid);
			sta->bssid_set = true;
			memcpy(sta->bssid, info->mac.mac, sizeof sta->bssid);
			sta->channel = info->ch;
		}
		return ESPR_TO_ESP_ERR(r);
	} else if (interface == WIFI_IF_AP) {
		wifi_ap_config_t * ap  = &conf->ap;
		esp_ap_conf_t conf[1];
		r = esp_ap_get_conf(conf, NULL, NULL, 1);
		if (r == espOK) {
			ap->ssid_len = strlen(conf->ssid);
			strcpy(ap->ssid, conf->ssid);
			strcpy(ap->password, conf->pwd);
			ap->channel = conf->ch;
			ap->authmode = conf->ecn;
			ap->ssid_hidden = conf->hidden;
			ap->max_connection = conf->max_cons;
		}
		return ESPR_TO_ESP_ERR(r);
	}
	return ESP_ERR_INVALID_ARG;
}

/**
  * @brief     Get STAs associated with soft-AP
  *
  * @attention SSC only API
  *
  * @param[out] sta  station list
  *             ap can get the connected sta's phy mode info through the struct member
  *             phy_11b，phy_11g，phy_11n，phy_lr in the wifi_sta_info_t struct.
  *             For example, phy_11b = 1 imply that sta support 802.11b mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_MODE: WiFi mode is wrong
  *    - ESP_ERR_WIFI_CONN: WiFi internal error, the station/soft-AP control block is invalid
  */
esp_err_t esp_wifi_ap_get_sta_list(wifi_sta_list_t *sta) {
	sta = sta;
	log_v(" +++\r\n");
	return ESP_OK;
}


/**
  * @brief     Set the WiFi API configuration storage type
  *
  * @attention 1. The default value is WIFI_STORAGE_FLASH
  *
  * @param     storage : storage type
  *
  * @return
  *   - ESP_OK: succeed
  *   - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *   - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_storage(wifi_storage_t storage) {
	storage = storage;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set auto connect
  *            The default value is true
  *
  * @param     en : true - enable auto connect / false - disable auto connect
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_MODE: WiFi internal error, the station/soft-AP control block is invalid
  *    - others: refer to error code in esp_err.h
  */
esp_err_t esp_wifi_set_auto_connect(bool en) {
	en = en;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get the auto connect flag
  *
  * @param[out] en  store current auto connect configuration
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_auto_connect(bool *en) {
	en = en;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set 802.11 Vendor-Specific Information Element
  *
  * @param     enable If true, specified IE is enabled. If false, specified IE is removed.
  * @param     type Information Element type. Determines the frame type to associate with the IE.
  * @param     idx  Index to set or clear. Each IE type can be associated with up to two elements (indices 0 & 1).
  * @param     vnd_ie Pointer to vendor specific element data. First 6 bytes should be a header with fields matching vendor_ie_data_t.
  *            If enable is false, this argument is ignored and can be NULL. Data does not need to remain valid after the function returns.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init()
  *    - ESP_ERR_INVALID_ARG: Invalid argument, including if first byte of vnd_ie is not WIFI_VENDOR_IE_ELEMENT_ID (0xDD)
  *      or second byte is an invalid length.
  *    - ESP_ERR_NO_MEM: Out of memory
  */
esp_err_t esp_wifi_set_vendor_ie(bool enable, wifi_vendor_ie_type_t type, wifi_vendor_ie_id_t idx, const void *vnd_ie) {
	enable = enable;
	type = type;
	idx = idx;
	vnd_ie = vnd_ie;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Register Vendor-Specific Information Element monitoring callback.
  *
  * @param     cb   Callback function
  * @param     ctx  Context argument, passed to callback function.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_vendor_ie_cb(esp_vendor_ie_cb_t cb, void *ctx) {
	cb = cb;
	ctx = ctx;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set maximum WiFi transmitting power
  *
  * @param     power  Maximum WiFi transmitting power, unit is 0.25dBm, range is [40, 82] corresponding to 10dBm - 20.5dBm here.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_WIFI_NOT_ARG: invalid argument
  */
esp_err_t esp_wifi_set_max_tx_power(int8_t power) {
	power = power;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get maximum WiFi transmiting power
  *
  * @param     power  Maximum WiFi transmitting power, unit is 0.25dBm.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_max_tx_power(int8_t *power) {
	power = power;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set mask to enable or disable some WiFi events
  *
  * @attention 1. Mask can be created by logical OR of various WIFI_EVENT_MASK_ constants.
  *               Events which have corresponding bit set in the mask will not be delivered to the system event handler.
  * @attention 2. Default WiFi event mask is WIFI_EVENT_MASK_AP_PROBEREQRECVED.
  * @attention 3. There may be lots of stations sending probe request data around.
  *               Don't unmask this event unless you need to receive probe request data.
  *
  * @param     mask  WiFi event mask.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_event_mask(uint32_t mask) {
	mask = mask;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get mask of WiFi events
  *
  * @param     mask  WiFi event mask.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_ARG: invalid argument
  */
esp_err_t esp_wifi_get_event_mask(uint32_t *mask) {
	mask = mask;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Send raw ieee80211 data
  *
  * @attention Currently only support for sending beacon/probe request/probe response/action and non-QoS
  *            data frame
  * 
  * @param     ifx interface if the Wi-Fi mode is Station, the ifx should be WIFI_IF_STA. If the Wi-Fi
  *            mode is SoftAP, the ifx should be WIFI_IF_AP. If the Wi-Fi mode is Station+SoftAP, the 
  *            ifx should be WIFI_IF_STA or WIFI_IF_AP. If the ifx is wrong, the API returns ESP_ERR_WIFI_IF.
  * @param     buffer raw ieee80211 buffer
  * @param     len the length of raw buffer, the len must be <= 1500 Bytes and >= 24 Bytes
  * @param     en_sys_seq indicate whether use the internal sequence number. If en_sys_seq is false, the 
  *            sequence in raw buffer is unchanged, otherwise it will be overwritten by WiFi driver with 
  *            the system sequence number.
  *            Generally, if esp_wifi_80211_tx is called before the Wi-Fi connection has been set up, both
  *            en_sys_seq==true and en_sys_seq==false are fine. However, if the API is called after the Wi-Fi
  *            connection has been set up, en_sys_seq must be true, otherwise ESP_ERR_WIFI_ARG is returned.
  *
  * @return
  *    - ESP_OK: success
  *    - ESP_ERR_WIFI_IF: Invalid interface
  *    - ESP_ERR_INVALID_ARG: Invalid parameter
  *    - ESP_ERR_WIFI_NO_MEM: out of memory
  */

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq) {
	ifx = ifx;
	buffer = buffer;
	len = len;
	en_sys_seq = en_sys_seq;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief Register the RX callback function of CSI data.
  *
  *        Each time a CSI data is received, the callback function will be called.
  *
  * @param cb  callback
  * @param ctx context argument, passed to callback function
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */

esp_err_t esp_wifi_set_csi_rx_cb(wifi_csi_cb_t cb, void *ctx) {
	cb = cb;
	ctx = ctx;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief Set CSI data configuration
  *
  * @param config configuration
  * 
  * return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start or promiscuous mode is not enabled
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_csi_config(const wifi_csi_config_t *config) {
	config = config;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief Enable or disable CSI
  *
  * @param en true - enable, false - disable
  *
  * return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start or promiscuous mode is not enabled
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_csi(bool en) {
	en = en;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Set antenna GPIO configuration
  *
  * @param     config  Antenna GPIO configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_ARG: Invalid argument, e.g. parameter is NULL, invalid GPIO number etc
  */
esp_err_t esp_wifi_set_ant_gpio(const wifi_ant_gpio_config_t *config) {
	config = config;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get current antenna GPIO configuration
  *
  * @param     config  Antenna GPIO configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_ARG: invalid argument, e.g. parameter is NULL
  */
esp_err_t esp_wifi_get_ant_gpio(wifi_ant_gpio_config_t *config) {
	config = config;
	log_v(" +++\r\n");
	return ESP_OK;
}


/**
  * @brief     Set antenna configuration
  *
  * @param     config  Antenna configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_ARG: Invalid argument, e.g. parameter is NULL, invalid antenna mode or invalid GPIO number
  */
esp_err_t esp_wifi_set_ant(const wifi_ant_config_t *config) {
	config = config;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Get current antenna configuration
  *
  * @param     config  Antenna configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_ARG: invalid argument, e.g. parameter is NULL
  */
esp_err_t esp_wifi_get_ant(wifi_ant_config_t *config) {
	config = config;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
  * @brief     Start SmartConfig, config ESP device to connect AP. You need to broadcast information by phone APP.
  *            Device sniffer special packets from the air that containing SSID and password of target AP.
  *
  * @attention 1. This API can be called in station or softAP-station mode.
  * @attention 2. Can not call esp_smartconfig_start twice before it finish, please call
  *               esp_smartconfig_stop first.
  *
  * @param     cb  SmartConfig callback function.
  * @param     ... log  1: UART output logs; 0: UART only outputs the result.
  *
  * @return
  *     - ESP_OK: succeed
  *     - others: fail
  */
esp_err_t esp_smartconfig_start(sc_callback_t cb, ...) {
	espr_t r;

	cb = cb;

	log_v(" +++\r\n");

	r = esp_smart_configure(1, NULL, NULL, 1);
	return ESPR_TO_ESP_ERR(r);
}

/**
  * @brief     Stop SmartConfig, free the buffer taken by esp_smartconfig_start.
  *
  * @attention Whether connect to AP succeed or not, this API should be called to free
  *            memory taken by smartconfig_start.
  *
  * @return
  *     - ESP_OK: succeed
  *     - others: fail
  */
esp_err_t esp_smartconfig_stop(void) {
	espr_t r;

	log_v(" +++\r\n");

	r = esp_smart_configure(0, NULL, NULL, 1);
	return ESPR_TO_ESP_ERR(r);
}
