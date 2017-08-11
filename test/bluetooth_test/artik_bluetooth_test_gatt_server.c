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

#define IMMEDIATE_ALERT_SERVICE "00001802-0000-1000-8000-00805f9b34fb"
#define BATTER_LEVEL_SERVICE "0000180f-0000-1000-8000-00805f9b34fb"
#define BATTERY_LEVEL_CHARACTERISTIC "00002a19-0000-1000-8000-00805f9b34fb"
#define TEST_DESCRIPTOR "ffffffff-ffff-ffff-ffff-abcdabcdabcd"

#define TEST_DESCRIPTOR_VALUE "artik-test-descriptor-value"

static int battery_char_props = BT_GATT_CHAR_PROPERTY_READ
		| BT_GATT_CHAR_PROPERTY_WRITE
		| BT_GATT_CHAR_PROPERTY_NOTIFY;

static int test_desc_props = BT_GATT_DESC_PROPERTY_READ;

static artik_bluetooth_module *bt;
static artik_loop_module *loop;
static int svc_id, char_id, desc_id, adv_id, loop_id;

static int _periodic_callback(void *user_data)
{
	static unsigned char byte = 100;

	if (byte-- <= 0) {
		loop->quit();
		return 0;
	}

	fprintf(stdout, "battery level: %d\n", byte);
	bt->gatt_notify(svc_id, char_id, &byte, sizeof(byte));

	return 1;
}

static void on_gatt_notify(bool state, void *user_data)
{
	fprintf(stdout, "received gatt request notify\n");

	if (state)
		loop->add_periodic_callback(&loop_id, 1000, _periodic_callback, NULL);

	else
		loop->remove_periodic_callback(loop_id);
}

static void set_advertisement(artik_bt_advertisement *adv)
{
	adv->type = BT_ADV_TYPE_PERIPHERAL;
	adv->svc_uuid_len = 2;
	adv->svc_uuid = (const char **)malloc(
			sizeof(IMMEDIATE_ALERT_SERVICE) * adv->svc_uuid_len);
	adv->svc_uuid[0] = IMMEDIATE_ALERT_SERVICE;
	adv->svc_uuid[1] = BATTER_LEVEL_SERVICE;
}

static int on_signal(void *user_data)
{
	loop->quit();

	return true;
}

int main(void)
{
	artik_bt_advertisement adv = {0};
	artik_bt_gatt_service svc = {0};
	artik_bt_gatt_chr chr = {0};
	artik_bt_gatt_desc desc = {0};

	fprintf(stdout, "start gatt server\n");

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	set_advertisement(&adv);
	bt->register_advertisement(&adv, &adv_id);
	fprintf(stdout, "start advertising\n");

	svc.uuid = BATTER_LEVEL_SERVICE;
	svc.primary = true;
	bt->gatt_add_service(svc, &svc_id);

	chr.uuid = BATTERY_LEVEL_CHARACTERISTIC;
	chr.property = battery_char_props;
	chr.length = 10;
	chr.value = (unsigned char *)malloc(chr.length);
	for (int i = 0; i < chr.length; i++)
		chr.value[i] = i;
	bt->gatt_add_characteristic(svc_id, chr, &char_id);

	desc.uuid = TEST_DESCRIPTOR;
	desc.property = test_desc_props;
	desc.length = sizeof(TEST_DESCRIPTOR_VALUE);
	desc.value = (unsigned char *)malloc(sizeof(desc.value) * desc.length + 1);
	strcpy((char *)desc.value, TEST_DESCRIPTOR_VALUE);
	bt->gatt_add_descriptor(svc_id, char_id, desc, &desc_id);
	bt->gatt_set_char_on_notify_request(svc_id, char_id, on_gatt_notify, NULL);

	bt->gatt_register_service(svc_id);
	fprintf(stdout, "gatt service registered\n");

	loop->add_signal_watch(SIGINT, on_signal, NULL, NULL);
	loop->run();

	free(desc.value);
	bt->unregister_advertisement(adv_id);
	bt->gatt_unregister_service(svc_id);

	artik_release_api_module(bt);
	artik_release_api_module(loop);
	return 1;
}
