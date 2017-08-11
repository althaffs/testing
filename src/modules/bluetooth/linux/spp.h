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

#ifndef __ARTIK_BT_SPP_H
#define __ARTIK_BT_SPP_H

#include <stdbool.h>
#include <artik_bluetooth.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	release_callback release_func;
	new_connection_callback connect_func;
	request_disconnect_callback disconnect_func;
	void *user_data;
} artik_bt_spp_server;

artik_error bt_spp_register_profile(artik_bt_spp_profile_option *opt);

artik_error bt_spp_unregister_profile(void);

artik_error bt_spp_set_callback(release_callback release_func,
		new_connection_callback connect_func,
		request_disconnect_callback disconnect_func,
		void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_SPP_H */
