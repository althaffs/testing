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

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>

#define MAX_BDADDR_LEN			17
#define BUFFER_LEN				128
#define SCAN_TIME_MILLISECONDS	(20*1000)
#define BUFFER_SIZE				17

static char buffer[BUFFER_SIZE];

static artik_loop_module *loop_main;

static int uninit(void *user_data)
{
	artik_error ret;

	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	ret = bt->pan_disconnect();
	if (ret != S_OK)
		fprintf(stdout, "Disconnect Error!\r\n");
	else
		fprintf(stdout, "Disconnect OK!\r\n");
	loop_main->quit();

	return true;
}

static void print_devices(artik_bt_device *devices, int num)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		char *re_name;

		fprintf(stdout, "[Device]: %s\t",
			devices[i].remote_address ? devices[i].remote_address : "(null)");
		re_name = (devices[i].remote_name ? devices[i].remote_name : "(null)");
		if (strlen(re_name) < 8) {
			fprintf(stdout, "%s\t\t",
				devices[i].remote_name ? devices[i].remote_name : "(null)");
			}
		else{
			fprintf(stdout, "%s\t",
				devices[i].remote_name ? devices[i].remote_name : "(null)");
			}
		fprintf(stdout, "RSSI: %d\t", devices[i].rssi);
		fprintf(stdout, "Bonded: %s\n",
			devices[i].is_bonded ? "true" : "false");
	}
}

static void on_scan(void *data, void *user_data)
{
	artik_bt_device *dev = (artik_bt_device *) data;

	print_devices(dev, 1);
}

static void on_connect(void *data, void *user_data)
{
	char buf[BUFFER_LEN];
	char *interface = NULL;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	bool connected = *(bool *)data;

	if (!connected) {
		fprintf(stdout, "<PANU>: Connect error\n");
		goto exit;
	}

	if (bt->pan_get_interface(&interface) == S_OK) {
		snprintf(buf, BUFFER_LEN, "dhclient -r %s", interface);
		if (system(buf) < 0) {
			fprintf(stdout, "cmd system error\n");
			goto exit;
		}
		snprintf(buf, BUFFER_LEN, "dhclient %s", interface);
		if (system(buf) < 0) {
			fprintf(stdout, "cmd system error\n");
			goto exit;
		}
		snprintf(buf, BUFFER_LEN, "ifconfig eth0 down");
		if (system(buf) < 0) {
			fprintf(stdout, "cmd system error\n");
			goto exit;
		}
		fprintf(stdout, "Please input test command(max length is 127) or 'q' to exit\n");
		for (;;) {
			memset(buf, 0, BUFFER_LEN);
			if (fgets(buf, BUFFER_LEN, stdin) == NULL) {
				fprintf(stdout, "cmd system error\n");
				break;
			}
			if (strlen(buf) > 1) {
				if (buf[strlen(buf)-1] == '\n')
					buf[strlen(buf)-1] = '\0';
				if (strcmp(buf, "q") == 0)
					break;
				if (system(buf) < 0) {
					fprintf(stdout, "cmd system error\n");
					break;
				}
			}
		}
		uninit(NULL);
	}
exit:
	artik_release_api_module(bt);
}

static void on_bond(void *data, void *user_data)
{
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	char *remote_address = (char *)user_data;
	bool paired = *(bool *)data;
	char *network_interface = NULL;
	artik_error ret = S_OK;

	if (paired) {
		fprintf(stdout, "<PANU>: %s - %s\n", __func__, "Paired");
		fprintf(stdout, "<PANU>: %s - start connect\n", __func__);
		const char *uuid = "nap";

		ret = bt->pan_connect(remote_address, uuid,
			&network_interface);
		if ((ret == S_OK) && network_interface)
			fprintf(stdout, "<PANU>: call pan connect sucess\n");
		else
			fprintf(stdout, "<PANU>: call pan connect error\n");

	} else {
		fprintf(stdout, "<PANU>: %s - %s\n", __func__, "Unpaired");
	}
	artik_release_api_module(bt);
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
	artik_loop_module *loop = (artik_loop_module *)user_data;

	fprintf(stdout, "<PANU>: %s - stop scan\n", __func__);
	loop->quit();
}

artik_error bluetooth_scan(void)
{
	artik_error ret;
	artik_loop_module *loop = (artik_loop_module *)
		artik_request_api_module("loop");
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	int timeout_id = 0;

	fprintf(stdout, "<PANU>: %s - starting\n", __func__);

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
	bt->stop_scan();
	bt->unset_callback(BT_EVENT_SCAN);

	artik_release_api_module(loop);
	artik_release_api_module(bt);
	return ret;
}

artik_error get_addr(char *remote_addr)
{
	char mac_other[2] = "";
	artik_error ret = S_OK;

	fprintf(stdout, "\n<PANU>: Input Server MAC address:\n");

	if (fgets(remote_addr, MAX_BDADDR_LEN + 1, stdin) == NULL)
		return E_BT_ERROR;
	if (fgets(mac_other, 2, stdin) == NULL)
		return E_BT_ERROR;
	if (strlen(remote_addr) != MAX_BDADDR_LEN)
		ret =  E_BT_ERROR;
	return ret;
}

static artik_error set_callback(char *remote_addr)
{
	artik_error ret;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

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

static void ask(char *prompt)
{
	printf("%s\n", prompt);
	char *buf;

	buf = fgets(buffer, BUFFER_SIZE, stdin);
	if (buf == NULL)
		printf("Request Error\n");
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

	passkey = strtoul(buffer, NULL, 10);
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

int main(int argc, char *argv[])
{
	artik_error ret = S_OK;
	artik_bluetooth_module *bt_main = NULL;
	char remote_address[MAX_BDADDR_LEN] = "";
	int status = -1;

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		fprintf(stdout, "<PANU>: Bluetooth not available!\n");
		return -1;
	}

	status = system("systemctl stop connman");
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("<PANU>: Stop connman service failed\r\n");
		return -1;
	}

	bt_main = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	loop_main = (artik_loop_module *)
		artik_request_api_module("loop");
	if (!bt_main || !loop_main)
		goto loop_quit;

	ret = agent_register();
	if (ret != S_OK) {
		fprintf(stdout, "<PANU>: Agent register error!\n");
		goto loop_quit;
	}
	fprintf(stdout, "<PANU>: Agent register success!\n");

	ret = bluetooth_scan();
	if (ret != S_OK) {
		fprintf(stdout, "<PANU>: Scan error!\n");
		goto loop_quit;
	}

	ret = get_addr(remote_address);
	if (ret != S_OK) {
		fprintf(stdout, "<PANU>: Get address error!\n");
		goto loop_quit;
	}

	ret = set_callback(remote_address);
	if (ret != S_OK) {
		fprintf(stdout, "<PANU>: Set callback error!\n");
		goto loop_quit;
	}

	fprintf(stdout, "<PANU>: Start bond!\n");
	bt_main->start_bond(remote_address);
	loop_main->add_signal_watch(SIGINT, uninit, NULL, NULL);
	loop_main->run();

loop_quit:
	if (bt_main)
		artik_release_api_module(bt_main);
	if (loop_main)
		artik_release_api_module(loop_main);
	fprintf(stdout, "<PANU>: Loop quit!\n");
	return S_OK;
}
