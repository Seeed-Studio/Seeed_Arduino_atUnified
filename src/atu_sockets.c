/*
 *  The MIT License (MIT)
 *  Copyright (C) 2019  Seeed Technology Co.,Ltd.
 */
#include <stdbool.h>
#include <errno.h>
#include "esp_at_lib.h"
#include "atu_netdb.h"
#include "atu_sockets.h"
#include "esp_hal_log.h"

#ifndef DEBUG
#undef log_printf
#define log_printf(...)
#endif

typedef struct netconn_fd_s {
	esp_netconn_p conn;

	esp_pbuf_p pbuf;
	size_t bytes_left;

	esp_sys_mutex_t fd_lock;

	esp_netconn_type_t nc_type;
	uint16_t keep_alive;

	uint32_t file_flag;
} netconn_fd_t;
static netconn_fd_t* netconn_maps;
static esp_sys_mutex_t maps_lock;
static int maps_size;

netconn_fd_t* atu_fd2conn(int s) {
	if (s < 0) {
		return NULL;
	}
	if (s >= maps_size) {
		// dump_tasks();
		return NULL;
	}
	return &netconn_maps[s];
}

int atu_conn2fd(esp_netconn_p netconn) {
	int i;

	/* lock */
	esp_sys_mutex_lock(&maps_lock);
	for (i = 0; i < maps_size; i++) {
		if (netconn_maps[i].conn == netconn) {
			break;
		}
	}
	if (i >= maps_size) {
		for (i = 0; i < maps_size; i++) {
			if (!netconn_maps[i].conn) {
				netconn_maps[i].conn = netconn;
				break;
			}
		}
	}
	if (i >= maps_size) {
		netconn_fd_t* new_maps;

		new_maps = calloc(sizeof(netconn_fd_t), maps_size + 16);
		if (!new_maps) {
			/* unlock */
			esp_sys_mutex_unlock(&maps_lock);
			return -1;
		}

		if (netconn_maps) {
			memcpy(new_maps, netconn_maps, maps_size * sizeof(netconn_fd_t));
		}
		maps_size += 16;
		netconn_maps = new_maps;

		netconn_maps[i].conn = netconn;
	}
	/* unlock */
	esp_sys_mutex_unlock(&maps_lock);
	return i;
}

static void* get_netconn_arg(esp_netconn_p nc) {
	esp_conn_p econ;
	econ = esp_netconn_get_conn(nc);

	return esp_conn_get_arg(econ);
}

int atu_socket(int domain, int type, int protocol) {
	esp_netconn_type_t nc_type;
	esp_netconn_p nc;
	esp_conn_p econ;
	netconn_fd_t* nf;
	int fd;

	domain = domain;
	protocol = protocol;

	log_v(" +++\r\n");

	switch (type) {
	case SOCK_STREAM:
		nc_type = ESP_NETCONN_TYPE_TCP; break;
	case SOCK_DGRAM:
		nc_type = ESP_NETCONN_TYPE_UDP; break;
	default:
		return -1;
	}

	if (!esp_sys_mutex_isvalid(&maps_lock)) {
		esp_sys_mutex_create(&maps_lock);
	}

	nc = esp_netconn_new(nc_type);
	if (!nc) {
		return -2;
	}

	fd = atu_conn2fd(nc);

	econ = esp_netconn_get_conn(nc);
	nf = atu_fd2conn(fd);
	nf->nc_type = nc_type;
	if (econ) {
		esp_conn_set_arg(econ, nf);
	} else {
		log_d(" netconn_fd %d has no valid conn\r\n", fd);
	}

	return fd;
}

int atu_accept_r(int s, struct sockaddr *addr, socklen_t *addrlen) {
	netconn_fd_t* nf;
	esp_netconn_p client;
	espr_t r;

	log_d(" socket = %d\r\n", s);
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}
	r = esp_netconn_accept(nf->conn, &client);
	if (r != espOK) {
		return -1;
	}
	return atu_conn2fd(client);
}

int atu_bind_r(int s, const struct sockaddr *name, socklen_t namelen) {
	struct sockaddr_in* addr = (struct sockaddr_in*)name;
	netconn_fd_t* nf;
	espr_t r;

	log_d(" socket = %d\r\n", s);
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}

	r = esp_netconn_bind(nf->conn, ntohs(addr->sin_port));
	if (r != espOK) {
		log_e("NETCONN bind Fail error = %d\r\n", r);
		return -1;
	}
	return 0;
}

int atu_getpeername_r (int s, struct sockaddr *name, socklen_t *namelen) {
	struct sockaddr_in* addr = (struct sockaddr_in*)name;
	netconn_fd_t* nf;
	uint32_t remote;
	esp_port_t port;
	espr_t r;

	log_d(" socket = %d\r\n", s);
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}

	r = esp_conn_get_remote_ip(esp_netconn_get_conn(nf->conn), (esp_ip_t*)&remote);
	if (!r) {
		log_e("Get peer name Fail error = %d\r\n", r);
		return -1;
	}

	addr->sin_addr.s_addr = remote;
	addr->sin_port = esp_conn_get_remote_port(esp_netconn_get_conn(nf->conn));
	return 0;
}

int atu_getsockopt_r (int s, int level, int optname, void *optval, socklen_t *optlen) {
	struct timeval* tv;
	netconn_fd_t* nf;
	espr_t r;

	level = level;

	log_d("%s(opt 0x%X) +++ L%d\r\n", __func__, optname, __LINE__);
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		log_e("%s() Invalid connection\r\n", __func__);
		return -1;
	}

	switch (optname) {
	case SO_ERROR:
		*(int*)optval = 0;
		break;

	case SO_RCVTIMEO:
		break;

	case SO_SNDTIMEO:
	case SO_KEEPALIVE:
	case SO_DEBUG:
		break;

	default:
		log_e("getsockopt_r() unhandled option %d \r\n", optname);
		errno = EINVAL;
		return -1;
		break;
	}
	return 0;
}

int atu_setsockopt_r (int s, int level, int optname, const void *optval, socklen_t optlen) {
	struct timeval* tv;
	netconn_fd_t* nf;
	int used = 0;
	espr_t r;

	level = level;

	log_v(" +++\r\n");
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}

	if (level == SOL_SOCKET) {
		switch (optname) {
		case SO_RCVTIMEO:
			if (optlen == sizeof(struct timeval)) {
				tv = (struct timeval*)optval;
				esp_netconn_set_receive_timeout(nf->conn, tv->tv_sec * 1000 + tv->tv_usec / 1000);
				used = 1;
			}
			else if (optlen == sizeof(int)) {
				esp_netconn_set_receive_timeout(nf->conn, *(int*)optval);
				used = 1;
			}
			break;

		case SO_KEEPALIVE:
			if (optlen != sizeof(int)) break;
			nf->keep_alive = *(const int*)optval;
			used = 1;
			break;

		case SO_SNDTIMEO:
		case SO_DEBUG:
		case SO_REUSEADDR:
			used = 1;
			break;
		}
	} else if (level == IPPROTO_TCP) {
		switch (optname) {
		case TCP_NODELAY:
			used = 1;
			break;
		}
	} else if (level == IPPROTO_IP) {
		switch (optname) {
		case IP_ADD_MEMBERSHIP:
		case IP_DROP_MEMBERSHIP:
			used = 1;
			break;
		}
	}

	if (!used) {
		log_e("setsockopt_r() unhandled option %d \r\n", optname);
		errno = EINVAL;
		return -1;
	}
	return 0;
}

int atu_close_r(int s) {
	netconn_fd_t* nf;

	log_v(" +++\r\n");
	nf = atu_fd2conn(s);

	if (!nf || !nf->conn) {
		return -1;
	}
	if (nf->conn) {
		esp_netconn_close(nf->conn);
		esp_netconn_delete(nf->conn);
		nf->conn = NULL;
	}
	if (nf->pbuf) {
		esp_pbuf_free(nf->pbuf);
		nf->pbuf = NULL;
	}
	nf->bytes_left = 0;
	return 0;
}

int atu_connect_r(int s, const struct sockaddr *name, socklen_t namelen) {
	struct sockaddr_in* addr = (struct sockaddr_in*)name;
	char ip_str[16];
	netconn_fd_t* nf;
	espr_t r;

	namelen = namelen;

	log_v(" +++\r\n");
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}

	esp_ip_2_str(ip_str, *(esp_ip_t*)&addr->sin_addr.s_addr);

	log_i("%s() connect to ip %s\r\n", __func__, ip_str);
	r = esp_netconn_connect_ex(nf->conn, ip_str, ntohs(addr->sin_port), nf->keep_alive, NULL, 0, 0);
	if (r != espOK) {
		return -1;
	}
	return 0;
}

int atu_listen_r(int s, int backlog) {
	netconn_fd_t* nf;
	espr_t r;

	backlog = backlog;

	log_v(" +++\r\n");
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}
	r = esp_netconn_listen(nf->conn);
	if (r != espOK) {
		return -1;
	}
	return 0;
}

int atu_recv_r(int s, void *mem, size_t len, int flags) {
	esp_conn_p econ;
	netconn_fd_t* nf;
	espr_t r;
	size_t plen, sz;
	char* data;

	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}

	econ = esp_netconn_get_conn(nf->conn);

	if (!nf->pbuf || !nf->bytes_left) {
		bool nowait;
		uint32_t last_to;

		nowait = (flags & MSG_DONTWAIT) || !len || (nf->file_flag & O_NONBLOCK);
		/*
		 * get a new pbuf
		 */

		if (nowait) {
			last_to = esp_netconn_get_receive_timeout(nf->conn);
			esp_netconn_set_receive_timeout(nf->conn, -1UL);
		}

		r = esp_netconn_receive(nf->conn, &nf->pbuf);

		if (nowait) {
			esp_netconn_set_receive_timeout(nf->conn, last_to);
		}

		if (r != espOK) {
			if (nf->nc_type == ESP_NETCONN_TYPE_TCP && !esp_conn_is_active(econ)) {
				return 0;
			}
			if (r != espTIMEOUT) {
				errno = ENOENT;
				log_d("recv_r %d\r\n", r);
				return -1;
			}
		}
		if (nf->pbuf == NULL) {
			errno = EWOULDBLOCK;
			return -1;
		}

		plen = esp_pbuf_length(nf->pbuf, 0);
		data = (char*)esp_pbuf_data(nf->pbuf);
		nf->bytes_left = plen;
	} else {
		/*
		 * last time we have bytes left in static pbuf
		 */

		plen = esp_pbuf_length(nf->pbuf, 0);
		data = (char*)esp_pbuf_data(nf->pbuf) + plen - nf->bytes_left;
	}

	sz = min(nf->bytes_left, len);
	memcpy(mem, data, sz);

	nf->bytes_left -= sz;
	if (!nf->bytes_left) {
		esp_pbuf_free(nf->pbuf);
		nf->pbuf = NULL;
	}

	log_v("copied = %d, bytes_left = %d\r\n", sz, nf->bytes_left);

	return sz;
}

int atu_recvfrom_r(int s, void *mem, size_t len, int flags,
      struct sockaddr *from, socklen_t *fromlen) {
	return atu_recv_r(s, mem, len, flags);
}

int atu_send_r(int s, const void *dataptr, size_t size, int flags) {
	netconn_fd_t* nf;
	espr_t r;

	log_d("@T@\r\n");
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}
	r = esp_netconn_write(nf->conn, dataptr, size);
	if (r != espOK) {
		log_d("NETCONN write Fail error = %d\r\n", r);
		return -1;
	}
	esp_netconn_flush(nf->conn);
	return size;
}

int atu_sendto_r(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen) {
	struct sockaddr_in* addr = (struct sockaddr_in*)to;
	netconn_fd_t* nf;
	esp_ip_t remote;
	esp_conn_p econ;
	espr_t r;

	log_d("@T- socket = %d\r\n", s);

	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}

	econ = esp_netconn_get_conn(nf->conn);
	if (!esp_conn_is_active(econ)) {
		int ar;

		ar = atu_connect_r(s, to, tolen);
		if (ar < 0) {
			log_d("NETCONN connect fail %d\r\n", ar);
			return ar;
		}
	}

	*(uint32_t*)&remote = addr->sin_addr.s_addr;
	r = esp_netconn_sendto(nf->conn, &remote, ntohs(addr->sin_port), dataptr, size);
	if (r != espOK) {
		log_e("NETCONN sendto Fail error = %d\r\n", r);
		return -1;
	}
	return size;
}

int atu_ioctl_r(int s, long cmd, void *argp) {
	netconn_fd_t* nf;
	esp_conn_p econ;

	log_printf("#");
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		log_printf(")");
		return -1;
	}

	if (cmd == FIONREAD) {
		if (!nf->pbuf || !nf->bytes_left) {
			espr_t r;
			uint32_t timeout;

			/*
			 * get a new pbuf
			 */
			timeout = esp_netconn_get_receive_timeout(nf->conn);

			econ = esp_netconn_get_conn(nf->conn);
			if (1 /*!esp_conn_is_active(econ) || (nf->file_flag & O_NONBLOCK)*/) {
				esp_netconn_set_receive_timeout(nf->conn, -1UL);
			}
			r = esp_netconn_receive(nf->conn, &nf->pbuf);
			esp_netconn_set_receive_timeout(nf->conn, timeout);

			if (r != espOK) {
				if (!esp_conn_is_active(econ)) {
					errno = EBADF;
					log_printf("0)");
					return -1;
				}
				if (r != espTIMEOUT) {
					errno = ENOENT;
					log_printf("1)");
					return -1;
				}
			}
			if (nf->pbuf != NULL) {
				nf->bytes_left = esp_pbuf_length(nf->pbuf, 0);
			} else {
				nf->bytes_left = 0;
			}
		}
		if (argp) {
			*(int*)argp = nf->bytes_left;
		}
	}
	log_printf("2)");
	return 0;
}

int atu_fcntl_r(int s, int cmd, int val) {
	netconn_fd_t* nf;

	log_printf("@C@\r\n");
	nf = atu_fd2conn(s);
	if (!nf || !nf->conn) {
		return -1;
	}
	switch (cmd) {
	case F_GETFL:
		return nf->file_flag;

	case F_SETFL:
		nf->file_flag = val;
		break;

	default:
		log_e("atu_fcntl_r() unhandled cmd %d \r\n", cmd);
		errno = EINVAL;
		return -1;
	}
	return 0;
}

int atu_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout) {
	int i;
	fd_set* fds;

	nfds = nfds;
	log_printf("@S@\r\n");

	if (readfds) {
		fds = readfds;
	} else if (writefds) {
		fds = writefds;
	} else if (errorfds) {
		fds = errorfds;
	} else {
		fds = NULL;
		return 0;
	}
	for (i = 0; i < nfds; i++) {
		if (FD_ISSET(i, fds)) {
			return 1;
		}
	}
	return 0;
}

int select(int nfds, fd_set* readset, fd_set* writeset, fd_set* exceptset, struct timeval* timeout) {
    return atu_select(nfds, readset, writeset, exceptset, timeout);
}

u16_t atu_htons(u16_t x) {
	u16_t y = ((x << 8) | (x >> 8));
	return y;
}

u32_t atu_htonl(u32_t x) {
	union {
		u32_t y;
		char c[4];
	} u;

	u.c[0] = ((char*)&x)[3];
	u.c[1] = ((char*)&x)[2];
	u.c[2] = ((char*)&x)[1];
	u.c[3] = ((char*)&x)[0];
	return u.y;
}
