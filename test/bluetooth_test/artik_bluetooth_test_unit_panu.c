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
#define BUFFER_LEN		128
#define UUID			"nap"

static char remote_mac_addr[BT_ADDRESS_LEN];
static artik_error connect_status = S_OK;


static int init_suite1(void)
{
	printf("%s\n", __func__);
	return 0;
}

static int clean_suite1(void)
{
	printf("%s\n", __func__);
	return 0;
}

static artik_error _pan_init(void)
{
	int status = -1;

	status = system("systemctl stop connman");
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	return S_OK;
}

static void on_connect(artik_bt_event event, void *data, void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)user_data;
	bool connected = *(bool *)data;

	if (connected)
		connect_status = S_OK;
	else
		connect_status = E_BT_ERROR;
	loop->quit();
}

static int on_keyboard_received(int fd,
		enum watch_io id, void *user_data)
{
	char buffer[BUFFER_LEN];
	artik_loop_module *loop = (artik_loop_module *)user_data;

	fprintf(stdout, "press any key to quit\n");
	if (fgets(buffer, BUFFER_LEN, stdin) == NULL)
		return 1;
	fprintf(stdout, "keyboard quit: %s!\n", buffer);
	connect_status = E_BT_ERROR;
	loop->quit();
	return 1;
}

static artik_error _pan_start_connect(void)
{
	artik_error ret = S_OK;
	int watch_id = 0;
	char *network_interface = NULL;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
		artik_request_api_module("loop");

	ret = _pan_init();
	if (ret != S_OK)
		goto quit;

	ret = bt->set_callback(BT_EVENT_CONNECT,
		on_connect, (void *)loop);
	if (ret != S_OK)
		goto quit;

	ret = bt->pan_connect(remote_mac_addr,
		UUID, &network_interface);
	if ((ret != S_OK) || (!network_interface)) {
		ret = E_BT_ERROR;
		goto quit;
	}

	loop->add_fd_watch(STDIN_FILENO,
			(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP
			| WATCH_IO_NVAL),
			on_keyboard_received, (void *)loop, &watch_id);
	loop->run();

quit:
	if (watch_id)
		loop->remove_fd_watch(watch_id);
	artik_release_api_module(bt);
	artik_release_api_module(loop);
	return ret;
}

static void pan_connect_test(void)
{
	artik_error ret = S_OK;
	char *network_interface = NULL;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	const char *test_uuid = "test_uuid";

	ret = bt->pan_connect(remote_mac_addr,
		test_uuid, &network_interface);
	CU_ASSERT(ret != S_OK);

	ret = _pan_start_connect();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(connect_status == S_OK);

	ret = _pan_start_connect();
	CU_ASSERT(ret != S_OK);
	CU_ASSERT(connect_status == S_OK);

	ret = bt->pan_disconnect();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void pan_disconnect_test(void)
{
	artik_error ret = S_OK;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->pan_disconnect();
	CU_ASSERT(ret != S_OK);

	ret = _pan_start_connect();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(connect_status == S_OK);

	ret = bt->pan_disconnect();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void pan_get_connected_test(void)
{
	artik_error ret = S_OK;
	bool connected;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->pan_get_connected(&connected);
	CU_ASSERT(ret != S_OK);

	ret = _pan_start_connect();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(connect_status == S_OK);

	ret = bt->pan_get_connected(&connected);
	CU_ASSERT(ret == S_OK);

	ret = bt->pan_disconnect();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void pan_get_interface_test(void)
{
	artik_error ret = S_OK;
	char *network_interface = NULL;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->pan_get_interface(&network_interface);
	CU_ASSERT(ret != S_OK);

	ret = _pan_start_connect();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(connect_status == S_OK);

	ret = bt->pan_get_interface(&network_interface);
	CU_ASSERT(ret == S_OK);

	ret = bt->pan_disconnect();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void pan_get_uuid_test(void)
{
	artik_error ret = S_OK;
	char *network_uuid = NULL;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->pan_get_UUID(&network_uuid);
	CU_ASSERT(ret != S_OK);

	ret = _pan_start_connect();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(connect_status == S_OK);

	ret = bt->pan_get_UUID(&network_uuid);
	CU_ASSERT(ret == S_OK);

	ret = bt->pan_disconnect();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

artik_error cunit_add_suite(CU_pSuite *psuite)
{
	CU_add_test(*psuite, "pan_connect_test",
		pan_connect_test);
	CU_add_test(*psuite, "pan_disconnect_test",
		pan_disconnect_test);
	CU_add_test(*psuite, "pan_get_connected_test",
		pan_get_connected_test);
	CU_add_test(*psuite, "pan_get_interface_test",
		pan_get_interface_test);
	CU_add_test(*psuite, "pan_get_uuid_test",
		pan_get_uuid_test);

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
	fprintf(stdout, "remote address: %s-%zd\n",
		remote_mac_addr, strlen(remote_mac_addr));

	return S_OK;
}

int main(void)
{
	artik_error ret = S_OK;
	CU_pSuite pSuite = NULL;

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		fprintf(stdout, "Bluetooth module not available!\n");
		goto loop_quit;
	}

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
	return S_OK;
}

