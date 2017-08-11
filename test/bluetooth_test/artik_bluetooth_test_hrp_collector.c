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

#define HEART_RATE_SERVICE "0000180d-0000-1000-8000-00805f9b34fb"
#define HEART_RATE_SENSOR_LOCATION "00002a38-0000-1000-8000-00805f9b34fb"
#define HEART_RATE_MEASUREMENT "00002a37-0000-1000-8000-00805f9b34fb"

static artik_bluetooth_module *bt;
static artik_loop_module *loop;
static char remote_address[18];

static void print_property(artik_bt_gatt_char_properties prop)
{
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
}

static void print_location(unsigned char *byte)
{
	char *location;

	switch (byte[0]) {
	case 0:
		location = "Other";
		break;
	case 1:
		location = "Chest";
		break;
	case 2:
		location = "Wrist";
		break;
	case 3:
		location = "Finger";
		break;
	case 4:
		location = "Hand";
		break;
	case 5:
		location = "Ear Lobe";
		break;
	case 6:
		location = "Foot";
		break;
	default:
		location = "None";
		break;
	}
	fprintf(stdout, "Sensor location: %s\n", location);
}

static void on_scan(artik_bt_event event, void *data, void *user_data)
{
	artik_bt_device dev = *(artik_bt_device *)data;

	fprintf(stdout, "Address: %s\n", dev.remote_address);
	strcpy(remote_address, dev.remote_address);

	bt->stop_scan();
	bt->connect(remote_address);
}

static void on_connect(artik_bt_event event, void *data, void *user_data)
{
	fprintf(stdout, "%s %s\n", __func__, *(bool *)data ? "true" : "false");
	fprintf(stdout, "waiting for gatt properties\n");
}

static void on_gatt_property(artik_bt_event event, void *data, void *user_data)
{
	int uuid_len = 0, byte_len = 0;
	int i = 0;
	unsigned char *byte;

	artik_bt_uuid *uuid_list;
	artik_bt_gatt_char_properties prop;

	fprintf(stdout, "Get GATT Services\n");
	bt->gatt_get_service_list(remote_address, &uuid_list, &uuid_len);
	for (i = 0; i < uuid_len; i++) {
		fprintf(stdout, "%s [%s]\n", uuid_list[i].uuid, uuid_list[i].uuid_name);
		free(uuid_list[i].uuid);
		free(uuid_list[i].uuid_name);
	}
	free(uuid_list);

	fprintf(stdout, "Get GATT Chracteristics\n");
	bt->gatt_get_characteristic_list(remote_address, HEART_RATE_SERVICE,
			&uuid_list, &uuid_len);
	for (i = 0; i < uuid_len; i++) {
		fprintf(stdout, "%s [%s]\n", uuid_list[i].uuid, uuid_list[i].uuid_name);
		free(uuid_list[i].uuid);
		free(uuid_list[i].uuid_name);
	}
	free(uuid_list);

	fprintf(stdout, "Get heart rate measurement Properties\n");
	prop = 0;
	if (bt->gatt_get_char_properties(remote_address, HEART_RATE_SERVICE, HEART_RATE_MEASUREMENT,
			&prop) == 0) {
		print_property(prop);
		if (prop & BT_GATT_CHAR_PROPERTY_NOTIFY) {
			if (bt->gatt_start_notify(remote_address, HEART_RATE_SERVICE, HEART_RATE_MEASUREMENT)
					!= 0) {
				fprintf(stdout, "fail to start notify\n");
			}
		}
	} else {
		fprintf(stdout, "not found heart rate measurement characteristic\n");
	}

	fprintf(stdout, "Get heart rate sensor location Properties\n");
	prop = 0;
	if (bt->gatt_get_char_properties(remote_address, HEART_RATE_SERVICE, HEART_RATE_SENSOR_LOCATION,
			&prop) == 0) {
		print_property(prop);
		if (prop & BT_GATT_CHAR_PROPERTY_READ) {
			if (bt->gatt_char_read_value(remote_address,
					HEART_RATE_SERVICE, HEART_RATE_SENSOR_LOCATION, &byte, &byte_len) == 0) {
				print_location(byte);
				free(byte);
			}
		}
	} else {
		fprintf(stdout, "not found heart rate sensor location characteristic\n");
	}
}

static void on_hr_received(artik_bt_event event, void *data, void *user_data)
{
	int i = 0;
	artik_bt_hrp_data measure = *(artik_bt_hrp_data *)data;

	fprintf(stdout, "BPM: %d\n", measure.bpm);
	fprintf(stdout, "Energy: %d\n", measure.energy);
	fprintf(stdout, "Contact: %s\n", measure.contact ? "true" : "false");
	for (i = 0; i < measure.interval_count; i++)
		fprintf(stdout, "interval[%d]: %d\n", i, measure.interval[i]);
}

void set_user_callbacks(void)
{
	bt->set_callback(BT_EVENT_SCAN, on_scan, NULL);
	bt->set_callback(BT_EVENT_CONNECT, on_connect, NULL);
	bt->set_callback(BT_EVENT_GATT_PROPERTY, on_gatt_property, NULL);
	bt->set_callback(BT_EVENT_PF_HEARTRATE, on_hr_received, NULL);
}

void on_timeout(void *user_data)
{
	fprintf(stdout, "%s\n", __func__);

	if (bt->gatt_stop_notify(remote_address, HEART_RATE_SERVICE, HEART_RATE_MEASUREMENT) != 0)
		fprintf(stdout, "fail to stop notify\n");

	/* removing filter */
	bt->set_scan_filter(NULL);

	loop->quit();
}

static int on_signal(void *user_data)
{
	loop->quit();

	return true;
}

int main(int argc, char *argv[])
{
	int id;

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	artik_bt_scan_filter filter = {
		.type = 0,
		.uuid_list = NULL,
		.uuid_length = 0,
		.rssi = -100
	};

	set_user_callbacks();

	filter.type = BT_SCAN_LE;
	filter.uuid_length = 1;
	filter.uuid_list = (artik_bt_uuid *)malloc(
			sizeof(artik_bt_uuid) * filter.uuid_length);
	filter.uuid_list[0].uuid = HEART_RATE_SERVICE;
	filter.rssi = -90;

	artik_error err = bt->set_scan_filter(&filter);

	if (err != S_OK) {
		printf("TEST: failed to set the device discovery filter [%d]\n", err);
		return -1;
	}

	bt->start_scan();

	loop->add_timeout_callback(&id, 50000, on_timeout, NULL);
	loop->add_signal_watch(SIGINT, on_signal, NULL, NULL);
	loop->run();

	artik_release_api_module(bt);
	artik_release_api_module(loop);

	free(filter.uuid_list);

	return 0;
}
