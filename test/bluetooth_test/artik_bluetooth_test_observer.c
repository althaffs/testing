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

#include <artik_module.h>
#include <artik_bluetooth.h>
#include <artik_loop.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static artik_loop_module *loop;

static void on_scan(artik_bt_event event, void *data, void *user_data)
{
	int i;
	artik_bt_device dev = *(artik_bt_device *)data;

	printf("[advertiser: %s]\n", dev.remote_address);

	for (i = 0; i < dev.uuid_length; i++)
		printf("%s\n", dev.uuid_list[i].uuid);

	if (dev.manufacturer_data_len > 0) {
		printf("manufacturer name: %s\n", dev.manufacturer_name);
		printf("manufacturer data: ");
		for (i = 0; i < dev.manufacturer_data_len; i++)
			printf("0x%02X ", dev.manufacturer_data[i]);
		printf("\n");
	}

	if (dev.svc_data_len > 0) {
		printf("service uuid: %s\n", dev.svc_uuid);
		printf("service data: ");
		for (i = 0; i < dev.svc_data_len; i++)
			printf("0x%02X ", dev.svc_data[i]);
		printf("\n");
	}
}

static int on_signal(void *user_data)
{
	loop->quit();

	return true;
}

int main(int argc, char *argv[])
{
	artik_bluetooth_module *bt;
	artik_bt_scan_filter filter = {
		.type = 0,
		.uuid_list = NULL,
		.uuid_length = 0,
		.rssi = -100
	};
	int opt;

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	filter.type = BT_SCAN_LE;
	filter.rssi = -90;

	while ((opt = getopt(argc, argv, "u:")) != -1) {
		switch (opt) {
		case 'u':
			filter.uuid_length = 1;
			filter.uuid_list = (artik_bt_uuid *)malloc(
					sizeof(artik_bt_uuid) * filter.uuid_length);
			filter.uuid_list[0].uuid = optarg;
			break;
		default:
			printf("Usage: bluetooth-test-observer -u <target UUID>\n");
			return 0;
		}
	}

	bt->set_callback(BT_EVENT_SCAN, on_scan, NULL);
	if (bt->set_scan_filter(&filter) != 0) {
		printf("Error: invalid UUID\n");
		return -1;
	}
	bt->start_scan();

	loop->add_signal_watch(SIGINT, on_signal, NULL, NULL);
	loop->run();

	artik_release_api_module(bt);
	artik_release_api_module(loop);

	return 0;
}
