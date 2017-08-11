/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <regex.h>

#include <artik_network.h>
#include <artik_loop.h>
#include <artik_module.h>
#include <artik_log.h>

#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "linux/netutils/dhcpc.h"
#include "linux/netutils/dhcpd.h"
#include "linux/netutils/netlib.h"

#include "os_network.h"

#define ROUTE_EXISTS 17

typedef struct {
	artik_list *root;
	int fd;
	bool current_online_status;
	int watch_id;
} watch_online_status_t;

typedef struct {
	watch_online_status_callback callback;
	void *user_data;
} watch_online_config;

typedef struct {
	artik_list node;
	watch_online_config config;
} watch_online_node_t;

typedef struct {
	artik_list node;
	int sockfd;
	int watch_id;
	const char *interface;
	artik_network_dhcp_server_config config;
} dhcp_handle_server;

typedef struct {
	artik_list node;
	int renew_cbk_id;
	artik_loop_module *loop_module;
	const char *interface;
} dhcp_handle_client;

static int dhcp_client_renew(artik_network_dhcp_client_handle *handle,
		const char *interface);

static watch_online_status_t *watch_online_status = NULL;

static artik_list *requested_node = NULL;

static int check_dhcp_server_config(artik_network_dhcp_server_config *config)
{
	const char *str_regex = "^(([0-9]|[1-9][0-9]"\
	"|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]"\
	"|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";

	if (str_regex) {
		regex_t preg;
		int err;

		err = regcomp(&preg, str_regex, REG_EXTENDED);

		if (err == 0) {
			int match;
			size_t nmatch = 0;
			regmatch_t *pmatch = NULL;

			nmatch = preg.re_nsub + 1;

			pmatch = malloc(sizeof(*pmatch) * nmatch);

			if (nmatch == 4) {

				if (strcmp(config->ip_addr.address, "") != 0) {
					match = regexec(&preg,
						config->ip_addr.address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong ip_addr");
						return -1;
					}
				} else {
					log_err("ip_addr not defined");
					return -1;
				}

				if (strcmp(config->netmask.address, "") != 0) {
					match = regexec(&preg,
						config->netmask.address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong netmask");
						return -1;
					}
				} else {
					log_err("netmask not defined");
					return -1;
				}

				if (strcmp(config->gw_addr.address, "") != 0) {
					match = regexec(&preg,
						config->gw_addr.address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong gw_addr");
						return -1;
					}
				} else {
					log_err("gw_addr not defined");
					return -1;
				}

				if (strcmp(config->dns_addr[0].address, "")
									!= 0) {
					match = regexec(&preg,
						config->dns_addr[0].address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong dns_addr[0]");
						return -1;
					}
				} else {
					log_err("dns_addr[0] not defined");
					return -1;
				}

				if (strcmp(config->dns_addr[1].address, "")
									!= 0) {
					match = regexec(&preg,
						config->dns_addr[1].address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong dns_addr[1]");
						return -1;
					}
				}

				if (strcmp(config->start_addr.address, "")
									!= 0) {
					match = regexec(&preg,
						config->start_addr.address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong start_addr");
						return -1;
					}
				} else {
					log_err("start_addr not defined");
					return -1;
				}
			}
		}
	}

	return 0;
}

static int check_network_config(artik_network_config *config)
{
	const char *str_regex = "^(([0-9]|[1-9][0-9]"\
	"|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]"\
	"|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";

	if (str_regex) {
		regex_t preg;
		int err;

		err = regcomp(&preg, str_regex, REG_EXTENDED);

		if (err == 0) {
			int match;
			size_t nmatch = 0;
			regmatch_t *pmatch = NULL;

			nmatch = preg.re_nsub + 1;

			pmatch = malloc(sizeof(*pmatch) * nmatch);

			if (nmatch == 4) {

				if (strcmp(config->ip_addr.address, "") != 0) {
					match = regexec(&preg,
						config->ip_addr.address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong ip_addr");
						return -1;
					}
				} else {
					log_err("ip_addr not defined");
					return -1;
				}

				if (strcmp(config->netmask.address, "") != 0) {
					match = regexec(&preg,
						config->netmask.address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong netmask");
						return -1;
					}
				} else {
					log_err("netmask not defined");
					return -1;
				}

				if (strcmp(config->gw_addr.address, "") != 0) {
					match = regexec(&preg,
						config->gw_addr.address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong gw_addr");
						return -1;
					}
				} else {
					log_err("gw_addr not defined");
					return -1;
				}

				if (strcmp(config->dns_addr[0].address, "")
									!= 0) {
					match = regexec(&preg,
						config->dns_addr[0].address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong dns_addr[0]");
						return -1;
					}
				} else {
					log_err("dns_addr[0] not defined");
					return -1;
				}

				if (strcmp(config->dns_addr[1].address, "")
									!= 0) {
					match = regexec(&preg,
						config->dns_addr[1].address,
						nmatch, pmatch, 0);

					if (match != 0) {
						log_err("Wrong dns_addr[1]");
						return -1;
					}
				}
			}
		}
	}

	return 0;
}

static int network_connection(int fd, enum watch_io io, void *user_data)
{
	int len = 0;
	unsigned char buf[4096];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl addr;
	struct nlmsghdr *hdr = NULL;
	struct ifinfomsg *infomsg = NULL;
	struct msghdr msg = { &addr, sizeof(addr), &iov, 1, NULL, 0, 0 };
	bool check_online_status = false;
	bool old_online_status = watch_online_status->current_online_status;

	artik_error ret = S_OK;

	if (io & (WATCH_IO_NVAL | WATCH_IO_HUP | WATCH_IO_ERR)) {
		log_dbg("%s netlink error", __func__);
		return 1;
	}
	len = recvmsg(fd, &msg, 0);

	for (hdr = (struct nlmsghdr *)buf; len > 0 && NLMSG_OK(hdr, len); hdr =
							NLMSG_NEXT(hdr, len)) {
		if (hdr->nlmsg_type == NLMSG_DONE ||
						hdr->nlmsg_type == NLMSG_ERROR)
			break;

		if (hdr->nlmsg_type ==  RTM_NEWLINK ||
					hdr->nlmsg_type == RTM_DELLINK) {
			infomsg = (struct ifinfomsg *)NLMSG_DATA(hdr);
			check_online_status = infomsg->ifi_flags & IFF_UP ?
				!old_online_status : old_online_status;
		} else if (hdr->nlmsg_type == RTM_NEWADDR ||
						hdr->nlmsg_type == RTM_NEWROUTE)
			check_online_status = !old_online_status;

		else if (hdr->nlmsg_type == RTM_DELADDR ||
						hdr->nlmsg_type == RTM_DELROUTE)
			check_online_status = old_online_status;
	}

	if (check_online_status) {
		ret = artik_get_online_status(
				&watch_online_status->current_online_status);
		if (ret != S_OK)
			return 1;
	}

	if (watch_online_status->current_online_status != old_online_status) {
		watch_online_node_t *node = (watch_online_node_t *)
						watch_online_status->root;
		do {
			node->config.callback(
				watch_online_status->current_online_status,
				node->config.user_data);
			node = (watch_online_node_t *)node->node.next;
		} while (node);

	}

	return 1;
}

static artik_error initialize_watch_online_status(void)
{
	artik_error ret = S_OK;
	struct sockaddr_nl addr;
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	watch_online_status = (watch_online_status_t *)
					malloc(sizeof(watch_online_status_t));
	if (!watch_online_status)
		return E_NO_MEM;

	watch_online_status->fd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC,
								NETLINK_ROUTE);
	if (watch_online_status->fd == -1) {
		log_err("couldn't open NETLINK_ROUTE socket");
		free(watch_online_status);
		watch_online_status = NULL;
		return E_ACCESS_DENIED;
	}

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE |
					(1<<(RTNLGRP_ND_USEROPT-1));

	if (bind(watch_online_status->fd, (struct sockaddr *)&addr,
							sizeof(addr)) != 0) {
		log_err("couldn't bind NETLINK_ROUTE socket");
		free(watch_online_status);
		watch_online_status = NULL;
		return E_ACCESS_DENIED;
	}

	ret = artik_get_online_status(
				&watch_online_status->current_online_status);
	if (ret != S_OK) {
		log_err("couldn't get initial online status");
		free(watch_online_status);
		watch_online_status = NULL;
		return ret;
	}

	loop->add_fd_watch(watch_online_status->fd,
		(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP |
		WATCH_IO_NVAL),
		network_connection,
		NULL,
		&(watch_online_status->watch_id));

	watch_online_status->root = NULL;

	return S_OK;
}

artik_error os_network_add_watch_online_status(
				watch_online_status_handle * handle,
				watch_online_status_callback app_callback,
				void *user_data)
{
	artik_error ret = S_OK;

	if (!watch_online_status) {
		ret = initialize_watch_online_status();

		if (ret != S_OK)
			return ret;
	}

	watch_online_node_t *node = (watch_online_node_t *)
		artik_list_add(&(watch_online_status->root), 0,
						sizeof(watch_online_node_t));

	if (!node)
		return E_NO_MEM;

	node->config.callback = app_callback;
	node->config.user_data = user_data;

	if (ret != S_OK)
		artik_list_delete_node(&(watch_online_status->root),
							(artik_list *)node);

	*handle = (watch_online_status_handle)node->node.handle;

	return ret;
}

artik_error os_network_remove_watch_online_status(
					watch_online_status_handle handle)
{
	if (!watch_online_status)
		return E_NOT_INITIALIZED;

	artik_list_delete_handle(&(watch_online_status->root),
						(ARTIK_LIST_HANDLE)handle);
	if (artik_list_size(watch_online_status->root) == 0) {
		artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
		loop->remove_fd_watch(watch_online_status->watch_id);
		close(watch_online_status->fd);
		free(watch_online_status);
		watch_online_status = NULL;
		artik_release_api_module(loop);
	}

	return S_OK;
}

static void on_dhcp_client_renew_callback(void *user_data)
{
	artik_network_dhcp_client_handle *handle =
				(artik_network_dhcp_client_handle *)user_data;

	dhcp_handle_client *dhcp_client = (dhcp_handle_client *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) * handle);

	struct in_addr addr;

	if (!dhcp_client) {
		log_err("No dhcp_client");
		return;
	}

	if (dhcp_client_renew(handle, dhcp_client->interface) != OK) {
		log_err("Failed to renew IP address in callback");

		/* Set IP address to 0.0.0.0 */
		addr.s_addr = INADDR_ANY;
		if (set_ipv4addr(dhcp_client->interface, &addr) == ERROR) {
			log_err("Set IPv4 address failed: %s", strerror(errno));
			return;
		}

		artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

		loop->quit();
	}
}

static int dhcp_client_renew(artik_network_dhcp_client_handle *handle,
		const char *interface)
{
	int ret = OK;
	struct in_addr addr;

	if (!*handle) {
		log_err("DHCP Client open failed");
		ret = ERROR;
		goto exit;
	} else {
		struct dhcpc_state ds;
		dhcp_handle_client *dhcp_client = (dhcp_handle_client *)
			artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) * handle);

		log_dbg("Renewing IP address");

		get_ipv4addr(interface, &addr);

		if (dhcpc_request(*handle, &ds, interface, &addr, true)
								== ERROR) {
			log_err("DHCP Client request failed");
			ret = ERROR;
			goto exit;
		}

		/* Set IP address */
		if (set_ipv4addr(interface, &ds.ipaddr) == ERROR) {
			log_err("Set IPv4 address failed: %s", strerror(errno));
			ret = ERROR;
			goto exit;
		}

		/* Set net mask */
		if (ds.netmask.s_addr != 0) {
			if (set_ipv4netmask(interface, &ds.netmask) == ERROR) {
				log_err("Set IPv4 network mask failed: %s",
							strerror(errno));
				ret = ERROR;
				goto exit;
			}
		}

		/* Set default router */
		if (ds.default_router.s_addr != 0) {
			if (set_dripv4addr(interface, &ds.default_router)
								== ERROR) {
				log_err("Set default router address failed: %s",
							strerror(errno));
				ret = ERROR;
				goto exit;
			}
		}

		/* Set DNS address */
		if (ds.dnsaddr.s_addr != 0) {
			if (set_ipv4dnsaddr(&ds.dnsaddr, false) == ERROR) {
				log_err("Set DNS adress failed: %s",
							strerror(errno));
				ret = ERROR;
				goto exit;
			}
		}

		/* Set route with gateway */
		if (set_defaultroute(interface, &ds.default_router, true)
					== ERROR && errno != ROUTE_EXISTS) {
			log_err("Set default route with GW failed: %s",
							strerror(errno));
			ret = ERROR;
			goto exit;
		}

		/*
		 * Add timeout callback for renewing IP address before the lease
		 * expires
		 */
		ret = dhcp_client->loop_module->remove_timeout_callback(
						dhcp_client->renew_cbk_id);

		if (ret != S_OK) {
			log_err("Failed to remove callback for renewing IP addr"
									);
			return ret;
		}

		ret = dhcp_client->loop_module->add_timeout_callback(
						&dhcp_client->renew_cbk_id,
						(ds.lease_time-(30))*1000,
						on_dhcp_client_renew_callback,
						handle);

		if (ret != S_OK) {
			log_err("Failed to start callback for renewing IP addr"
									);
			return ret;
		}

		log_dbg("IP: %s", inet_ntoa(ds.ipaddr));
	}


exit:
	return ret;
}


artik_error os_dhcp_client_start(artik_network_dhcp_client_handle *handle,
		artik_network_interface_t interface)
{
	artik_error ret = S_OK;
	uint8_t mac[IFHWADDRLEN];
	const char *_interface = interface == ARTIK_WIFI ? "wlan0" : "eth0";
	struct in_addr addr;
	dhcp_handle_client *dhcp_client = NULL;

	/* Delete all routes from interface if they exist */
	if (del_allroutes_interface(_interface) == ERROR) {
		log_err("Delete all routes from interface %s failed: %s",
			_interface,
			strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set the default route to 0.0.0.0 */
	addr.s_addr = INADDR_ANY;
	if (set_defaultroute(_interface, &addr, false) == ERROR) {
		log_err("Set default route failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Get the MAC address */
	if (getmacaddr(_interface, mac) == ERROR) {
		log_err("Get MAC address failed : %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set up the DHCPC modules */
	*handle = (artik_network_dhcp_client_handle)dhcpc_open(&mac,
								IFHWADDRLEN);

	if (!*handle) {
		log_err("DHCP Client open failed");
		ret = E_NETWORK_ERROR;
	} else {
		struct dhcpc_state ds;


		log_dbg("Getting IP address");

		if (dhcpc_request(*handle, &ds, _interface, NULL, false)
								== ERROR) {
			log_err("DHCP Client request failed");
			return E_NETWORK_ERROR;
		}

		dhcp_client = (dhcp_handle_client *)artik_list_add(
				&requested_node, 0, sizeof(dhcp_handle_client));

		if (!dhcp_client)
			return E_NO_MEM;

		dhcp_client->node.handle = (ARTIK_LIST_HANDLE)*handle;
		dhcp_client->loop_module = (artik_loop_module *)
					artik_request_api_module("loop");
		dhcp_client->interface = interface ==
						ARTIK_WIFI ? "wlan0" : "eth0";

		/* Set IP address */
		if (set_ipv4addr(_interface, &ds.ipaddr) == ERROR) {
			log_err("Set IPv4 address failed: %s", strerror(errno));
			return E_NETWORK_ERROR;
		}

		/* Set net mask */
		if (ds.netmask.s_addr != 0) {
			if (set_ipv4netmask(_interface, &ds.netmask) == ERROR) {
				log_err("Set IPv4 network mask failed: %s",
					strerror(errno));
				return E_NETWORK_ERROR;
			}
		}

		/* Set default router */
		if (ds.default_router.s_addr != 0) {
			if (set_dripv4addr(_interface, &ds.default_router)
								== ERROR) {
				log_err("Set default router address failed: %s",
					strerror(errno));
				return E_NETWORK_ERROR;
			}
		}

		/* Set DNS address */
		if (ds.dnsaddr.s_addr != 0) {
			if (set_ipv4dnsaddr(&ds.dnsaddr, false) == ERROR) {
				log_err("Set DNS adress failed: %s",
					strerror(errno));
				return E_NETWORK_ERROR;
			}
		}

		/* Set route with gateway */
		if (set_defaultroute(_interface, &ds.default_router, true)
								== ERROR) {
			log_err("Set default route with GW failed: %s",
				strerror(errno));
			return E_NETWORK_ERROR;
		}

		/*
		 * Add timeout callback for renewing IP address before the lease
		 * expires
		 */
		ret = dhcp_client->loop_module->add_timeout_callback(
						&dhcp_client->renew_cbk_id,
						(ds.lease_time-(30))*1000,
						on_dhcp_client_renew_callback,
						(void *)handle);

		if (ret != S_OK) {
			log_err("Failed to start callback for renewing IP addr"
									);
			return ret;
		}

		log_dbg("IP: %s", inet_ntoa(ds.ipaddr));
	}

	return ret;
}

artik_error os_dhcp_client_stop(artik_network_dhcp_client_handle handle)
{
	artik_error ret = S_OK;
	dhcp_handle_client *dhcp_client = (dhcp_handle_client *)
		artik_list_get_by_handle(requested_node,
		(ARTIK_LIST_HANDLE) handle);

	if (!dhcp_client) {
		log_err("No dhcp_client");
		return E_NO_MEM;
	}

	if (handle) {
		dhcpc_close(handle);
		log_dbg("DHCP Client stopped");
	} else {
		log_err("Handle NULL");
		return E_NETWORK_ERROR;
	}

	return ret;
}

artik_error os_dhcp_server_start(artik_network_dhcp_server_handle *handle,
		artik_network_dhcp_server_config *config)
{
	artik_error ret = S_OK;
	struct in_addr addr;
	dhcp_handle_server *dhcp_server = NULL;

	if (!handle) {
		log_err("handle is NULL");
		return E_BAD_ARGS;
	}

	if (!config) {
		log_err("config is NULL");
		return E_BAD_ARGS;
	}

	if (check_dhcp_server_config(config) < 0) {
		log_err("Wrong server config");
		return E_NETWORK_ERROR;
	}

	dhcp_server = (dhcp_handle_server *)artik_list_add(&requested_node, 0,
						sizeof(dhcp_handle_server));

	if (!dhcp_server)
		return E_NO_MEM;

	dhcp_server->node.handle = (ARTIK_LIST_HANDLE)dhcp_server;
	dhcp_server->interface = config->interface == ARTIK_WIFI ?
							"wlan0" : "eth0";
	memcpy(&dhcp_server->config, config, sizeof(dhcp_server->config));

	*handle = (artik_network_dhcp_server_handle)dhcp_server;

	/* Delete all routes from interface if they exist */
	if (del_allroutes_interface(dhcp_server->interface) == ERROR) {
		log_err("Delete all routes from interface %s failed: %s",
			dhcp_server->interface,
			strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set IP address */
	addr.s_addr = inet_addr(dhcp_server->config.ip_addr.address);
	if (set_ipv4addr(dhcp_server->interface, &addr) == ERROR) {
		log_err("Set IPv4 address failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set net mask */
	addr.s_addr = inet_addr(dhcp_server->config.netmask.address);
	if (set_ipv4netmask(dhcp_server->interface, &addr) == ERROR) {
		log_err("Set IPv4 network mask failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set default router */
	addr.s_addr = inet_addr(dhcp_server->config.gw_addr.address);
	if (set_dripv4addr(dhcp_server->interface, &addr) == ERROR) {
		log_err("Set default router address failed: %s",
			strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set the default route to 0.0.0.0 */
	addr.s_addr = INADDR_ANY;
	if (set_defaultroute(dhcp_server->interface, &addr, false) == ERROR) {
		log_err("Set default route failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Launch DHCP server */
	if (dhcpd_run(&dhcp_server->config, &dhcp_server->sockfd,
					&dhcp_server->watch_id) == ERROR) {
		log_err("DHCP Server run failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	return ret;
}

artik_error os_dhcp_server_stop(artik_network_dhcp_server_handle handle)
{
	artik_error ret = S_OK;
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	dhcp_handle_server *dhcp_server = (dhcp_handle_server *)
		artik_list_get_by_handle(requested_node,
		(ARTIK_LIST_HANDLE) handle);
	struct in_addr addr;

	if (!dhcp_server)
		return E_NO_MEM;

	if (close(dhcp_server->sockfd) < 0) {
		log_err("Stop DHCP server failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	if (loop->remove_fd_watch(dhcp_server->watch_id) != S_OK) {
		log_err("Failed to remove fd watch");
		return E_NETWORK_ERROR;
	}

	/* Delete all routes from interface if they exist */
	if (del_allroutes_interface(dhcp_server->interface) == ERROR) {
		log_err("Delete all routes from interface %s failed: %s",
			dhcp_server->interface,
			strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set the default route to 0.0.0.0 */
	addr.s_addr = INADDR_ANY;
	if (set_defaultroute(dhcp_server->interface, &addr, false) == ERROR) {
		log_err("Set default route failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set IP address to 0.0.0.0 */
	if (set_ipv4addr(dhcp_server->interface, &addr) == ERROR) {
		log_err("Set IPv4 address failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	return ret;
}

artik_error os_set_network_config(artik_network_config *config,
		artik_network_interface_t interface)
{
	artik_error ret = S_OK;
	const char *_interface = interface == ARTIK_WIFI ? "wlan0" : "eth0";
	struct in_addr addr;

	if (check_network_config(config) < 0) {
		log_err("Wrong network config");
		return E_NETWORK_ERROR;
	}

	/* Set IP address */
	if (inet_aton(config->ip_addr.address, &addr) == 0) {
		log_err("Error inet_aton ip_addr");
		return E_NETWORK_ERROR;
	}

	if (set_ipv4addr(_interface, &addr) == ERROR) {
		log_err("Set IPv4 address failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set net mask */
	if (inet_aton(config->netmask.address, &addr) == 0) {
		log_err("Error inet_aton netmask");
		return E_NETWORK_ERROR;
	}

	if (set_ipv4netmask(_interface, &addr) == ERROR) {
		log_err("Set IPv4 network mask failed: %s",
			strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set default router */
	if (inet_aton(config->gw_addr.address, &addr) == 0) {
		log_err("Error inet_aton gw_addr");
		return E_NETWORK_ERROR;
	}

	if (set_dripv4addr(_interface, &addr) == ERROR) {
		log_err("Set default router address failed: %s",
			strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set route with gateway */
	if (set_defaultroute(_interface, &addr, true) == ERROR) {
		log_err("Set default route with GW failed: %s",
			strerror(errno));
		return E_NETWORK_ERROR;
	}

	/* Set DNS address */
	for (int i = 0; i < MAX_DNS_ADDRESSES; i++) {

		bool append = i == 0 ? false : true;

		if (strcmp(config->dns_addr[i].address, "") != 0) {
			if (inet_aton(config->dns_addr[i].address,
							&addr) == 0) {
				log_err("Error inet_aton dns_addr");
				return E_NETWORK_ERROR;
			}


			if (set_ipv4dnsaddr(&addr, append) == ERROR) {
				log_err("Set DNS adress failed: %s",
					strerror(errno));
				return E_NETWORK_ERROR;
			}
		}
	}

	return ret;
}

artik_error os_get_network_config(artik_network_config *config,
		artik_network_interface_t interface)
{
	artik_error ret = S_OK;
	uint8_t mac[IFHWADDRLEN];
	const char *_interface = interface == ARTIK_WIFI ? "wlan0" : "eth0";
	struct in_addr addr;
	struct in_addr dnsAddr[MAX_DNS_ADDRESSES] = { 0 };

	if (!config) {
		log_err("config is NULL");
		return E_BAD_ARGS;
	}

	/* Get IP address */
	if (get_ipv4addr(_interface, &addr) == ERROR) {
		log_err("Get IPv4 address failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	snprintf(config->ip_addr.address, MAX_IP_ADDRESS_LEN,
			"%d.%d.%d.%d",
			(addr.s_addr) & 0xff,
			(addr.s_addr >> 8) & 0xff,
			(addr.s_addr >> 16) & 0xff,
			(addr.s_addr >> 24) & 0xff);

	/* Get MAC address */
	if (getmacaddr(_interface, mac) == ERROR) {
		log_err("Get MAC address failed : %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	snprintf((char *)config->mac_addr, MAX_MAC_ADDRESS_LEN,
		"%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2],
		mac[3], mac[4], mac[5]);

	/* Get Mask */
	if (get_ipv4netmask(_interface, &addr) == ERROR) {
		log_err("Get mask failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	snprintf(config->netmask.address, MAX_IP_ADDRESS_LEN,
			"%d.%d.%d.%d",
			(addr.s_addr) & 0xff,
			(addr.s_addr >> 8) & 0xff,
			(addr.s_addr >> 16) & 0xff,
			(addr.s_addr >> 24) & 0xff);

	/* Get the gateway address */
	if (get_dripv4addr(_interface, &addr) == ERROR) {
		log_err("Get gateway address failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	snprintf(config->gw_addr.address, MAX_IP_ADDRESS_LEN,
			"%d.%d.%d.%d",
			(addr.s_addr) & 0xff,
			(addr.s_addr >> 8) & 0xff,
			(addr.s_addr >> 16) & 0xff,
			(addr.s_addr >> 24) & 0xff);

	/* Get DNS servers */
	if (get_ipv4dnsaddr(dnsAddr, MAX_DNS_ADDRESSES) == ERROR) {
		log_err("Get DNS servers failed: %s", strerror(errno));
		return E_NETWORK_ERROR;
	}

	for (int i = 0; i < MAX_DNS_ADDRESSES; i++) {
		snprintf(config->dns_addr[i].address, MAX_IP_ADDRESS_LEN,
			"%d.%d.%d.%d",
			(dnsAddr[i].s_addr) & 0xff,
			(dnsAddr[i].s_addr >> 8) & 0xff,
			(dnsAddr[i].s_addr >> 16) & 0xff,
			(dnsAddr[i].s_addr >> 24) & 0xff);
	}

	return ret;
}
