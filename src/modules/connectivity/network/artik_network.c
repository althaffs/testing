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

#include <artik_module.h>
#include <artik_http.h>
#include <artik_loop.h>
#include <artik_log.h>
#include <artik_network.h>

#include "os_network.h"

#define ARTIK_CURRENT_IP_URL	"http://www.checkip.org/"
#define ARRAY_SIZE(a)		(sizeof(a) / sizeof((a)[0]))

static artik_error artik_set_network_config(
		artik_network_config * config,
		artik_network_interface_t interface);
static artik_error artik_get_network_config(
		artik_network_config * config,
		artik_network_interface_t interface);
static artik_error artik_dhcp_client_start(
		artik_network_dhcp_client_handle * handle,
		artik_network_interface_t interface);
static artik_error artik_dhcp_client_stop(
		artik_network_dhcp_client_handle handle);
static artik_error artik_dhcp_server_start(
		artik_network_dhcp_server_handle * handle,
		artik_network_dhcp_server_config *config);
static artik_error artik_dhcp_server_stop(
		artik_network_dhcp_server_handle handle);
static artik_error artik_add_watch_online_status(
		watch_online_status_handle * handle,
		watch_online_status_callback app_callback,
		void *user_data);
static artik_error artik_remove_watch_online_status(
		watch_online_status_handle handle);

const artik_network_module network_module = {
		artik_set_network_config,
		artik_get_network_config,
		artik_get_current_public_ip,
		artik_dhcp_client_start,
		artik_dhcp_client_stop,
		artik_dhcp_server_start,
		artik_dhcp_server_stop,
		artik_get_online_status,
		artik_add_watch_online_status,
		artik_remove_watch_online_status
};

artik_error artik_get_current_public_ip(artik_network_ip *ip)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int i = 0;
	char *response = NULL;
	char *point;
	char *token = NULL;
	char delimiter[] = "<>";
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"user-agent", "Artik browser"},
		{"Accept-Language", "en-US,en;q=0.8"},
	};

	log_dbg("");

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Perform the request */
	ret = http->get(ARTIK_CURRENT_IP_URL, &headers, &response, NULL, false);
	if (ret != S_OK)
		goto exit;

	point = strstr(response, "Your IP Address:");
	if (point != NULL) {
		token = strtok(point, delimiter);
		for (i = 0; token != NULL && i < 2; i++)
			token = strtok(NULL, delimiter);
		strncpy(ip->address, token, 15);
	}
exit:
	artik_release_api_module(http);

	return ret;
}

artik_error artik_get_online_status(bool *online_status)
{
	artik_network_ip current_ip;
	artik_error ret = artik_get_current_public_ip(&current_ip);

	if (ret == S_OK) {
		*online_status = true;
	} else if (ret == E_HTTP_ERROR) {
		*online_status = false;
		ret = S_OK;
	}

	return ret;
}

artik_error artik_dhcp_client_start(artik_network_dhcp_client_handle *handle,
		artik_network_interface_t interface)
{
	return os_dhcp_client_start(handle, interface);
}

artik_error artik_dhcp_client_stop(artik_network_dhcp_client_handle handle)
{
	return os_dhcp_client_stop(handle);
}

artik_error artik_dhcp_server_start(artik_network_dhcp_server_handle *handle,
		artik_network_dhcp_server_config *config)
{
	return os_dhcp_server_start(handle, config);
}

artik_error artik_dhcp_server_stop(artik_network_dhcp_server_handle handle)
{
	return os_dhcp_server_stop(handle);
}

artik_error artik_set_network_config(artik_network_config *config,
		artik_network_interface_t interface)
{
	return os_set_network_config(config, interface);
}

artik_error artik_get_network_config(artik_network_config *config,
		artik_network_interface_t interface)
{
	return os_get_network_config(config, interface);
}

artik_error artik_add_watch_online_status(watch_online_status_handle *handle,
				watch_online_status_callback app_callback,
				void *user_data)
{
	return os_network_add_watch_online_status(handle, app_callback,
							user_data);
}

artik_error artik_remove_watch_online_status(watch_online_status_handle handle)
{
	return os_network_remove_watch_online_status(handle);
}
