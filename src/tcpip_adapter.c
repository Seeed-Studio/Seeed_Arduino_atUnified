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

/**
 * @brief  Initialize tcpip adapter
 *
 * This will initialize TCPIP stack inside.
 */
void tcpip_adapter_init(void) {
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return;
}

/**
 * @brief  Start the ethernet interface with specific MAC and IP
 *
 * @param[in]  mac: set MAC address of this interface
 * @param[in]  ip_info: set IP address of this interface
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_NO_MEM
 */
esp_err_t tcpip_adapter_eth_start(uint8_t *mac, tcpip_adapter_ip_info_t *ip_info) {
	mac = mac;
	ip_info = ip_info;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Start the Wi-Fi station interface with specific MAC and IP
 *
 * Station interface will be initialized, connect WiFi stack with TCPIP stack.
 *
 * @param[in]  mac: set MAC address of this interface
 * @param[in]  ip_info: set IP address of this interface
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_NO_MEM
 */
esp_err_t tcpip_adapter_sta_start(uint8_t *mac, tcpip_adapter_ip_info_t *ip_info) {
	mac = mac;
	ip_info = ip_info;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Start the Wi-Fi AP interface with specific MAC and IP
 *
 * softAP interface will be initialized, connect WiFi stack with TCPIP stack.
 *
 * DHCP server will be started automatically.
 *
 * @param[in]  mac: set MAC address of this interface
 * @param[in]  ip_info: set IP address of this interface
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_NO_MEM
 */
esp_err_t tcpip_adapter_ap_start(uint8_t *mac, tcpip_adapter_ip_info_t *ip_info) {
	mac = mac;
	ip_info = ip_info;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Stop an interface
 *
 * The interface will be cleanup in this API, if DHCP server/client are started, will be stopped.
 *
 * @param[in]  tcpip_if: the interface which will be started
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY
 */
esp_err_t tcpip_adapter_stop(tcpip_adapter_if_t tcpip_if) {
	tcpip_if = tcpip_if;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;}

/**
 * @brief  Bring up an interface
 *
 * Only station interface need to be brought up, since station interface will be shut down when disconnect.
 *
 * @param[in]  tcpip_if: the interface which will be up
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY
 */
esp_err_t tcpip_adapter_up(tcpip_adapter_if_t tcpip_if) {
	tcpip_if = tcpip_if;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Shut down an interface
 *
 * Only station interface need to be shut down, since station interface will be brought up when connect.
 *
 * @param[in]  tcpip_if: the interface which will be down
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY
 */
esp_err_t tcpip_adapter_down(tcpip_adapter_if_t tcpip_if) {
	printf("%s() +++ L%d\r\n", __func__, __LINE__);

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
 * @brief  Set DNS Server's information
 *
 * There has an DNS Server information copy in adapter library, set DNS Server for appointed interface and type.
 *
 * 1.In station mode, if dhcp client is enabled, then only the fallback DNS server can be set(TCPIP_ADAPTER_DNS_FALLBACK).
 *   Fallback DNS server is only used if no DNS servers are set via DHCP.
 *   If dhcp client is disabled, then need to set main/backup dns server(TCPIP_ADAPTER_DNS_MAIN, TCPIP_ADAPTER_DNS_BACKUP).
 * 
 * 2.In soft-AP mode, the DNS Server's main dns server offered to the station is the IP address of soft-AP, 
 *   if the application don't want to use the IP address of soft-AP, they can set the main dns server.
 *
 * This function is mainly used for setting static or Fallback DNS Server.
 *
 * @param[in]  tcpip_if: the interface which we want to set DNS Server information
 * @param[in]  type: the type of DNS Server,including TCPIP_ADAPTER_DNS_MAIN, TCPIP_ADAPTER_DNS_BACKUP, TCPIP_ADAPTER_DNS_FALLBACK
 * @param[in]  dns:  the DNS Server address to be set
 * 
 * @return 
 *      - ESP_OK on success
 *      - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS invalid params
 */
esp_err_t tcpip_adapter_set_dns_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_dns_type_t type, tcpip_adapter_dns_info_t *dns) {
	tcpip_if = tcpip_if;
	type = type;
	dns = dns;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Get DNS Server's information
 *
 * When set the DNS Server information successfully, can get the DNS Server's information via the appointed tcpip_if and type 
 *
 * This function is mainly used for getting DNS Server information.
 *
 * @param[in]  tcpip_if: the interface which we want to get DNS Server information
 * @param[in]  type: the type of DNS Server,including TCPIP_ADAPTER_DNS_MAIN, TCPIP_ADAPTER_DNS_BACKUP, TCPIP_ADAPTER_DNS_FALLBACK
 * @param[in]  dns:  the DNS Server address to be get
 * 
 * @return 
 *      - ESP_OK on success
 *      - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS invalid params
 */
esp_err_t tcpip_adapter_get_dns_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_dns_type_t type, tcpip_adapter_dns_info_t *dns) {
	tcpip_if = tcpip_if;
	type = type;
	dns = dns;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Get interface's old IP information
 *
 * When the interface successfully gets a valid IP from DHCP server or static configured, a copy of 
 * the IP information is set to the old IP information. When IP lost timer expires, the old IP 
 * information is reset to 0.
 *
 * @param[in]   tcpip_if: the interface which we want to get old IP information
 * @param[out]  ip_info: If successful, IP information will be returned in this argument.
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_get_old_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t *ip_info) {
	tcpip_if = tcpip_if;
	ip_info = ip_info;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Set interface's old IP information
 *
 * When the interface successfully gets a valid IP from DHCP server or static configured, a copy of 
 * the IP information is set to the old IP information. When IP lost timer expires, the old IP 
 * information is reset to 0.
 *
 * @param[in]  tcpip_if: the interface which we want to set old IP information
 * @param[in]  ip_info: store the IP information which needs to be set to specified interface
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_set_old_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t *ip_info) {
	tcpip_if = tcpip_if;
	ip_info = ip_info;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
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
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
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
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

#if 0
esp_err_t tcpip_adapter_get_mac(tcpip_adapter_if_t tcpip_if, uint8_t *mac) {
	printf("%s() +++ L%d\r\n", __func__, __LINE__);

}

esp_err_t tcpip_adapter_set_mac(tcpip_adapter_if_t tcpip_if, uint8_t *mac) {
	printf("%s() +++ L%d\r\n", __func__, __LINE__);

}
#endif

/**
 * @brief  Get DHCP server's status
 *
 * @param[in]   tcpip_if: the interface which we will get status of DHCP server
 * @param[out]  status: If successful, the status of DHCP server will be return in this argument.
 *
 * @return ESP_OK
 */
esp_err_t tcpip_adapter_dhcps_get_status(tcpip_adapter_if_t tcpip_if, tcpip_adapter_dhcp_status_t *status) {\
	tcpip_if = tcpip_if;
	status = status;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
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
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
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

	printf("%s() +++ L%d\r\n", __func__, __LINE__);

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

	printf("%s() +++ L%d\r\n", __func__, __LINE__);

	if (tcpip_if == TCPIP_ADAPTER_IF_AP) {
		r = esp_dhcp_configure(0, 1, 0, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

/**
 * @brief  Get DHCP client status
 *
 * @param[in]  tcpip_if: the interface which we will get status of DHCP client
 * @param[out]  status: If successful, the status of DHCP client will be return in this argument.
 *
 * @return ESP_OK
 */
esp_err_t tcpip_adapter_dhcpc_get_status(tcpip_adapter_if_t tcpip_if, tcpip_adapter_dhcp_status_t *status) {
	tcpip_if = tcpip_if;
	status = status;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Set or Get DHCP client's option
 *
 * @note   This function is not implement now.
 *
 * @param[in]  opt_op: option operate type, 1 for SET, 2 for GET.
 * @param[in]  opt_id: option index, 32 for ROUTER, 50 for IP POLL, 51 for LEASE TIME, 52 for REQUEST TIME
 * @param[in]  opt_val: option parameter
 * @param[in]  opt_len: option length
 *
 * @return ESP_OK
 */
esp_err_t tcpip_adapter_dhcpc_option(tcpip_adapter_option_mode_t opt_op, tcpip_adapter_option_id_t opt_id, void *opt_val, uint32_t opt_len) {
	opt_op = opt_op;
	opt_id = opt_id;
	opt_val = opt_val;
	opt_len = opt_len;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
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

	printf("%s() +++ L%d\r\n", __func__, __LINE__);

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

	printf("%s() +++ L%d\r\n", __func__, __LINE__);

	if (tcpip_if == TCPIP_ADAPTER_IF_STA) {
		r = esp_dhcp_configure(1, 0, 0, NULL, NULL, 1);
	} else {
		return ESP_ERR_INVALID_ARG;
	}

	return ESPR_TO_ESP_ERR(r);
}

/**
 * @brief  Get data from ethernet interface
 *
 * This function should be installed by esp_eth_init, so Ethernet packets will be forward to TCPIP stack.
 *
 * @param[in]  void *buffer: the received data point
 * @param[in]  uint16_t len: the received data length
 * @param[in]  void *eb: parameter
 *
 * @return ESP_OK
 */
esp_err_t tcpip_adapter_eth_input(void *buffer, uint16_t len, void *eb) {
	buffer =  buffer;
	len = len;
	eb = eb;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Get data from station interface
 *
 * This function should be installed by esp_wifi_reg_rxcb, so WiFi packets will be forward to TCPIP stack.
 *
 * @param[in]  void *buffer: the received data point
 * @param[in]  uint16_t len: the received data length
 * @param[in]  void *eb: parameter
 *
 * @return ESP_OK
 */
esp_err_t tcpip_adapter_sta_input(void *buffer, uint16_t len, void *eb) {
	buffer = buffer;
	len = len;
	eb = eb;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Get data from softAP interface
 *
 * This function should be installed by esp_wifi_reg_rxcb, so WiFi packets will be forward to TCPIP stack.
 *
 * @param[in]  void *buffer: the received data point
 * @param[in]  uint16_t len: the received data length
 * @param[in]  void *eb: parameter
 *
 * @return ESP_OK
 */
esp_err_t tcpip_adapter_ap_input(void *buffer, uint16_t len, void *eb) {
	buffer = buffer;
	len = len;
	eb = eb;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Get WiFi interface index
 *
 * Get WiFi interface from TCPIP interface struct pointer.
 *
 * @param[in]  void *dev: adapter interface
 *
 * @return ESP_IF_WIFI_STA
 *         ESP_IF_WIFI_AP
 *         ESP_IF_ETH
 *         ESP_IF_MAX
 */
esp_interface_t tcpip_adapter_get_esp_if(void *dev) {
	dev = dev;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}

/**
 * @brief  Get the station information list
 *
 * @param[in]   wifi_sta_list_t *wifi_sta_list: station list info
 * @param[out]  tcpip_adapter_sta_list_t *tcpip_sta_list: station list info
 *
 * @return ESP_OK
 *         ESP_ERR_TCPIP_ADAPTER_NO_MEM
 *         ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_get_sta_list(wifi_sta_list_t *wifi_sta_list, tcpip_adapter_sta_list_t *tcpip_sta_list) {
	wifi_sta_list = wifi_sta_list;
	tcpip_sta_list = tcpip_sta_list;
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return ESP_OK;

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
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
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
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
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
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
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
	printf("%s() +++ L%d\r\n", __func__, __LINE__);
	return true;
}
