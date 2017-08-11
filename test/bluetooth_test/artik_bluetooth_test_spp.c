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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <gio/gio.h>
#pragma GCC diagnostic pop
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/socket.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>

#define MAX_PACKET_SIZE 1024

artik_bluetooth_module *bt;
artik_loop_module *loop;
artik_error err;

artik_bt_spp_profile_option profile_option;

static int on_socket(int fd, enum watch_io io, void *user_data)
{
	if (io & WATCH_IO_IN) {
		uint8_t buffer[MAX_PACKET_SIZE];
		int num_bytes = 0;

		num_bytes = recv(fd, buffer, MAX_PACKET_SIZE, 0);
		if (num_bytes == -1) {
			printf("Error in recvfrom()\n");
		} else {
			printf("Buffer received %d bytes\n", num_bytes);
			buffer[num_bytes] = '\0';
			printf("%s\n", buffer);
			send(fd, "Hello\n", 7, 0);
		}
	}
	return 1;
}

static void release_handler(void *user_data)
{
	printf("%s\n", __func__);
}

static void new_connection_handler(char *device_path, int fd, int version,
		int features, void *user_data) {
	loop->add_fd_watch(fd,
			WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP | WATCH_IO_NVAL,
			on_socket, NULL, NULL);
}

static void request_disconnect_handler(char *device_path, void *user_data)
{
	printf("%s\n", __func__);
}

static int uninit(void *user_data)
{
	err = bt->spp_unregister_profile();
	if (err != S_OK)
		printf("<SPP>:Unregister Error:%d!\r\n", err);
	else
		printf("<SPP>:Unregister OK!\r\n");
	loop->quit();

	return true;
}

int main(int argc, char *argv[])
{
	profile_option.name = "Artik SPP Loopback";
	profile_option.service = "spp char loopback";
	profile_option.role = "server";
	profile_option.channel = 22;
	profile_option.PSM = 3;
	profile_option.require_authentication = TRUE;
	profile_option.auto_connect = TRUE;
	profile_option.version = 10;
	profile_option.features = 20;

	printf("TEST:require Bluetooth module\n");
	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		printf("TEST:Bluetooth module is not available, skipping test...\n");
		return -1;
	}
	printf("TEST:require bt module\n");

	bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");
	loop = (artik_loop_module *) artik_request_api_module("loop");
	printf("TEST:bt over SPP\n");

	err = bt->spp_register_profile(&profile_option);
	if (err != S_OK) {
		printf("<SPP> TEST:SPP register error!\n");
		loop->quit();
	} else {
		printf("<SPP> TEST:SPP register success!\n");
	}

	err = bt->spp_set_callback(
		release_handler,
		new_connection_handler,
		request_disconnect_handler,
		NULL);
	if (err != S_OK) {
		printf("<SPP> TEST:SPP set callback error!\n");
		loop->quit();
	} else {
		printf("<SPP> TEST:SPP set callback success!\n");
	}
	loop->add_signal_watch(SIGINT, uninit, NULL, NULL);
	loop->run();

	artik_release_api_module(loop);
	artik_release_api_module(bt);

	return S_OK;
}

