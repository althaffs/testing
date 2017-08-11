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
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>

#define MAX_BDADDR_LEN         17
#define MAX_PACKET_SIZE        1024
#define SCAN_TIME_MILLISECONDS (20*1000)
#define BUFFER_SIZE            17

static artik_bluetooth_module *bt_main;
static artik_loop_module *loop_main;

static char buffer[BUFFER_SIZE];

static void release_handler(void *user_data)
{
	fprintf(stdout, "<SPP>: Release_handler called\n");
}

static int on_keyboard_received(int fd, enum watch_io id, void *user_data)
{
	char buffer[MAX_PACKET_SIZE];
	intptr_t socket_fd = (intptr_t) user_data;

	fprintf(stdout, "<SPP>: Keyboard reviced fd: %zd\n", socket_fd);
	if (fgets(buffer, MAX_PACKET_SIZE, stdin) == NULL)
		return 1;
	fprintf(stdout, "<SPP>: Input: %s\n", buffer);

	if (send(socket_fd, buffer, strlen(buffer), 0) < 0)
		return -1;
	else
		return 1;
}

static void new_connection_handler(char *device_path, int fd, int version,
		int features, void *user_data)
{
	loop_main->add_fd_watch(STDIN_FILENO,
			(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP
			| WATCH_IO_NVAL),
			on_keyboard_received, (void *)(intptr_t)fd, NULL);
	fprintf(stdout, "<SPP>: Key board start success\n");
}

static void request_disconnect_handler(char *device_path, void *user_data)
{
	fprintf(stdout, "<SPP>: Request_disconnect_handler called\n");
}

static int uninit(void *user_data)
{
	fprintf(stdout, "<SPP>: Process cancel\n");
	loop_main->quit();

	return true;
}

static void print_devices(artik_bt_device *devices, int num)
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

static void on_scan(void *data, void *user_data)
{
	artik_bt_device *dev = (artik_bt_device *) data;

	print_devices(dev, 1);
}

static void on_bond(void *data, void *user_data)
{
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");
	char *remote_address = (char *)user_data;
	bool paired = *(bool *)data;

	if (paired) {
		fprintf(stdout, "<SPP>: %s - %s\n", __func__, "Paired");
		fprintf(stdout, "<SPP>: %s - start connect\n", __func__);
		bt->connect(remote_address);
		fprintf(stdout, "<SPP>: %s connect finished\n", __func__);

		artik_release_api_module(bt);
	} else {
		fprintf(stdout, "<SPP>: %s - %s\n", __func__, "Unpaired");
	}
}

static void on_connect(void *data, void *user_data)
{
	bool connected = *(bool *)data;

	fprintf(stdout, "<SPP>: %s %s\n", __func__,
		connected ? "Connected" : "Disconnected");
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
	default:
		break;
	}
}

static void scan_timeout_callback(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *) user_data;

	fprintf(stdout, "<SPP>: %s - stop scan\n", __func__);
	loop->quit();
}

artik_error bluetooth_scan(void)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");
	artik_error ret = S_OK;
	int timeout_id = 0;

	fprintf(stdout, "<SPP>: %s - starting\n", __func__);

	ret = bt->remove_devices();
	if (ret != S_OK)
		goto exit;

	ret = bt->set_callback(BT_EVENT_SCAN, user_callback, NULL);
	if (ret != S_OK)
		goto exit;

	ret = bt->start_scan();
	if (ret != S_OK)
		goto exit;

	loop->add_timeout_callback(&timeout_id,
			SCAN_TIME_MILLISECONDS, scan_timeout_callback,
			(void *)loop);
	loop->run();

exit:
	ret = bt->stop_scan();
	ret = bt->unset_callback(BT_EVENT_SCAN);
	fprintf(stdout, "<SPP>: %s - %s\n", __func__,
		(ret == S_OK) ? "succeeded" : "failed");

	artik_release_api_module(loop);
	artik_release_api_module(bt);
	return ret;
}

artik_error get_addr(char *remote_addr)
{
	char mac_other[2] = "";
	artik_error ret = S_OK;

	fprintf(stdout, "\n<SPP>: Input SPP Server MAC address:\n");

	if (fgets(remote_addr, MAX_BDADDR_LEN + 1, stdin) == NULL) {
		fprintf(stdout, "<SPP>: %s failed! fgets error\n", __func__);
		return E_BT_ERROR;
	}
	if (fgets(mac_other, 2, stdin) == NULL) {
		fprintf(stdout, "<SPP>: %s failed! fgets error\n", __func__);
		return E_BT_ERROR;
	}

	if (strlen(remote_addr) != MAX_BDADDR_LEN)
		ret =  E_BT_ERROR;

	return ret;
}

static artik_error set_callback(char *remote_addr)
{
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");
	artik_error ret = S_OK;

	ret = bt->set_callback(BT_EVENT_BOND, user_callback,
			     (void *)remote_addr);
	if (ret != S_OK)
		goto exit;
	ret = bt->set_callback(BT_EVENT_CONNECT, user_callback,
			     (void *)remote_addr);
	if (ret != S_OK)
		goto exit;

exit:
	artik_release_api_module(bt);
	return ret;
}

static artik_error profile_register(void)
{
	artik_error ret = S_OK;
	artik_bluetooth_module *bt =
		(artik_bluetooth_module *)artik_request_api_module("bluetooth");
	static artik_bt_spp_profile_option profile_option;

	profile_option.name = "Artik SPP Loopback";
	profile_option.service = "spp char loopback";
	profile_option.role = "client";
	profile_option.channel = 22;
	profile_option.PSM = 3;
	profile_option.require_authentication = 1;
	profile_option.auto_connect = 1;
	profile_option.version = 10;
	profile_option.features = 20;

	ret = bt->spp_register_profile(&profile_option);
	if (ret != S_OK) {
		fprintf(stdout, "<SPP>: Register profile failed!\n");
		goto exit;
	}

	ret = bt->spp_set_callback(
		release_handler,
		new_connection_handler,
		request_disconnect_handler,
		NULL);
	if (ret != S_OK) {
		fprintf(stdout, "<SPP>: Profile set call back failed!\n");
		goto exit;
	}

exit:
	artik_release_api_module(bt);
	return ret;
}

static void ask(char *prompt)
{
	printf("%s\n", prompt);
	if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
		fprintf(stdout, "<SPP>: %s failed! fgets error\n", __func__);
}

static void m_request_pincode(
			artik_bt_agent_request_handle handle,
			char *device, void *user_data)
{
	printf("Request pincode (%s)\n", device);
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");

	ask("Enter PIN Code: ");

	bt->agent_send_pincode(handle, buffer);
	artik_release_api_module(bt);
}

static void m_request_passkey(
			artik_bt_agent_request_handle handle,
			char *device, void *user_data)
{
	unsigned int passkey;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");

	printf("Request passkey (%s)\n", device);
	ask("Enter passkey (1~999999): ");
	if (sscanf(buffer, "%u", &passkey) == 1)
		bt->agent_send_passkey(handle, passkey);

	artik_release_api_module(bt);
}

static void m_request_confirmation(
				artik_bt_agent_request_handle handle,
				char *device, unsigned int passkey,
				void *user_data)
{
	printf("Request confirmation (%s)\nPasskey: %06u\n", device, passkey);
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");

	ask("Confirm passkey? (yes/no): ");
	if (!strncmp(buffer, "yes", 3))
		bt->agent_send_empty_response(handle);
	else
		bt->agent_send_error(handle, BT_AGENT_REQUEST_REJECTED, "");

	artik_release_api_module(bt);
}

static void m_request_authorization(
				artik_bt_agent_request_handle handle,
				char *device, void *user_data)
{
	printf("Request authorization (%s)\n", device);
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");

	ask("Authorize? (yes/no): ");
	if (!strncmp(buffer, "yes", 3))
		bt->agent_send_empty_response(handle);
	else
		bt->agent_send_error(handle, BT_AGENT_REQUEST_REJECTED, "");

	artik_release_api_module(bt);
}

static void m_authorize_service(
					artik_bt_agent_request_handle handle,
					char *device, char *uuid,
					void *user_data)
{
	printf("Authorize Service (%s, %s)\n", device, uuid);
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
					artik_request_api_module("bluetooth");

	ask("Authorize connection? (yes/no): ");
	if (!strncmp(buffer, "yes", 3))
		bt->agent_send_empty_response(handle);
	else
		bt->agent_send_error(handle, BT_AGENT_REQUEST_REJECTED, "");

	artik_release_api_module(bt);
}

static artik_error agent_register(void)
{
	artik_error ret = S_OK;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
			artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
			artik_request_api_module("loop");
	artik_bt_agent_callbacks *m_callback =
		(artik_bt_agent_callbacks *)
		malloc(sizeof(artik_bt_agent_callbacks));
	artik_bt_agent_capability g_capa = BT_CAPA_KEYBOARDDISPLAY;

	memset(m_callback, 0, sizeof(artik_bt_agent_callbacks));
	m_callback->authorize_service_func = m_authorize_service;
	m_callback->request_authorization_func = m_request_authorization;
	m_callback->request_confirmation_func = m_request_confirmation;
	m_callback->request_passkey_func = m_request_passkey;
	m_callback->request_pincode_func = m_request_pincode;

	bt->set_discoverable(true);

	printf("Invoke register...\n");
	bt->agent_register_capability(g_capa);
	bt->agent_set_default();

	artik_release_api_module(loop);
	artik_release_api_module(bt);
	free(m_callback);
	return ret;

}

int main(void)
{
	artik_error ret = S_OK;
	char remote_address[MAX_BDADDR_LEN] = "";

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		fprintf(stdout, "<SPP>: Bluetooth module not available!\n");
		goto loop_quit;
	}

	bt_main = (artik_bluetooth_module *)
			artik_request_api_module("bluetooth");
	loop_main = (artik_loop_module *)artik_request_api_module("loop");
	if (!bt_main || !loop_main)
		goto loop_quit;

	ret = profile_register();
	if (ret != S_OK) {
		fprintf(stdout, "<SPP>: SPP register error!\n");
		goto loop_quit;
	}
	fprintf(stdout, "<SPP>: SPP register profile success!\n");

	ret = agent_register();
	if (ret != S_OK) {
		fprintf(stdout, "<SPP>: SPP agent register error!\n");
		goto loop_quit;
	}
	fprintf(stdout, "<SPP>: Spp agent register success!\n");

	ret = bluetooth_scan();
	if (ret != S_OK) {
		fprintf(stdout, "<SPP>: SPP scan error!\n");
		goto spp_quit;
	}

	ret = get_addr(remote_address);
	if (ret != S_OK) {
		fprintf(stdout, "<SPP>: SPP get address error!\n");
		goto spp_quit;
	}
	fprintf(stdout, "<SPP>: get remote addr: %s\n", remote_address);

	bt_main->set_trust(remote_address);
	ret = set_callback(remote_address);
	if (ret != S_OK) {
		fprintf(stdout, "<SPP>: SPP set callback error!\n");
		goto spp_quit;
	}
	fprintf(stdout, "<SPP>: SPP start bond!\n");
	bt_main->start_bond(remote_address);
	loop_main->add_signal_watch(SIGINT, uninit, NULL, NULL);
	loop_main->run();

spp_quit:
	bt_main->unset_callback(BT_EVENT_BOND);
	bt_main->unset_callback(BT_EVENT_CONNECT);
	bt_main->disconnect(remote_address);
	bt_main->spp_unregister_profile();
	fprintf(stdout, "<SPP>: SPP quit!\n");
loop_quit:
	bt_main->agent_unregister();
	if (bt_main)
		artik_release_api_module(bt_main);
	if (loop_main)
		artik_release_api_module(loop_main);

	fprintf(stdout, "<SPP>: Loop quit!\n");
	return S_OK;
}
