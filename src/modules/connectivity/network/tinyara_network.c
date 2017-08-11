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

#include <apps/netutils/dhcpc.h>
#include <artik_network.h>
#include <artik_log.h>

#include "os_network.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netlib.h>

#define DHCPC_HANDLE	((artik_network_dhcp_client_handle)0x44484343)
#define DHCPS_HANDLE	((artik_network_dhcp_server_handle)0x44484353)

static const char *wifi_iface = "wl1";

artik_error os_dhcp_client_start(artik_network_dhcp_client_handle *handle,
		artik_network_interface_t interface)
{
	artik_error ret = S_OK;
	struct dhcpc_state state;
	void *dhcp_handle = NULL;
	int err = 0;

	log_dbg("");

	if (interface != ARTIK_WIFI)
		return E_NOT_SUPPORTED;

	dhcp_handle = dhcpc_open(wifi_iface);
	err = dhcpc_request(dhcp_handle, &state);
	dhcpc_close(dhcp_handle);

	if (err != OK) {
		log_err("Failed to request DHCP lease (err=%d)\n", err);
		switch (err) {
		case -100:
			ret = E_BAD_ARGS;
			break;
		case -2:
			ret = E_TIMEOUT;
			break;
		default:
			ret = E_NOT_CONNECTED;
			break;
		}
		goto exit;
	}

	netlib_set_ipv4addr(wifi_iface, &state.ipaddr);
	netlib_set_ipv4netmask(wifi_iface, &state.netmask);
	netlib_set_dripv4addr(wifi_iface, &state.default_router);

	*handle = DHCPC_HANDLE;

exit:
	return ret;
}

artik_error os_dhcp_client_stop(artik_network_dhcp_client_handle handle)
{
	struct in_addr zeroip;

	log_dbg("");

	if (handle != DHCPC_HANDLE)
		return E_BAD_ARGS;

	zeroip.s_addr = inet_addr("0.0.0.0");

	netlib_set_ipv4addr(wifi_iface, &zeroip);
	netlib_set_ipv4netmask(wifi_iface, &zeroip);
	netlib_set_dripv4addr(wifi_iface, &zeroip);

	return S_OK;
}

artik_error os_dhcp_server_start(artik_network_dhcp_server_handle *handle,
		artik_network_dhcp_server_config *config)
{
	struct in_addr ipaddr;

	log_dbg("");

	if (config->interface != ARTIK_WIFI)
		return E_NOT_SUPPORTED;

	if (dhcpd_start(wifi_iface)) {
		log_err("Failed to start DHCP server\n");
		ipaddr.s_addr = INADDR_ANY;
		netlib_set_ipv4addr(wifi_iface, &ipaddr);
		return E_NOT_CONNECTED;
	}

	*handle = DHCPS_HANDLE;

	return S_OK;
}

artik_error os_dhcp_server_stop(artik_network_dhcp_server_handle handle)
{
	struct in_addr ipaddr;

	log_dbg("");

	if (handle != DHCPS_HANDLE)
		return E_BAD_ARGS;

	dhcpd_stop();
	ipaddr.s_addr = INADDR_ANY;
	netlib_set_ipv4addr(wifi_iface, &ipaddr);

	return S_OK;
}

artik_error os_network_add_watch_online_status(
				watch_online_status_handle * handle,
				watch_online_status_callback app_callback,
				void *user_data)
{
	log_dbg("");

	return E_NOT_SUPPORTED;
}

artik_error os_network_remove_watch_online_status(
					watch_online_status_handle handle)
{
	log_dbg("");

	return E_NOT_SUPPORTED;
}

typedef struct {
	size_t count;
	artik_network_ip *dns_addr;
} dns_addresses;

static int add_nameserver(void *arg, struct sockaddr *addr, socklen_t addrlen)
{
	struct sockaddr_in *sin;
	dns_addresses *dns = arg;
	artik_network_ip *dns_addr = dns->dns_addr + dns->count;

	/* Ignore IPV6 address */
	if (addr->sa_family != AF_INET)
		return OK;

	if (dns->count > MAX_DNS_ADDRESSES)
		return OK;

	sin = (struct sockaddr_in *) addr;
	dns_addr->type = ARTIK_IPV4;

	dns->count++;
	if (!inet_ntop(AF_INET, &sin->sin_addr, dns_addr->address, MAX_IP_ADDRESS_LEN)) {
		log_dbg("Failed to convert DNS ip address into a character string.");
		return ERROR;
	}

	return OK;
}

artik_error os_get_network_config(
	artik_network_config * config,
	artik_network_interface_t interface
	)
{
	int sockfd;
	int ret;
	struct in_addr ipv4_host, ipv4_gw;
	struct sockaddr_in *ipv4_netmask, *sockaddr_in;
	struct ifreq req;
	dns_addresses dns;
	uint8_t macaddr[IFHWADDRLEN];

	if (!config) {
		log_dbg("config is NULL");
		return E_BAD_ARGS;
	}

	if (interface != ARTIK_WIFI) {
		log_dbg("Only ARTIK_WIFI is supported.");
		return E_BAD_ARGS;
	}

	sockfd = socket(PF_INET, NETLIB_SOCK_IOCTL, 0);
	if (sockfd < 0) {
		log_dbg("Failed to open the socket.");
		return E_NETWORK_ERROR;
	}

	memset(config, 0, sizeof(artik_network_config));
	memset(&req, 0, sizeof(struct ifreq));
	strncpy(req.ifr_name, "wl1", IFNAMSIZ);

	ret = ioctl(sockfd, SIOCGIFADDR, &req);
	if (ret) {
		log_dbg("Failed to get IP address. (err %d)", ret);
		return E_NETWORK_ERROR;
	}

	sockaddr_in = (struct sockaddr_in *)&req.ifr_addr;
	memcpy(&ipv4_host, &sockaddr_in->sin_addr, sizeof(struct in_addr));

	ret = ioctl(sockfd, SIOCGIFDSTADDR, &req);
	if (ret) {
		log_dbg("Failed to get GW address.");
		return E_NETWORK_ERROR;
	}

	sockaddr_in = (struct sockaddr_in *)&req.ifr_addr;
	memcpy(&ipv4_gw, &sockaddr_in->sin_addr, sizeof(struct in_addr));

	ret = ioctl(sockfd, SIOCGIFNETMASK, &req);
	if (ret) {
		log_dbg("Failed to get netmask");
		return E_NETWORK_ERROR;
	}

	ipv4_netmask = (struct sockaddr_in *)&req.ifr_netmask;

	if (netlib_getmacaddr("wl1", macaddr) != OK) {
		log_dbg("Failed to get mac address");
		return E_NETWORK_ERROR;
	}

	snprintf(config->mac_addr, MAX_MAC_ADDRESS_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
		macaddr[0], macaddr[1], macaddr[2],
		macaddr[3], macaddr[4], macaddr[5]);

	dns.count = 0;
	dns.dns_addr = config->dns_addr;
	printf("config->dns_addr %p\n", config->dns_addr);
	if (dns_foreach_nameserver(add_nameserver, &dns) != OK) {
		log_dbg("Failed to get DNS servers.");
		return E_NETWORK_ERROR;
	}

	if (!inet_ntop(AF_INET, &ipv4_host, config->ip_addr.address, MAX_IP_ADDRESS_LEN)) {
		log_dbg("Failed to convert host ip address into a character string.");
		return E_NETWORK_ERROR;
	}

	if (!inet_ntop(AF_INET, &ipv4_gw, config->gw_addr.address, MAX_IP_ADDRESS_LEN)) {
		log_dbg("Failed to convert gw ip address into a character string.");
		return E_NETWORK_ERROR;
	}

	if (!inet_ntop(AF_INET, &ipv4_netmask->sin_addr, config->netmask.address, MAX_IP_ADDRESS_LEN)) {
		log_dbg("Failed to convert netmask ip address into a character string.");
		return E_NETWORK_ERROR;
	}

	config->ip_addr.type = ARTIK_IPV4;
	config->gw_addr.type = ARTIK_IPV4;
	config->netmask.type = ARTIK_IPV4;

	return S_OK;
}

artik_error os_set_network_config(artik_network_config *config, artik_network_interface_t interface)
{
	struct in_addr host_addr, gw_addr, netmask_addr;
	struct in_addr dns[MAX_DNS_ADDRESSES];
	int i;
	size_t count = 0;

	if (!config) {
		log_dbg("config is NULL");
		return E_BAD_ARGS;
	}

	if (interface != ARTIK_WIFI) {
		log_dbg("Only ARTIK_WIFI is supported.");
		return E_BAD_ARGS;
	}

	if (!inet_pton(AF_INET, config->ip_addr.address, &host_addr)) {
		log_dbg("Failed to convert host ip addresse into a network address structure.");
		return E_BAD_ARGS;
	}

	if (!inet_pton(AF_INET, config->gw_addr.address, &gw_addr)) {
		log_dbg("Failed to convert gw ip addresse into a network address structure.");
		return E_BAD_ARGS;
	}

	if (!inet_pton(AF_INET, config->netmask.address, &netmask_addr)) {
		log_dbg("Failed to convert netmask addresse into a network address structure.");
		return E_BAD_ARGS;
	}

	for (i = 0; i < MAX_DNS_ADDRESSES; i++) {
		if (config->dns_addr[i].address[0] == '\0')
			continue;

		if (!inet_pton(AF_INET, config->dns_addr[i].address, dns + count)) {
			log_dbg("Failed to convert dns address into a network address structure.");
			return E_BAD_ARGS;
		}

		count++;
	}

	if (netlib_set_ipv4addr("wl1", &host_addr) != OK) {
		log_dbg("Failed to set ipv4 address");
		return E_NETWORK_ERROR;
	}

	if (netlib_set_dripv4addr("wl1", &gw_addr) != OK) {
		log_dbg("Failed to set gw address.");
		return E_NETWORK_ERROR;
	}

	if (netlib_set_ipv4netmask("wl1", &netmask_addr) != OK) {
		log_dbg("Failed to set netmask address.");
		return E_NETWORK_ERROR;
	}

	for (i = 0; i < count; i++) {
		struct sockaddr_in sockaddr_in;

		sockaddr_in.sin_family = AF_INET;
		sockaddr_in.sin_port = 0;
		sockaddr_in.sin_addr.s_addr = dns[i].s_addr;
		if (dns_add_nameserver((struct sockaddr *)&sockaddr_in, sizeof(struct sockaddr_in)) == ERROR) {
			log_dbg("Failed to add new DNS.");
			return E_NETWORK_ERROR;
		}
	}

	return S_OK;
}
