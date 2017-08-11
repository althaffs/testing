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
#include <signal.h>

static artik_bluetooth_module *bt;
static artik_loop_module *loop;
static char remote_address[18];

#define BATTER_LEVEL_SERVICE "0000180f-0000-1000-8000-00805f9b34fb"
#define CHAR_BATTERY_LEVEL "00002a19-0000-1000-8000-00805f9b34fb"
#define TEST_DESCRIPTOR "ffffffff-ffff-ffff-ffff-abcdabcdabcd"

static void on_scan(artik_bt_event event, void *data, void *user_data)
{
	int i;
	artik_bt_device dev = *(artik_bt_device *)data;

	fprintf(stdout, "found: %s\n", dev.remote_address);

	for (i = 0; i < dev.uuid_length; i++) {
		if (strcmp(dev.uuid_list[i].uuid, BATTER_LEVEL_SERVICE) == 0) {
			fprintf(stdout, "found: %s [%s]\n",
				dev.uuid_list[i].uuid_name, dev.uuid_list[i].uuid);

			strcpy(remote_address, dev.remote_address);
			bt->stop_scan();
			bt->connect(remote_address);
			break;
		}
	}
}

static void on_connect(artik_bt_event event, void *data, void *user_data)
{
	fprintf(stdout, "%s connect: %s\n",
		__func__, *(bool *)data ? "true" : "false");

	if (!(*(bool *)data))
		return;
}

static void on_gatt_property(artik_bt_event event, void *data, void *user_data)
{
	artik_bt_gatt_char_properties prop;
	unsigned char v[3] = {0xff, 0xff, 0xff};
	unsigned char *b;
	int i, len;

	fprintf(stdout, "%s\n", __func__);

	if (bt->gatt_get_char_properties(remote_address, BATTER_LEVEL_SERVICE, CHAR_BATTERY_LEVEL,
			&prop) == 0) {
		fprintf(stdout, "battery level characteristic Properties:\n");
		if (prop & BT_GATT_CHAR_PROPERTY_WRITE) {
			fprintf(stdout, "Write\n");
			bt->gatt_char_write_value(remote_address, BATTER_LEVEL_SERVICE,
					CHAR_BATTERY_LEVEL, v, sizeof(v));
		}
		if (prop & BT_GATT_CHAR_PROPERTY_READ) {
			fprintf(stdout, "Read\n");
			bt->gatt_char_read_value(remote_address, BATTER_LEVEL_SERVICE,
					CHAR_BATTERY_LEVEL, &b, &len);
			for (i = 0; i < len; i++)
				fprintf(stdout, "characteristic value[%d]: 0x%02X\n", i, b[i]);

			len = 0;
			bt->gatt_desc_read_value(remote_address, BATTER_LEVEL_SERVICE,
					CHAR_BATTERY_LEVEL, TEST_DESCRIPTOR, &b, &len);
			fprintf(stdout, "descriptor value:\n");
			for (i = 0; i < len; i++)
				fprintf(stdout, "%c", b[i]);
			fprintf(stdout, "\n");
		}
	} else {
		fprintf(stdout, "no battery level characteristic\n");
	}
}

static void set_user_callbacks(void)
{
	bt->set_callback(BT_EVENT_SCAN, on_scan, NULL);
	bt->set_callback(BT_EVENT_CONNECT, on_connect, NULL);
	bt->set_callback(BT_EVENT_GATT_PROPERTY, on_gatt_property, NULL);
}

static void on_timeout_callback(void *user_data)
{
	fprintf(stdout, "%s exiting loop\n", __func__);
	if (bt->is_connected(remote_address)) {
		fprintf(stdout, "disconnect %s\n", remote_address);
		bt->disconnect(remote_address);
	}
	loop->quit();
}

static int on_signal(void *user_data)
{
	loop->quit();

	return true;
}

int main(int argc, char *argv[])
{
	int id = 0;
	artik_bt_scan_filter filter = {
		.type = 0,
		.uuid_list = NULL,
		.uuid_length = 0,
		.rssi = -100
	};

	filter.type = BT_SCAN_LE;
	filter.uuid_length = 1;
	filter.uuid_list = (artik_bt_uuid *)malloc(
			sizeof(artik_bt_uuid) * filter.uuid_length);
	filter.uuid_list[0].uuid = BATTER_LEVEL_SERVICE;
	filter.rssi = -90;

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	set_user_callbacks();

	fprintf(stdout, "start scan\n");

	bt->remove_devices();
	bt->set_scan_filter(&filter);
	bt->start_scan();

	loop->add_timeout_callback(&id, 50000, on_timeout_callback, NULL);
	loop->add_signal_watch(SIGINT, on_signal, NULL, NULL);
	loop->run();

	if (bt->is_connected(remote_address)) {
		fprintf(stdout, "disconnect %s\n", remote_address);
		bt->disconnect(remote_address);
	}

	artik_release_api_module(bt);
	artik_release_api_module(loop);
	return 0;
}
