// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//  Copyright (C) 2019-2020  Seeed Technology Co.,Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Seeed_atUnified.h"
#include "mdns.h"

static char* mdns_hostname;

/**
 * @brief  Initialize mDNS on given interface
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_ERR_INVALID_ARG when bad tcpip_if is given
 *     - ESP_ERR_INVALID_STATE when the network returned error
 *     - ESP_ERR_NO_MEM on memory error
 *     - ESP_ERR_WIFI_NOT_INIT when WiFi is not initialized by eps_wifi_init
 */
esp_err_t mdns_init() {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Stop and free mDNS server
 *
 */
void mdns_free() {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Set the hostname for mDNS server
 *         required if you want to advertise services
 *
 * @param  hostname     Hostname to set
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NO_MEM memory error
 */
esp_err_t mdns_hostname_set(const char * hostname) {
	char* name = mdns_hostname;

	printf("%s() L%d\r\n", __func__, __LINE__);

	if (name) {
		free(name);
	}
	name = malloc(strlen(hostname) + 1);
	if (!name) {
		return ESP_ERR_NO_MEM;
	}
	strcpy(name, hostname);

	mdns_hostname = name;

	// a default hostname
	// TODO: remove
	esp_mdns_configure(1, name, "tcp", 80, NULL, NULL, true);
	return ESP_OK;
}
/**
 * @brief  Set the default instance name for mDNS server
 *
 * @param  instance_name     Instance name to set
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NO_MEM memory error
 */
esp_err_t mdns_instance_name_set(const char * instance_name) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Add service to mDNS server
 *
 * @param  instance_name    instance name to set. If NULL,
 *                          global instance name or hostname will be used
 * @param  service_type     service type (_http, _ftp, etc)
 * @param  proto            service protocol (_tcp, _udp)
 * @param  port             service port
 * @param  num_items        number of items in TXT data
 * @param  txt              string array of TXT data (eg. {{"var","val"},{"other","2"}})
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NO_MEM memory error
 */
esp_err_t mdns_service_add(const char * instance_name, const char * service_type, const char * proto, uint16_t port, mdns_txt_item_t txt[], size_t num_items) {
	const char* name;
	espr_t r;

	printf("%s() +++ L%d\r\r\n", __func__, __LINE__);

	name = instance_name? instance_name: mdns_hostname;
	r = esp_mdns_configure(1, name, proto, port, NULL, NULL, true);
	if (r != espOK) {
		printf("esp_mdns_configure() return error %d\r\r\n",  r);
	}
	return ESPR_TO_ESP_ERR(r);
}
/**
 * @brief  Remove service from mDNS server
 *
 * @param  service_type service type (_http, _ftp, etc)
 * @param  proto        service protocol (_tcp, _udp)
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NOT_FOUND Service not found
 *     - ESP_FAIL unknown error
 */
esp_err_t mdns_service_remove(const char * service_type, const char * proto) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Set instance name for service
 *
 * @param  service_type     service type (_http, _ftp, etc)
 * @param  proto            service protocol (_tcp, _udp)
 * @param  instance_name    instance name to set
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NOT_FOUND Service not found
 *     - ESP_ERR_NO_MEM memory error
 */
esp_err_t mdns_service_instance_name_set(const char * service_type, const char * proto, const char * instance_name) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Set service port
 *
 * @param  service_type service type (_http, _ftp, etc)
 * @param  proto        service protocol (_tcp, _udp)
 * @param  port         service port
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NOT_FOUND Service not found
 */
esp_err_t mdns_service_port_set(const char * service_type, const char * proto, uint16_t port) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Replace all TXT items for service
 *
 * @param  service_type service type (_http, _ftp, etc)
 * @param  proto        service protocol (_tcp, _udp)
 * @param  num_items    number of items in TXT data
 * @param  txt          array of TXT data (eg. {{"var","val"},{"other","2"}})
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NOT_FOUND Service not found
 *     - ESP_ERR_NO_MEM memory error
 */
esp_err_t mdns_service_txt_set(const char * service_type, const char * proto, mdns_txt_item_t txt[], uint8_t num_items) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Set/Add TXT item for service TXT record
 *
 * @param  service_type service type (_http, _ftp, etc)
 * @param  proto        service protocol (_tcp, _udp)
 * @param  key          the key that you want to add/update
 * @param  value        the new value of the key
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NOT_FOUND Service not found
 *     - ESP_ERR_NO_MEM memory error
 */
esp_err_t mdns_service_txt_item_set(const char * service_type, const char * proto, const char * key, const char * value) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Remove TXT item for service TXT record
 *
 * @param  service_type service type (_http, _ftp, etc)
 * @param  proto        service protocol (_tcp, _udp)
 * @param  key          the key that you want to remove
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_ERR_NOT_FOUND Service not found
 *     - ESP_ERR_NO_MEM memory error
 */
esp_err_t mdns_service_txt_item_remove(const char * service_type, const char * proto, const char * key) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Remove and free all services from mDNS server
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t mdns_service_remove_all() {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Query mDNS for host or service
 *         All following query methods are derived from this one
 *
 * @param  name         service instance or host name (NULL for PTR queries)
 * @param  service_type service type (_http, _arduino, _ftp etc.) (NULL for host queries)
 * @param  proto        service protocol (_tcp, _udp, etc.) (NULL for host queries)
 * @param  type         type of query (MDNS_TYPE_*)
 * @param  timeout      time in milliseconds to wait for answers.
 * @param  max_results  maximum results to be collected
 * @param  results      pointer to the results of the query
 *                      results must be freed using mdns_query_results_free below
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_STATE  mDNS is not running
 *     - ESP_ERR_NO_MEM         memory error
 *     - ESP_ERR_INVALID_ARG    timeout was not given
 */
esp_err_t mdns_query(const char * name, const char * service_type, const char * proto, uint16_t type, uint32_t timeout, size_t max_results, mdns_result_t ** results) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Free query results
 *
 * @param  results      linked list of results to be freed
 */
void mdns_query_results_free(mdns_result_t * results) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Query mDNS for service
 *
 * @param  service_type service type (_http, _arduino, _ftp etc.)
 * @param  proto        service protocol (_tcp, _udp, etc.)
 * @param  timeout      time in milliseconds to wait for answer.
 * @param  max_results  maximum results to be collected
 * @param  results      pointer to the results of the query
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_STATE  mDNS is not running
 *     - ESP_ERR_NO_MEM         memory error
 *     - ESP_ERR_INVALID_ARG    parameter error
 */
esp_err_t mdns_query_ptr(const char * service_type, const char * proto, uint32_t timeout, size_t max_results, mdns_result_t ** results) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Query mDNS for SRV record
 *
 * @param  instance_name    service instance name
 * @param  service_type     service type (_http, _arduino, _ftp etc.)
 * @param  proto            service protocol (_tcp, _udp, etc.)
 * @param  timeout          time in milliseconds to wait for answer.
 * @param  result           pointer to the result of the query
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_STATE  mDNS is not running
 *     - ESP_ERR_NO_MEM         memory error
 *     - ESP_ERR_INVALID_ARG    parameter error
 */
esp_err_t mdns_query_srv(const char * instance_name, const char * service_type, const char * proto, uint32_t timeout, mdns_result_t ** result) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Query mDNS for TXT record
 *
 * @param  instance_name    service instance name
 * @param  service_type     service type (_http, _arduino, _ftp etc.)
 * @param  proto            service protocol (_tcp, _udp, etc.)
 * @param  timeout          time in milliseconds to wait for answer.
 * @param  result           pointer to the result of the query
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_STATE  mDNS is not running
 *     - ESP_ERR_NO_MEM         memory error
 *     - ESP_ERR_INVALID_ARG    parameter error
 */
esp_err_t mdns_query_txt(const char * instance_name, const char * service_type, const char * proto, uint32_t timeout, mdns_result_t ** result) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Query mDNS for A record
 *
 * @param  host_name    host name to look for
 * @param  timeout      time in milliseconds to wait for answer.
 * @param  addr         pointer to the resulting IP4 address
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_STATE  mDNS is not running
 *     - ESP_ERR_NO_MEM         memory error
 *     - ESP_ERR_INVALID_ARG    parameter error
 */
esp_err_t mdns_query_a(const char * host_name, uint32_t timeout, ip4_addr_t * addr) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief  Query mDNS for A record
 *
 * @param  host_name    host name to look for
 * @param  timeout      time in milliseconds to wait for answer. If 0, max_results needs to be defined
 * @param  addr         pointer to the resulting IP6 address
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_STATE  mDNS is not running
 *     - ESP_ERR_NO_MEM         memory error
 *     - ESP_ERR_INVALID_ARG    parameter error
 */
esp_err_t mdns_query_aaaa(const char * host_name, uint32_t timeout, ip6_addr_t * addr) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
/**
 * @brief   System event handler
 *          This method controls the service state on all active interfaces and applications are required
 *          to call it from the system event handler for normal operation of mDNS service.
 *
 * @param  ctx          The system event context
 * @param  event        The system event
 */
esp_err_t mdns_handle_system_event(void *ctx, system_event_t *event) {
	printf("%s() L%d\r\n", __func__, __LINE__);
	return ESP_OK;
}
