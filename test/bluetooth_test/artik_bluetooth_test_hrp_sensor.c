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
#define HEART_RATE_MSRMT_CHAR "00002a37-0000-1000-8000-00805f9b34fb"
#define HEART_RATE_SNSR_LOC_CHAR "00002a38-0000-1000-8000-00805f9b34fb"

static int msrmt_char_props = BT_GATT_CHAR_PROPERTY_NOTIFY;
static int snsr_loc_char_props = BT_GATT_CHAR_PROPERTY_READ;

static artik_bluetooth_module *bt;
static artik_loop_module *loop;
static int svc_id, char_id1, char_id2, adv_id, loop_id;

static int _periodic_callback(void *user_data)
{
	static unsigned char byte[4] = {0};

	/* TODO: HRP specification implementation */
	byte[0] = 0x01;
	byte[1] = 0x02;
	byte[2] = 0x03;
	byte[3] = 0x04;

	bt->gatt_notify(svc_id, char_id1, byte, sizeof(byte));

	return 1;
}

static void on_gatt_notify(bool state, void *user_data)
{
	fprintf(stdout, "received notify with state = %d", state);
	if (state)
		loop->add_periodic_callback(&loop_id, 1000, _periodic_callback, NULL);
	else
		loop->remove_periodic_callback(loop_id);
}

static void set_advertisement(artik_bt_advertisement *adv)
{
	adv->type = BT_ADV_TYPE_PERIPHERAL;
	adv->svc_uuid_len = 1;
	adv->svc_uuid = (const char **)malloc(
			sizeof(HEART_RATE_SERVICE) * adv->svc_uuid_len);
	adv->svc_uuid[0] = HEART_RATE_SERVICE;
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

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	set_advertisement(&adv);
	bt->register_advertisement(&adv, &adv_id);
	fprintf(stdout, "start advertising\n");

	svc.uuid = HEART_RATE_SERVICE;
	svc.primary = true;
	bt->gatt_add_service(svc, &svc_id);

	chr.uuid = HEART_RATE_MSRMT_CHAR;
	chr.property = msrmt_char_props;
	bt->gatt_add_characteristic(svc_id, chr, &char_id1);

	chr.uuid = HEART_RATE_SNSR_LOC_CHAR;
	chr.property = snsr_loc_char_props;
	chr.length = 1;
	chr.value = (unsigned char *)malloc(chr.length);
	chr.value[0] = 0x06; // sensor location: foot
	bt->gatt_add_characteristic(svc_id, chr, &char_id2);
	bt->gatt_set_char_on_notify_request(svc_id, char_id1, on_gatt_notify, NULL);

	bt->gatt_register_service(svc_id);
	fprintf(stdout, "HRP service registered\n");

	loop->add_signal_watch(SIGINT, on_signal, NULL, NULL);
	loop->run();

	bt->unregister_advertisement(adv_id);
	bt->gatt_unregister_service(svc_id);

	artik_release_api_module(bt);
	artik_release_api_module(loop);
	return 0;
}
