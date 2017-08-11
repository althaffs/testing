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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_wifi.h>

#define MAX_PARAM_LEN	256

static char ssid[MAX_PARAM_LEN];
static char psk[MAX_PARAM_LEN];

static void get_scan_result(void)
{
	artik_wifi_module *wifi = (artik_wifi_module *)
					artik_request_api_module("wifi");
	artik_wifi_ap *list = NULL;
	int count = 0;
	int ret = 0;
	int i = 0;

	ret = wifi->get_scan_result(&list, &count);
	if (ret != S_OK)
		goto exit;

	for (i = 0; i < count; i++)
		fprintf(stdout, "%-20s %s 0x%X\n", list[i].bssid, list[i].name,
			list[i].encryption_flags);

	free(list);
	artik_release_api_module(wifi);

	return;

exit:

	artik_release_api_module(wifi);
	fprintf(stdout, "failed");
}

static void on_scan_result(void *result, void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_wifi_module *wifi = (artik_wifi_module *)
					artik_request_api_module("wifi");
	artik_error err = *((artik_error *)result);

	fprintf(stdout, "%s (err=%d)\n", __func__, err);

	get_scan_result();

	wifi->deinit();
	loop->quit();

	artik_release_api_module(loop);
	artik_release_api_module(wifi);
}

static void on_scan_timeout(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_wifi_module *wifi = (artik_wifi_module *)
					artik_request_api_module("wifi");

	fprintf(stdout, "TEST: %s reached timeout\n", __func__);

	get_scan_result();

	wifi->deinit();
	loop->quit();

	artik_release_api_module(loop);
	artik_release_api_module(wifi);
}

artik_error test_wifi_scan(void)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_wifi_module *wifi = (artik_wifi_module *)
					artik_request_api_module("wifi");
	artik_error ret;
	int timeout_id = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = wifi->init(ARTIK_WIFI_MODE_STATION);
	if (ret != S_OK)
		goto exit;
	ret = wifi->set_scan_result_callback(on_scan_result, NULL);
	if (ret != S_OK)
		goto exit;
	ret = wifi->scan_request();
	if (ret != S_OK)
		goto exit;

	loop->add_timeout_callback(&timeout_id, 10 * 1000, on_scan_timeout,
				   NULL);
	loop->run();

exit:
	fprintf(stdout, "TEST: %s %s\n", __func__,
		(ret == S_OK) ? "succeeded" : "failed");

	artik_release_api_module(loop);
	artik_release_api_module(wifi);

	return ret;
}

static void on_connect(void *result, void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_wifi_module *wifi = (artik_wifi_module *)
					artik_request_api_module("wifi");
	artik_wifi_connection_info *info = (artik_wifi_connection_info *)result;

	fprintf(stdout, "%s - err=%d, connected=%s\n", __func__, info->error,
		info->connected ? "true" : "false");

	wifi->deinit();
	loop->quit();

	artik_release_api_module(loop);
	artik_release_api_module(wifi);
}

artik_error test_wifi_connect(void)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_wifi_module *wifi = (artik_wifi_module *)
					artik_request_api_module("wifi");
	artik_error ret = S_OK;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = wifi->init(ARTIK_WIFI_MODE_STATION);
	if (ret != S_OK)
		goto exit;
	ret = wifi->set_connect_callback(on_connect, NULL);
	if (ret != S_OK)
		goto exit;
	ret = wifi->connect(ssid, psk, 0);
	if (ret != S_OK)
		goto exit;

	loop->run();

exit:
	fprintf(stdout, "TEST: %s %s\n", __func__,
		(ret == S_OK) ? "succeeded" : "failed");

	artik_release_api_module(loop);
	artik_release_api_module(wifi);

	return ret;
}

int main(int argc, char *argv[])
{
	int opt;
	artik_error ret = S_OK;

	if (!artik_is_module_available(ARTIK_MODULE_WIFI)) {
		fprintf(stdout,
			"TEST: Wifi module is not available,"\
			" skipping test...\n");
		return -1;
	}

	while ((opt = getopt(argc, argv, "s:p:")) != -1) {
		switch (opt) {
		case 's':
			strncpy(ssid, optarg, MAX_PARAM_LEN);
			fprintf(stdout, "ssid = %s\n", ssid);
			break;
		case 'p':
			strncpy(psk, optarg, MAX_PARAM_LEN);
			fprintf(stdout, "psk = %s\n", psk);
			break;
		default:
			printf("Usage: wifi-test [-s <ssid>] [-p <psk>] \r\n");
			return 0;
		}
	}

	ret = test_wifi_scan();
	if (ret != S_OK)
		goto exit;

	ret = test_wifi_connect();

exit:
	return (ret == S_OK) ? 0 : -1;
}
