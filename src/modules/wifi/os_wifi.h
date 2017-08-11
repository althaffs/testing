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

#ifndef	__OS_WIFI_H__
#define	__OS_WIFI_H__

#include "artik_error.h"

artik_error os_wifi_init(artik_wifi_mode_t mode);
artik_error os_wifi_deinit(void);
artik_error os_wifi_scan_request(void);
artik_error os_wifi_get_scan_result(artik_wifi_ap **aps, int *num_aps);
artik_error os_wifi_start_ap(const char *ssid, const char *password,
		unsigned int channel, unsigned int encryption_flags);
artik_error os_wifi_connect(const char *ssid, const char *password,
		bool persistent);
artik_error os_wifi_disconnect(void);
artik_error os_wifi_set_scan_result_callback(artik_wifi_callback user_callback,
		void *user_data);
artik_error os_wifi_set_connect_callback(artik_wifi_callback user_callback,
		void *user_data);

#endif  /* __OS_WIFI_H__ */
