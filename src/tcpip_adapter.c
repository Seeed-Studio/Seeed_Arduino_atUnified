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

/**
 * @brief TCPIP adapter library
 *
 * The aim of this adapter is to provide an abstract layer upon TCPIP stack.
 * With this layer, switch to other TCPIP stack is possible and easy in esp-idf.
 *
 * If users want to use other TCPIP stack, all those functions should be implemented
 * by using the specific APIs of that stack. The macros in CONFIG_TCPIP_LWIP should be
 * re-defined.
 *
 * tcpip_adapter_init should be called in the start of app_main for only once.
 *
 * Currently most adapter APIs are called in event_default_handlers.c.
 *
 * We recommend users only use set/get IP APIs, DHCP server/client APIs,
 * get free station list APIs in application side. Other APIs are used in esp-idf internal,
 * otherwise the state maybe wrong.
 *
 * TODO: ipv6 support will be added
 */
#include "esp_at_lib.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "tcpip_adapter.h"
#include "esp_hal_log.h"


/**
 * @brief  Initialize tcpip adapter
 *
 * This will initialize TCPIP stack inside.
 */
void tcpip_adapter_init(void) {
	log_v(" +++\r\n");
	return;
}

/**
 * @brief  Get interface's IP information
 *
 * There has an IP information copy in adapter library, if interface is up, get IP information from
 * interface, otherwise get from copy.
 *
 * @param[in]   tcpip_if: the interface which we want to get IP information
 * @param[out]  ip_info: If successful, IP information will be returned in this argument.
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_get_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t *ip_info) {
	espr_t r;

	if (tcpip_if == TCPIP_ADAPTER_IF_STA) {
		r = esp_sta_getip((esp_ip_t*)&ip_info->ip,
		                  (esp_ip_t*)&ip_info->gw,
		                  (esp_ip_t*)&ip_info->netmask, NULL, NULL, 1);
	} else
	if (tcpip_if == TCPIP_ADAPTER_IF_AP) {
		r = esp_ap_getip((esp_ip_t*)&ip_info->ip,
		                  (esp_ip_t*)&ip_info->gw,
		                  (esp_ip_t*)&ip_info->netmask, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

/**
 * @brief  Set interface's IP information
 *
 * There has an IP information copy in adapter library, if interface is up, also set interface's IP.
 * DHCP client/server should be stopped before set new IP information.
 *
 * This function is mainly used for setting static IP.
 *
 * @param[in]  tcpip_if: the interface which we want to set IP information
 * @param[in]  ip_info: store the IP information which needs to be set to specified interface
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_set_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t *ip_info) {
	espr_t r;

	if (tcpip_if == TCPIP_ADAPTER_IF_STA) {
		r = esp_sta_setip((esp_ip_t*)&ip_info->ip,
		                  (esp_ip_t*)&ip_info->gw,
		                  (esp_ip_t*)&ip_info->netmask, NULL, NULL, 1);
	} else
	if (tcpip_if == TCPIP_ADAPTER_IF_AP) {
		r = esp_ap_setip((esp_ip_t*)&ip_info->ip,
		                  (esp_ip_t*)&ip_info->gw,
		                  (esp_ip_t*)&ip_info->netmask, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

/**
 * @brief  create interface's linklocal IPv6 information
 *
 * @note this function will create a linklocal IPv6 address about input interface,
 *       if this address status changed to preferred, will call event call back ,
 *       notify user linklocal IPv6 address has been verified
 *
 * @param[in]  tcpip_if: the interface which we want to set IP information
 *
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_create_ip6_linklocal(tcpip_adapter_if_t tcpip_if) {
	tcpip_if = tcpip_if;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
 * @brief  get interface's linkloacl IPv6 information
 *
 * There has an IPv6 information copy in adapter library, if interface is up,and IPv6 info
 * is preferred,it will get IPv6 linklocal IP successfully
 *
 * @param[in]  tcpip_if: the interface which we want to set IP information
 * @param[in]  if_ip6: If successful, IPv6 information will be returned in this argument.
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_get_ip6_linklocal(tcpip_adapter_if_t tcpip_if, ip6_addr_t *if_ip6) {
	tcpip_if = tcpip_if;
	if_ip6 = if_ip6;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
 * @brief  Set or Get DHCP server's option
 *
 * @param[in]  opt_op: option operate type, 1 for SET, 2 for GET.
 * @param[in]  opt_id: option index, 32 for ROUTER, 50 for IP POLL, 51 for LEASE TIME, 52 for REQUEST TIME
 * @param[in]  opt_val: option parameter
 * @param[in]  opt_len: option length
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED
 *         ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED
 */
esp_err_t tcpip_adapter_dhcps_option(tcpip_adapter_option_mode_t opt_op, tcpip_adapter_option_id_t opt_id, void *opt_val, uint32_t opt_len) {
	opt_op = opt_op;
	opt_id = opt_id;
	opt_val = opt_val;
	opt_len = opt_len;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
 * @brief  Start DHCP server
 *
 * @note   Currently DHCP server is bind to softAP interface.
 *
 * @param[in]  tcpip_if: the interface which we will start DHCP server
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED
 */
esp_err_t tcpip_adapter_dhcps_start(tcpip_adapter_if_t tcpip_if) {
	espr_t r;

	log_v(" +++\r\n");

	if (tcpip_if == TCPIP_ADAPTER_IF_AP) {
		r = esp_dhcp_configure(0, 1, 1, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

/**
 * @brief  Stop DHCP server
 *
 * @note   Currently DHCP server is bind to softAP interface.
 *
 * @param[in]  tcpip_if: the interface which we will stop DHCP server
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPED
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY
 */
esp_err_t tcpip_adapter_dhcps_stop(tcpip_adapter_if_t tcpip_if) {
	espr_t r;

	log_v(" +++\r\n");

	if (tcpip_if == TCPIP_ADAPTER_IF_AP) {
		r = esp_dhcp_configure(0, 1, 0, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

/**
 * @brief  Start DHCP client
 *
 * @note   Currently DHCP client is bind to station interface.
 *
 * @param[in]  tcpip_if: the interface which we will start DHCP client
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED
 *         ESP_ERR_TCPIP_ADAPTER_DHCPC_START_FAILED
 */
esp_err_t tcpip_adapter_dhcpc_start(tcpip_adapter_if_t tcpip_if) {
	espr_t r;

	log_v(" +++\r\n");

	if (tcpip_if == TCPIP_ADAPTER_IF_STA) {
		r = esp_dhcp_configure(1, 0, 1, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

/**
 * @brief  Stop DHCP client
 *
 * @note   Currently DHCP client is bind to station interface.
 *
 * @param[in]  tcpip_if: the interface which we will stop DHCP client
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPED
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY
 */
esp_err_t tcpip_adapter_dhcpc_stop(tcpip_adapter_if_t tcpip_if) {
	espr_t r;

	log_v(" +++\r\n");

	if (tcpip_if == TCPIP_ADAPTER_IF_STA) {
		r = esp_dhcp_configure(1, 0, 0, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

#define TCPIP_HOSTNAME_MAX_SIZE    32
/**
 * @brief  Set the hostname to the interface
 *
 * @param[in]   tcpip_if: the interface which we will set the hostname
 * @param[in]   hostname: the host name for set the interface, the max length of hostname is 32 bytes
 *
 * @return ESP_OK:success
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY:interface status error
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS:parameter error
 */
esp_err_t tcpip_adapter_set_hostname(tcpip_adapter_if_t tcpip_if, const char *hostname) {
	tcpip_if = tcpip_if;
	hostname = hostname;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
 * @brief  Get the hostname from the interface
 *
 * @param[in]   tcpip_if: the interface which we will get the hostname
 * @param[in]   hostname: the host name from the interface
 *
 * @return ESP_OK:success
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY:interface status error
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS:parameter error
 */
esp_err_t tcpip_adapter_get_hostname(tcpip_adapter_if_t tcpip_if, const char **hostname) {
	tcpip_if = tcpip_if;
	hostname = hostname;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
 * @brief  Get the LwIP netif* that is assigned to the interface
 *
 * @param[in]   tcpip_if: the interface which we will get the hostname
 * @param[out]  void ** netif: pointer to fill the resulting interface
 *
 * @return ESP_OK:success
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY:interface status error
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS:parameter error
 */
esp_err_t tcpip_adapter_get_netif(tcpip_adapter_if_t tcpip_if, void ** netif) {
	tcpip_if = tcpip_if;
	netif = netif;
	log_v(" +++\r\n");
	return ESP_OK;
}

/**
 * @brief  Test if supplied interface is up or down
 *
 * @param[in]   tcpip_if: the interface which we will get the hostname
 *
 * @return  true:  tcpip_if is UP
 *          false: tcpip_if id DOWN
 */
bool tcpip_adapter_is_netif_up(tcpip_adapter_if_t tcpip_if) {
	tcpip_if = tcpip_if;
	log_v(" +++\r\n");
	return true;
}
