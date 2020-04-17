/*
 *  The MIT License (MIT)
 *  Copyright (C) 2019  Seeed Technology Co.,Ltd.
 */
#include <stdbool.h>
#include "esp_at_lib.h"
#include <lwip/dns.h>
#include "esp_err.h"
#include "atu_netdb.h"
#include "esp_system.h"
#include "esp_hal_log.h"

err_t dns_gethostbyname(const char *hostname, ip_addr_t *addr,
                        dns_found_callback found, void *callback_arg) {
	espr_t r;

	log_v(" +++\r\n");

	r = esp_dns_gethostbyname(hostname, (esp_ip_t*)addr, NULL, NULL, true);
	if (r != espOK) {
		return -1;
	}
	if (found) {
		found(hostname, addr, callback_arg);
	}
	return ESPR_TO_ESP_ERR(r);
}

struct hostent *atu_gethostbyname(const char *name) {
	static struct {
		struct hostent host[1];
		char* addr_list;
		uint32_t addr[2];
	} h = { {0}, (char*)&h.addr[0], {0} };
	ip_addr_t addr;
	err_t r;

	r = dns_gethostbyname(name, &addr, NULL, NULL);
	if (r != ESP_OK) {
		return NULL;
	}
	h.addr[0] = addr.u_addr.ip4.addr;
	h.host->h_length = sizeof(h);
	h.host->h_addr_list = &h.addr_list;
	return h.host;
}

void dns_setserver(u8_t numdns, const ip_addr_t *dnsserver) {
	u8_t other = !numdns;
	uint32_t dns[2];
	char dns1[18];
	char dns2[18];

	log_v(" +++\r\n");

	if (numdns > 1) return;

	dns[other] = dns_getserver(other).u_addr.ip4.addr;
	dns[numdns] = dnsserver->u_addr.ip4.addr;

	esp_ip_2_str(dns1, *(esp_ip_t*)&dns[0]);
	esp_ip_2_str(dns2, *(esp_ip_t*)&dns[1]);

	esp_dns_set_config(1, dns1, dns2, NULL, NULL, true);
	return;
}

ip_addr_t dns_getserver(u8_t numdns) {
	ip_addr_t ip_zero = {0};
	esp_ip_t dns[2];
	espr_t r;

	IP_ADDR4(&ip_zero, 0, 0, 0, 0);
	if (numdns > 1) return ip_zero;

	r = esp_dns_get_config(&dns[0], &dns[1], NULL, NULL, true);
	if (r != espOK) {
		return ip_zero;
	}

	// avoid type align
	memcpy(&ip_zero.u_addr.ip4.addr, &dns[numdns], sizeof(esp_ip_t));
	return ip_zero;
}

esp_err_t esp_read_mac(uint8_t* mac, esp_mac_type_t type) {
	espr_t r = espOK;

	log_v(" +++\r\n");

	switch (type) {
	case ESP_MAC_WIFI_STA:
		r = esp_sta_getmac((esp_mac_t*)mac, NULL, NULL, 1);
		break;

	case ESP_MAC_WIFI_SOFTAP:
		r = esp_ap_getmac((esp_mac_t*)mac, NULL, NULL, 1);
		break;

	case ESP_MAC_BT:
		// TODO:
		break;

	default:
		break;
	}
	return ESPR_TO_ESP_ERR(r);
}

// esp_ip_t is a big endian structure
// identical with ip_addr_t
int esp_ip_2_str(char* buf, esp_ip_t ip) {
	return sprintf(buf, "%d.%d.%d.%d",
	               ip.ip[0], ip.ip[1], ip.ip[2], ip.ip[3]);
}
