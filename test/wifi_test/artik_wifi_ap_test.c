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

#define MAX_PARAM_LEN	64

static char ssid[MAX_PARAM_LEN];
static char psk[MAX_PARAM_LEN];
static unsigned int channel = 1;
static unsigned int encryption = 0;

artik_error test_wifi_softap(void)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_wifi_module *wifi = (artik_wifi_module *)
					artik_request_api_module("wifi");
	artik_error ret;

	ret = wifi->init(ARTIK_WIFI_MODE_AP);
	if (ret != S_OK)
		goto exit;
	ret = wifi->start_ap(ssid, psk,  channel, encryption);

exit:
	artik_release_api_module(loop);
	artik_release_api_module(wifi);
	fprintf(stdout, "TEST: %s %s\n[%s]\n", __func__,
		(ret == S_OK) ? "succeeded" : "failed", error_msg(ret));
	return ret;
}

static void usage(void)
{
	fprintf(stdout, "Usage: wifi-ap-test [-s <ssid>] [-p <psk>]"\
		" [-c <channel>] [-e open|wpa2]\r\n");
}

int main(int argc, char *argv[])
{
	int opt;
	artik_error ret = E_BAD_ARGS;

	if (!artik_is_module_available(ARTIK_MODULE_WIFI)) {
		fprintf(stdout,
			"TEST: Wifi module is not available,"\
			" skipping test...\n");
		return -1;
	}

	memset(ssid, 0, MAX_PARAM_LEN);
	memset(psk, 0, MAX_PARAM_LEN);
	while ((opt = getopt(argc, argv, "s:p:c:e:")) != -1) {
		switch (opt) {
		case 's':
			strncpy(ssid, optarg, MAX_PARAM_LEN);
			fprintf(stdout, "ssid = %s\n", ssid);
			break;
		case 'p':
			strncpy(psk, optarg, MAX_PARAM_LEN);
			fprintf(stdout, "psk = %s\n", psk);
			break;
		case 'c':
			channel = atoi(optarg);
			fprintf(stdout, "chan = %d\n", channel);
			break;
		case 'e':
			if (!strncmp(optarg, "open", 4)) {
				encryption = WIFI_ENCRYPTION_OPEN;
			} else if (!strncmp(optarg, "wpa2", 4)) {
				encryption = WIFI_ENCRYPTION_WPA2;
			} else {
				fprintf(stdout, "%s is not a valid"\
					" encryption option\n", optarg);
				usage();
				return -1;
			}
			fprintf(stdout, "encryption = %d\n", encryption);
			break;
		default:
			return 0;
		}
	}

	if ((ssid[0] != 0 && psk[0] != 0 &&
			(encryption == WIFI_ENCRYPTION_WPA2)) ||
	    (ssid[0] != 0 && (encryption == WIFI_ENCRYPTION_OPEN)))	 {
		ret = test_wifi_softap();
		if (ret == E_ACCESS_DENIED)
			printf("'hostapd' should be start for modifying"\
				" its configuration.\n");
	}

	return (ret == S_OK) ? 0 : -1;
}
