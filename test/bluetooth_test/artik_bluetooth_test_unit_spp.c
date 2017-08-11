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
#include <time.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>

#include <CUnit/Basic.h>

#define BT_ADDRESS_LEN	18
#define MAX_PACKET_SIZE 1024

static char remote_mac_addr[BT_ADDRESS_LEN];
static artik_error err;

static int init_suite1(void)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

static int clean_suite1(void)
{
	fprintf(stdout, "\nclean_suite1\n");
	return 0;
}

static artik_error profile_init(
	artik_bt_spp_profile_option * profile_option)
{
	profile_option->name = "Artik SPP Loopback";
	profile_option->service = "spp char loopback";
	profile_option->role = "client";
	profile_option->channel = 22;
	profile_option->PSM = 3;
	profile_option->require_authentication = 1;
	profile_option->auto_connect = 1;
	profile_option->version = 10;
	profile_option->features = 20;

	return S_OK;
}

static void spp_register_profile_test(void)
{
	artik_error ret = S_OK;

	static artik_bt_spp_profile_option profile_option = {
		NULL, NULL, NULL, 0, 0,
		false, false, false, 0, 0};
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");

	ret = bt->spp_register_profile(&profile_option);
	CU_ASSERT(ret != S_OK);

	ret = bt->spp_unregister_profile();
	CU_ASSERT(ret != S_OK);

	ret = profile_init(&profile_option);
	ret = bt->spp_register_profile(&profile_option);
	CU_ASSERT(ret == S_OK);

	ret = bt->spp_unregister_profile();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void spp_unregister_profile_test(void)
{
	artik_error ret = S_OK;
	static artik_bt_spp_profile_option profile_option;

	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");

	ret = bt->spp_unregister_profile();
	CU_ASSERT(ret != S_OK);

	ret = profile_init(&profile_option);
	ret = bt->spp_register_profile(&profile_option);
	CU_ASSERT(ret == S_OK);

	ret = bt->spp_unregister_profile();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void release_handler(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)user_data;

	err = S_OK;
	loop->quit();
}

static void new_connection_handler(char *device_path,
		int fd, int version,
		int features, void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)user_data;

	err = S_OK;
	loop->quit();
}

static void request_disconnect_handler(char *device_path,
		void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)user_data;

	err = S_OK;
	loop->quit();
}

static int on_keyboard_received(int fd,
		enum watch_io id, void *user_data)
{
	char buffer[MAX_PACKET_SIZE] = {0};
	char *buf;
	artik_loop_module *loop = (artik_loop_module *)user_data;

	buf = fgets(buffer, MAX_PACKET_SIZE, stdin);
	while (buf != NULL)
		buf = fgets(buffer, MAX_PACKET_SIZE, stdin);
	fprintf(stdout, "Keyboard quit\n");
	err = E_BT_ERROR;
	loop->quit();
	return 1;
}

static void spp_set_callback_test(void)
{
	artik_error ret = S_OK;

	err = S_OK;
	int watch_id = 0;
	static artik_bt_spp_profile_option profile_option;

	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

	ret = profile_init(&profile_option);
	ret = bt->spp_register_profile(&profile_option);
	CU_ASSERT(ret == S_OK);

	ret = bt->spp_set_callback(
		release_handler,
		new_connection_handler,
		request_disconnect_handler,
		(void *)loop);
	CU_ASSERT(ret == S_OK);

	ret = bt->connect(remote_mac_addr);
	CU_ASSERT(ret == S_OK);

	loop->add_fd_watch(STDIN_FILENO,
			(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP
			| WATCH_IO_NVAL),
			on_keyboard_received, (void *)loop, &watch_id);
	loop->run();
	CU_ASSERT(err == S_OK);

	ret = bt->disconnect(remote_mac_addr);
	loop->run();
	CU_ASSERT(err == S_OK);

	ret = bt->spp_unregister_profile();
	CU_ASSERT(ret == S_OK);

	loop->run();
	CU_ASSERT(err == S_OK);

	loop->remove_fd_watch(watch_id);
	artik_release_api_module(bt);
	artik_release_api_module(loop);
}

artik_error cunit_add_suite(CU_pSuite *psuite)
{
	CU_add_test(*psuite, "spp_register_profile_test",
				spp_register_profile_test);
	CU_add_test(*psuite, "spp_unregister_profile_test",
				spp_unregister_profile_test);
	CU_add_test(*psuite, "spp_set_callback_test",
				spp_set_callback_test);

	return S_OK;
}

artik_error cunit_init(CU_pSuite *psuite)
{
	artik_error ret = S_OK;

	fprintf(stdout, "cunit init!\n");

	if (CU_initialize_registry() != CUE_SUCCESS)
		return CU_get_error();
	*psuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
	if (*psuite == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	ret = cunit_add_suite(psuite);

	return ret;
}

artik_error remote_info_get(void)
{
	int ret = 0;

	fprintf(stdout, "remote device mac address: ");
	ret = fscanf(stdin, "%s", remote_mac_addr);
	if (ret == -1)
		return E_INVALID_VALUE;
	fprintf(stdout, "remote address: %s-%zu\n",
		remote_mac_addr, strlen(remote_mac_addr));

	return S_OK;
}

int main(void)
{
	artik_error ret = S_OK;
	CU_pSuite pSuite = NULL;
	artik_bluetooth_module *bt_main = NULL;
	artik_loop_module *loop_main = NULL;

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		fprintf(stdout, "Bluetooth module not available!\n");
		goto loop_quit;
	}

	bt_main = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	loop_main = (artik_loop_module *)
		artik_request_api_module("loop");
	if (!bt_main || !loop_main)
		goto loop_quit;

	ret = cunit_init(&pSuite);
	if (ret != S_OK) {
		fprintf(stdout, "cunit init error!\n");
		goto loop_quit;
	}
	fprintf(stdout, "cunit init success!\n");

	ret = remote_info_get();

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

loop_quit:
	CU_cleanup_registry();
	if (bt_main)
		artik_release_api_module(bt_main);
	if (loop_main)
		artik_release_api_module(loop_main);

	return S_OK;
}


