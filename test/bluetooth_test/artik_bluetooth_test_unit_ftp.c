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
static artik_error session_status;
static artik_error property_status;

static int init_suite1(void)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

static int clean_suite1(void)
{
	fprintf(stdout, "%s\n", __func__);
	return 0;
}

artik_error _ftp_object_search(const char *object_name,
	const char *object_type, char **user_data)
{
	artik_error ret;
	artik_bt_ftp_file *file_list = NULL;
	artik_bt_ftp_file *list = NULL;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->ftp_list_folder(&file_list);
	if (ret != S_OK)
		goto quit;

	list = file_list;
	while (list != NULL) {
		fprintf(stdout, "object: %s\n", list->file_name);
		if (!strncmp(list->file_type, object_type,
				strlen(object_type))) {
			if ((object_name && (!strncmp(list->file_name,
				object_name, strlen(object_name))))
				|| (!object_name)) {
				if (user_data) {
					*user_data = (char *)
						malloc(strlen(list->file_name) + 1);
					if (*user_data) {
						strncpy(*user_data, list->file_name,
							strlen(list->file_name));
						(*user_data)[strlen(list->file_name)] = '\0';
					}
				}
				ret = S_OK;
				goto quit;
			}
		}
		list = list->next_file;
	}
	ret = E_BT_ERROR;

quit:
	if (file_list) {
		while (file_list != NULL) {
			list = file_list;
			if (list->file_type)
				free(list->file_type);
			if (list->file_name)
				free(list->file_name);
			if (list->modified)
				free(list->modified);
			if (list->file_permission)
				free(list->file_permission);

			file_list = file_list->next_file;
			free(list);
		}
	}
	artik_release_api_module(bt);
	return ret;
}

static int _on_keyboard_received(int fd,
		enum watch_io id, void *user_data)
{
	char buffer[MAX_PACKET_SIZE];
	char *buf;
	artik_loop_module *loop = (artik_loop_module *)user_data;

	buf = fgets(buffer, MAX_PACKET_SIZE, stdin);
	while (buf != NULL)
		buf = fgets(buffer, MAX_PACKET_SIZE, stdin);
	fprintf(stdout, "Keyboard quit\n");
	session_status = E_BT_ERROR;
	property_status = E_BT_ERROR;
	loop->quit();
	return 1;
}

static void _ftp_on_session(artik_bt_event event,
	void *data, void *user_data)
{
	bool connected = *(bool *)data;
	artik_loop_module *loop = (artik_loop_module *)user_data;

	if (connected)
		session_status = S_OK;
	else
		session_status = E_BT_ERROR;
	loop->quit();
}

static void _property_callback(artik_bt_event event,
	void *data, void *user_data)
{
	artik_bt_ftp_property *p = (artik_bt_ftp_property *)data;
	artik_loop_module *loop = (artik_loop_module *)user_data;
	const char *status_complete = "complete";

	if (!strncmp(p->status, (char *)status_complete,
		strlen(status_complete))) {
		property_status = S_OK;
		loop->quit();
	}
}

artik_error _ftp_start_session(void)
{
	artik_error ret;
	int watch_id = 0;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
		artik_request_api_module("loop");

	ret = bt->set_callback(BT_EVENT_CONNECT,
		_ftp_on_session, (void *)loop);
	if (ret != S_OK)
		goto quit;
	ret = bt->ftp_create_session(remote_mac_addr);
	if (ret != S_OK)
		goto quit;

	loop->add_fd_watch(STDIN_FILENO,
			(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP
			| WATCH_IO_NVAL),
			_on_keyboard_received, (void *)loop, &watch_id);
	loop->run();

quit:
	loop->remove_fd_watch(watch_id);
	artik_release_api_module(bt);
	artik_release_api_module(loop);
	return ret;
}

static void ftp_create_session_test(void)
{
	artik_error ret;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void ftp_remove_session_test(void)
{
	artik_error ret;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret != S_OK);

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void ftp_change_foler_test(void)
{
	artik_error ret;
	char *object_name = NULL;
	const char *test_object = "ut_test";
	const char *object_type = "folder";
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->ftp_change_folder((char *)test_object);
	CU_ASSERT(ret != S_OK);

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = _ftp_object_search(NULL, object_type, &object_name);
	if (ret == S_OK) {
		fprintf(stdout, "object found: %s\n", object_name);
		ret = bt->ftp_change_folder(object_name);
		CU_ASSERT(ret == S_OK);
	} else {
		fprintf(stdout, " no foder found!\n");
		ret = bt->ftp_create_folder((char *)test_object);
		CU_ASSERT(ret == S_OK);
		ret = bt->ftp_change_folder((char *)test_object);
		CU_ASSERT(ret == S_OK);
	}

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	if (object_name)
		free(object_name);
	artik_release_api_module(bt);
}

static void ftp_create_foler_test(void)
{
	artik_error ret;
	const char *object_name = "ut_test";
	const char *object_type = "folder";
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->ftp_create_folder((char *)object_name);
	CU_ASSERT(ret != S_OK);

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = _ftp_object_search(object_name, object_type, NULL);
	if (ret == S_OK) {
		fprintf(stdout, "object found: %s\n", object_name);
		ret = bt->ftp_delete_file((char *)object_name);
		CU_ASSERT(ret == S_OK);
	} else
		fprintf(stdout, "object not found: %s\n", object_name);
	ret = bt->ftp_create_folder((char *)object_name);
	CU_ASSERT(ret == S_OK);

	ret = _ftp_object_search(object_name, object_type, NULL);
	CU_ASSERT(ret == S_OK);

	ret = bt->ftp_delete_file((char *)object_name);
	CU_ASSERT(ret == S_OK);

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void ftp_delete_foler_test(void)
{
	artik_error ret;
	const char *object_name = "ut_test";
	const char *object_type = "folder";
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->ftp_delete_file((char *)object_name);
	CU_ASSERT(ret != S_OK);

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = _ftp_object_search(object_name, object_type, NULL);
	if (ret != S_OK) {
		fprintf(stdout, "object not found: %s\n", object_name);
		ret = bt->ftp_delete_file((char *)object_name);
		CU_ASSERT(ret != S_OK);
		ret = bt->ftp_create_folder((char *)object_name);
		CU_ASSERT(ret == S_OK);
	}
	ret = bt->ftp_delete_file((char *)object_name);
	CU_ASSERT(ret == S_OK);

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void ftp_list_foler_test(void)
{
	artik_error ret;
	artik_bt_ftp_file *file_list = NULL;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = bt->ftp_list_folder(&file_list);
	CU_ASSERT(ret != S_OK);

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = bt->ftp_list_folder(&file_list);
	CU_ASSERT(ret == S_OK);

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void ftp_get_file_test(void)
{
	artik_error ret = S_OK;
	int watch_id = 0;
	char *object_name = NULL;
	const char *object_type = "file";
	const char *target_object = "/root/target_file.c";
	const char *test_object = "test_file.c";
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
		artik_request_api_module("loop");

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = _ftp_object_search(NULL, object_type, &object_name);
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(object_name != NULL);

	ret = bt->set_callback(BT_EVENT_FTP,
		_property_callback, (void *)loop);
	CU_ASSERT(ret == S_OK);

	ret = bt->ftp_get_file((char *)target_object, object_name);
	CU_ASSERT(ret == S_OK);

	loop->add_fd_watch(STDIN_FILENO,
			(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP
			| WATCH_IO_NVAL),
			_on_keyboard_received, (void *)loop, &watch_id);
	loop->run();
	CU_ASSERT(property_status == S_OK);

	ret = _ftp_object_search(test_object, object_type, NULL);
	if (ret == S_OK)
		ret = bt->ftp_delete_file((char *)test_object);

	ret = bt->ftp_get_file((char *)target_object, (char *)test_object);
	CU_ASSERT(ret != S_OK);

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	if (object_name)
		free(object_name);
	loop->remove_fd_watch(watch_id);
	artik_release_api_module(bt);
	artik_release_api_module(loop);
}

static void ftp_put_file_test(void)
{
	artik_error ret = S_OK;
	int watch_id = 0;
	char *object_name = NULL;
	const char *object_type = "file";
	const char *target_object = "/root/target_file";
	const char *test_object = "/root/test_file";
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
		artik_request_api_module("loop");

	ret = _ftp_start_session();
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(session_status == S_OK);

	ret = _ftp_object_search(NULL, object_type, &object_name);
	CU_ASSERT(ret == S_OK);
	CU_ASSERT(object_name != NULL);

	ret = bt->set_callback(BT_EVENT_FTP,
		_property_callback, (void *)loop);
	CU_ASSERT(ret == S_OK);

	loop->add_fd_watch(STDIN_FILENO,
		(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP
		| WATCH_IO_NVAL),
		_on_keyboard_received, (void *)loop, &watch_id);

	ret = bt->ftp_get_file((char *)target_object, object_name);
	CU_ASSERT(ret == S_OK);
	loop->run();
	CU_ASSERT(property_status == S_OK);

	ret = bt->ftp_put_file((char *)test_object, object_name);
	CU_ASSERT(ret != S_OK);
	loop->run();
	CU_ASSERT(property_status == S_OK);

	ret = bt->ftp_put_file((char *)target_object, object_name);
	CU_ASSERT(ret == S_OK);
	loop->run();
	CU_ASSERT(property_status == S_OK);

	ret = bt->ftp_remove_session();
	CU_ASSERT(ret == S_OK);

	if (object_name)
		free(object_name);
	loop->remove_fd_watch(watch_id);
	artik_release_api_module(bt);
	artik_release_api_module(loop);
}

artik_error cunit_add_suite(CU_pSuite *psuite)
{
	CU_add_test(*psuite, "ftp_create_session_test",
		ftp_create_session_test);
	CU_add_test(*psuite, "ftp_remove_session_test",
		ftp_remove_session_test);
	CU_add_test(*psuite, "ftp_change_foler_test",
		ftp_change_foler_test);
	CU_add_test(*psuite, "ftp_create_foler_test",
		ftp_create_foler_test);
	CU_add_test(*psuite, "ftp_delete_foler_test",
		ftp_delete_foler_test);
	CU_add_test(*psuite, "ftp_list_foler_test",
		ftp_list_foler_test);
	CU_add_test(*psuite, "ftp_get_file_test",
		ftp_get_file_test);
	CU_add_test(*psuite, "ftp_put_file_test",
		ftp_put_file_test);

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
		return E_BAD_ARGS;
	if (strlen(remote_mac_addr) != (BT_ADDRESS_LEN - 1))
		return E_BAD_ARGS;
	fprintf(stdout, "remote address: %s-%zu\n",
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
