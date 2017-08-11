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


#include "artik_wifi.h"
#include "os_wifi.h"

static artik_error artik_wifi_scan_request(void);
static artik_error artik_wifi_get_scan_result(artik_wifi_ap **aps,
					      int *num_aps);
static artik_error artik_wifi_start_ap(const char *ssid, const char *password,
					   unsigned int channel,
					   unsigned int encryption_flags);
static artik_error artik_wifi_connect(const char *ssid, const char *password,
				      bool persistent);
static artik_error artik_wifi_disconnect(void);
static artik_error artik_wifi_set_scan_result_callback(artik_wifi_callback
						       user_callback,
						       void *user_data);
static artik_error artik_wifi_set_connect_callback(artik_wifi_callback
						   user_callback,
						   void *user_data);
static artik_error artik_wifi_init(artik_wifi_mode_t mode);
static artik_error artik_wifi_deinit(void);

artik_wifi_module wifi_module = {
	artik_wifi_init,
	artik_wifi_deinit,
	artik_wifi_scan_request,
	artik_wifi_get_scan_result,
	artik_wifi_start_ap,
	artik_wifi_connect,
	artik_wifi_disconnect,
	artik_wifi_set_scan_result_callback,
	artik_wifi_set_connect_callback,
};

artik_error artik_wifi_init(artik_wifi_mode_t mode)
{
	return os_wifi_init(mode);
}

artik_error artik_wifi_deinit(void)
{
	return os_wifi_deinit();
}

artik_error artik_wifi_scan_request(void)
{
	return os_wifi_scan_request();
}

artik_error artik_wifi_get_scan_result(artik_wifi_ap **aps, int *num_aps)
{
	return os_wifi_get_scan_result(aps, num_aps);
}

artik_error artik_wifi_start_ap(const char *ssid, const char *password,
			unsigned int channel,
			unsigned int encryption_flags)
{
	return os_wifi_start_ap(ssid, password, channel, encryption_flags);
}

artik_error artik_wifi_connect(const char *ssid, const char *password,
			       bool persistent)
{
	return os_wifi_connect(ssid, password, persistent);
}

artik_error artik_wifi_disconnect(void)
{
	return os_wifi_disconnect();
}

artik_error artik_wifi_set_scan_result_callback(artik_wifi_callback
						user_callback, void *user_data)
{
	return os_wifi_set_scan_result_callback(user_callback, user_data);
}

artik_error artik_wifi_set_connect_callback(artik_wifi_callback user_callback,
					    void *user_data)
{
	return os_wifi_set_connect_callback(user_callback, user_data);
}
