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
#include <unistd.h>
#include <string.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>

#define MAX_BDADDR_LEN	17

static artik_bluetooth_module *bt;

void print_devices(artik_bt_device *devices, int num)
{
	int i = 0, j = 0;

	for (i = 0; i < num; i++) {
		fprintf(stdout, "Address: %s\n",
			devices[i].remote_address ? devices[i].remote_address : "(null)");
		fprintf(stdout, "Name: %s\n",
			devices[i].remote_name ? devices[i].remote_name : "(null)");
		fprintf(stdout, "RSSI: %d\n", devices[i].rssi);
		fprintf(stdout, "Bonded: %s\n",
			devices[i].is_bonded ? "true" : "false");
		fprintf(stdout, "Connected: %s\n",
			devices[i].is_connected ? "true" : "false");
		fprintf(stdout, "Authorized: %s\n",
			devices[i].is_authorized ? "true" : "false");
		fprintf(stdout, "Class:\n");
		fprintf(stdout, "\tMajor:0x%02x\n", devices[i].cod.major);
		fprintf(stdout, "\tMinor:0x%02x\n", devices[i].cod.minor);
		fprintf(stdout, "\tService:0x%04x\n",
			devices[i].cod.service_class);
		if (devices[i].uuid_length > 0) {
			fprintf(stdout, "UUIDs:\n");
			for (j = 0; j < devices[i].uuid_length; j++) {
				fprintf(stdout, "\t%s [%s]\n",
					devices[i].uuid_list[j].uuid_name,
					devices[i].uuid_list[j].uuid);
			}
		}

		fprintf(stdout, "\n");
	}
}

static void scan_callback(artik_bt_event event, void *data, void *user_data)
{
	artik_bt_device *dev = (artik_bt_device *) data;

	print_devices(dev, 1);
}

static void on_timeout_callback(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *) user_data;

	fprintf(stdout, "TEST: %s stop scanning, exiting loop\n", __func__);

	loop->quit();
}

artik_error test_bluetooth_scan(void)
{
	artik_loop_module *loop = (artik_loop_module *)artik_request_api_module("loop");
	artik_error ret = S_OK;
	int timeout_id = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = bt->set_callback(BT_EVENT_SCAN, scan_callback, NULL);
	if (ret != S_OK)
		goto exit;
	ret = bt->start_scan();
	if (ret != S_OK)
		goto exit;
	loop->add_timeout_callback(&timeout_id, 10000, on_timeout_callback,
				   (void *)loop);
	loop->run();

exit:
	bt->stop_scan();
	bt->unset_callback(BT_EVENT_SCAN);

	fprintf(stdout, "TEST: %s %s\n", __func__,
		(ret == S_OK) ? "succeeded" : "failed");

	artik_release_api_module(loop);

	return ret;
}

static artik_error test_bluetooth_devices(void)
{
	artik_error ret = S_OK;
	artik_bt_device *devices = NULL;
	int num = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	fprintf(stdout, "TEST: %s - list devices\n", __func__);
	ret = bt->get_devices(&devices, &num);
	if (ret != S_OK)
		goto exit;
	print_devices(devices, num);
	bt->free_devices(devices, num);
	devices = NULL;
	num = 0;

	fprintf(stdout, "TEST: %s - list paired devices\n", __func__);
	ret = bt->get_paired_devices(&devices, &num);
	if (ret != S_OK)
		goto exit;
	print_devices(devices, num);
	bt->free_devices(devices, num);
	devices = NULL;
	num = 0;

	fprintf(stdout, "TEST: %s - list connected devices\n", __func__);
	ret = bt->get_connected_devices(&devices, &num);
	if (ret != S_OK)
		goto exit;
	print_devices(devices, num);
	bt->free_devices(devices, num);
	devices = NULL;
	num = 0;

exit:
	fprintf(stdout, "TEST: %s %s\n", __func__,
		(ret == S_OK) ? "succeeded" : "failed");

	if (devices && (num > 0))
		bt->free_devices(devices, num);

	return ret;
}

static void on_scan(void *data, void *user_data)
{
	artik_bt_device *dev = (artik_bt_device *)data;
	char *target_address = (char *)user_data;

	fprintf(stdout, "Address: %s\n", dev->remote_address);

	if (strncasecmp(target_address, dev->remote_address, MAX_BDADDR_LEN) == 0) {
		bt->stop_scan();
		bt->start_bond(target_address);
	}
}

static void on_bond(void *data, void *user_data)
{
	char *remote_address = (char *)user_data;
	bool paired = *(bool *)data;

	fprintf(stdout, "%s %s\n", __func__, paired ? "Paired" : "UnPaired");
	bt->connect(remote_address);
}

static void on_connect(void *data, void *user_data)
{
	bool connected = *(bool *)data;

	fprintf(stdout, "%s %s\n", __func__,
		connected ? "Connected" : "Disconnected");
}

static void on_proximity(void *data, void *user_data)
{
	artik_bt_gatt_data *bt_pxp_data = (artik_bt_gatt_data *) data;

	fprintf(stdout, "%s : property [%s] value [%s]\n", __func__,
		bt_pxp_data->key, bt_pxp_data->value);
}

static void user_callback(artik_bt_event event, void *data, void *user_data)
{
	switch (event) {
	case BT_EVENT_SCAN:
		on_scan(data, user_data);
		break;
	case BT_EVENT_BOND:
		on_bond(data, user_data);
		break;
	case BT_EVENT_CONNECT:
		on_connect(data, user_data);
		break;
	case BT_EVENT_PROXIMITY:
		on_proximity(data, user_data);
		break;
	default:
		break;
	}
}

static void on_connect_timeout(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)artik_request_api_module("loop");

	fprintf(stdout, "TEST: %s reached timeout\n", __func__);

	bt->unset_callback(BT_EVENT_PROXIMITY);
	bt->unset_callback(BT_EVENT_SCAN);
	bt->unset_callback(BT_EVENT_CONNECT);

	loop->quit();

	artik_release_api_module(loop);
}

static artik_error test_bluetooth_connect(const char *target)
{
	artik_loop_module *loop = (artik_loop_module *)artik_request_api_module("loop");
	artik_error ret = S_OK;
	int timeout_id = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret =
	    bt->set_callback(BT_EVENT_SCAN, user_callback, (void *)target);
	if (ret != S_OK)
		goto exit;
	ret =
	    bt->set_callback(BT_EVENT_BOND, user_callback, (void *)target);
	if (ret != S_OK)
		goto exit;
	ret =
	    bt->set_callback(BT_EVENT_CONNECT, user_callback, (void *)target);
	if (ret != S_OK)
		goto exit;
	ret =
	    bt->set_callback(BT_EVENT_PROXIMITY, user_callback, (void *)target);
	if (ret != S_OK)
		goto exit;
	ret = bt->start_scan();
	if (ret != S_OK)
		goto exit;

	loop->add_timeout_callback(&timeout_id, 60000, on_connect_timeout, (void *)target);
	loop->run();

exit:
	fprintf(stdout, "TEST: %s %s\n", __func__,
		(ret == S_OK) ? "succeeded" : "failed");

	artik_release_api_module(loop);

	return ret;
}

static artik_error test_bluetooth_disconnect_devices(void)
{
	artik_error ret = S_OK;
	artik_bt_device *devices = NULL;
	int num = 0;
	int i;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	fprintf(stdout, "TEST: %s - remove unpaired devices\n", __func__);
	ret = bt->remove_unpaired_devices();
	if (ret != S_OK)
		goto exit;

	fprintf(stdout, "TEST: %s - remove connected devices\n", __func__);
	ret = bt->get_connected_devices(&devices, &num);
	if (ret != S_OK)
		goto exit;

	for (i = 0; i < num; i++) {
		ret = bt->disconnect(devices[i].remote_address);
		if (ret != S_OK)
			goto exit;
	}

	bt->free_devices(devices, num);
	devices = NULL;
	num = 0;

	fprintf(stdout, "TEST: %s - remove paired devices\n", __func__);
	ret = bt->get_paired_devices(&devices, &num);
	if (ret != S_OK)
		goto exit;

	for (i = 0; i < num; i++) {
		ret = bt->remove_device(devices[i].remote_address);
		if (ret != S_OK)
			goto exit;
	}

	bt->free_devices(devices, num);
	devices = NULL;
	num = 0;

exit:
	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" : "failed");

	return ret;
}

int main(int argc, char *argv[])
{
	int opt;
	artik_error ret = S_OK;
	char target_address[MAX_BDADDR_LEN+1] = "";

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		fprintf(stdout,
			"TEST: Bluetooth module is not available, skipping test...\n");
		return -1;
	}

	while ((opt = getopt(argc, argv, "t:")) != -1) {
		switch (opt) {
		case 't':
			strncpy(target_address, optarg, MAX_BDADDR_LEN);
			break;
		default:
			printf("Usage: bluetooth-test -t <target BDADDR to connect to>\r\n");
			return 0;
		}
	}

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	if (!bt) {
		fprintf(stdout,
			"TEST: Failed tor request bluetooth module skipping test...\n");
		return -1;
	}


	ret = test_bluetooth_scan();
	if (ret != S_OK)
		goto exit;

	/* Only call this test if a target address was provided */
	if (strncmp(target_address, "", MAX_BDADDR_LEN) != 0) {
		ret = test_bluetooth_connect(target_address);
		if (ret != S_OK)
			goto exit;
	}

	ret = test_bluetooth_devices();
	if (ret != S_OK)
		goto exit;

	ret = test_bluetooth_disconnect_devices();
	if (ret != S_OK)
		goto exit;

	ret = test_bluetooth_devices();
	if (ret != S_OK)
		goto exit;

exit:
	artik_release_api_module(bt);
	return (ret == S_OK) ? 0 : -1;
}
