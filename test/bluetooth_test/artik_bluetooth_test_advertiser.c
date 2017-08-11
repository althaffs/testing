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

#define SERVICE_UUID_16_BIT "ffff"
#define SERVICE_UUID_32_BIT "0000180d"
#define SERVICE_UUID_128_BIT "00001802-0000-1000-8000-00805f9b34fb"

#define SERVICE_DATA_UUID_16_BIT "1234"

static artik_loop_module *loop;

static void set_advertisement(artik_bt_advertisement *adv)
{
	adv->type = BT_ADV_TYPE_BROADCAST;
	adv->svc_uuid_len = 3;
	adv->svc_uuid = (const char **)malloc(sizeof(SERVICE_UUID_128_BIT)
			* adv->svc_uuid_len);
	adv->svc_uuid[0] = SERVICE_UUID_16_BIT;
	adv->svc_uuid[1] = SERVICE_UUID_32_BIT;
	adv->svc_uuid[2] = SERVICE_UUID_128_BIT;

	adv->mfr_id = 0x0075; // Samsung Electronics Co. Ltd.
	adv->mfr_data_len = 4;
	adv->mfr_data = (unsigned char *)malloc(4);
	adv->mfr_data[0] = 0x01;
	adv->mfr_data[1] = 0x02;
	adv->mfr_data[2] = 0x03;
	adv->mfr_data[3] = 0x04;

	adv->svc_id = SERVICE_DATA_UUID_16_BIT;
	adv->svc_data_len = 4;
	adv->svc_data = (unsigned char *)malloc(4);
	adv->svc_data[0] = 0x05;
	adv->svc_data[1] = 0x06;
	adv->svc_data[2] = 0x07;
	adv->svc_data[3] = 0x08;

	adv->tx_power = true;
}

static int on_signal(void *user_data)
{
	loop->quit();

	return true;
}

int main(void)
{
	artik_bluetooth_module *bt;
	artik_bt_advertisement adv;
	int adv_id;

	bt = (artik_bluetooth_module *)artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	fprintf(stdout, "start broadcasting with random address\n");
	fprintf(stdout, "service includes:\n %s (16bit UUID)\n %s (32bit UUID)"
			"\n %s (128bit UUID)\n", SERVICE_UUID_16_BIT, SERVICE_UUID_32_BIT,
			SERVICE_UUID_128_BIT);

	set_advertisement(&adv);
	bt->register_advertisement(&adv, &adv_id);

	loop->add_signal_watch(SIGINT, on_signal, NULL, NULL);
	loop->run();

	bt->unregister_advertisement(adv_id);

	artik_release_api_module(bt);
	artik_release_api_module(loop);
	return 0;
}
