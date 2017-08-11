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

#define SERVICE_IMMEDIATE_ALERT "00001802-0000-1000-8000-00805f9b34fb"
#define CHAR_ALERT_LEVEL "00002a06-0000-1000-8000-00805f9b34fb"
#define CHAR_BATTERY_LEVEL "00002a19-0000-1000-8000-00805f9b34fb"

static void on_scan(artik_bt_event event, void *data, void *user_data)
{
	int i;
	artik_bt_device dev = *(artik_bt_device *)data;

	fprintf(stdout, "found: %s\n", dev.remote_address);

	for (i = 0; i < dev.uuid_length; i++) {
		if (strcmp(dev.uuid_list[i].uuid, SERVICE_IMMEDIATE_ALERT) == 0) {
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

	fprintf(stdout, "%s\n", __func__);

	if (bt->gatt_get_char_properties(remote_address, SERVICE_IMMEDIATE_ALERT, CHAR_ALERT_LEVEL,
			&prop) == 0) {
		fprintf(stdout, "immediate Alert characteristic Properties: [\n");

		if (prop & BT_GATT_CHAR_PROPERTY_BROADCAST)
			fprintf(stdout, "Broadcast\n");
		if (prop & BT_GATT_CHAR_PROPERTY_READ)
			fprintf(stdout, "Read\n");
		if (prop & BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE)
			fprintf(stdout, "Write Without Response\n");
		if (prop & BT_GATT_CHAR_PROPERTY_WRITE)
			fprintf(stdout, "Write\n");
		if (prop & BT_GATT_CHAR_PROPERTY_NOTIFY)
			fprintf(stdout, "Notify\n");
		if (prop & BT_GATT_CHAR_PROPERTY_INDICATE)
			fprintf(stdout, "Indicate\n");
		if (prop & BT_GATT_CHAR_PROPERTY_SIGNED_WRITE)
			fprintf(stdout, "Authenticated Signed Writes\n");
		fprintf(stdout, "]\n");
	} else {
		fprintf(stdout, "no alert level characteristic\n");
	}

	if (bt->gatt_get_char_properties(remote_address, SERVICE_IMMEDIATE_ALERT, CHAR_BATTERY_LEVEL,
			&prop) == 0) {
		fprintf(stdout, "battery level characteristic Properties: [\n");
		if (prop & BT_GATT_CHAR_PROPERTY_NOTIFY) {
			fprintf(stdout, "Notify\n");
			bt->gatt_start_notify(remote_address, SERVICE_IMMEDIATE_ALERT, CHAR_BATTERY_LEVEL);
		}
		fprintf(stdout, "]\n");
	} else {
		fprintf(stdout, "no battery level characteristic\n");
	}
}

static void on_received(artik_bt_event event, void *data, void *user_data)
{
	int i = 0;
	artik_bt_gatt_data d = *(artik_bt_gatt_data *)data;

	if (strcasecmp(d.char_uuid, CHAR_BATTERY_LEVEL) != 0)
		return;

	fprintf(stdout, "received %dbytes: ", d.length);
	for (i = 0; i < d.length; i++)
		fprintf(stdout, "0x%02X", d.bytes[i]);
	fprintf(stdout, "\n");
}

static void set_user_callbacks(void)
{
	bt->set_callback(BT_EVENT_SCAN, on_scan, NULL);
	bt->set_callback(BT_EVENT_CONNECT, on_connect, NULL);
	bt->set_callback(BT_EVENT_GATT_PROPERTY, on_gatt_property, NULL);
	bt->set_callback(BT_EVENT_PF_CUSTOM, on_received, NULL);
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
	filter.uuid_list[0].uuid = SERVICE_IMMEDIATE_ALERT;
	filter.rssi = -90;

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	set_user_callbacks();

	fprintf(stdout, "this application was tested with HTC FETCH\n");
	fprintf(stdout, "start scan\n");

	bt->remove_devices();
	artik_error err = bt->set_scan_filter(&filter);

	if (err != S_OK) {
		printf("TEST: failed to set the device discovery filter [%d]\n", err);
		return -1;
	}

	bt->start_scan();

	loop->add_timeout_callback(&id, 50000, on_timeout_callback, NULL);
	loop->add_signal_watch(SIGINT, on_signal, NULL, NULL);
	loop->run();

	artik_release_api_module(bt);
	artik_release_api_module(loop);
	return 0;
}
