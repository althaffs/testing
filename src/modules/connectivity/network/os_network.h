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

#ifndef	OS_NETWORK_H_
#define	OS_NETWORK_H_

#include "artik_error.h"

artik_error os_set_network_config(
		artik_network_config * config,
		artik_network_interface_t interface);
artik_error os_get_network_config(
		artik_network_config * config,
		artik_network_interface_t interface);
artik_error os_network_add_watch_online_status(
		watch_online_status_handle * handle,
		watch_online_status_callback app_callback, void *user_data);
artik_error os_network_remove_watch_online_status(
		watch_online_status_handle handle);
artik_error os_dhcp_client_start(artik_network_dhcp_client_handle *handle,
		artik_network_interface_t interface);
artik_error os_dhcp_client_stop(artik_network_dhcp_client_handle handle);
artik_error os_dhcp_server_start(artik_network_dhcp_server_handle *handle,
		artik_network_dhcp_server_config *config);
artik_error os_dhcp_server_stop(artik_network_dhcp_server_handle handle);

artik_error artik_get_current_public_ip(artik_network_ip *ip);
artik_error artik_get_online_status(bool *online_status);

#endif	/* OS_NETWORK_H_ */
