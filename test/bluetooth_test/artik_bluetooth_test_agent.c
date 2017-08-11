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

#include <artik_module.h>
#include <artik_bluetooth.h>
#include <artik_loop.h>

#define BUFFER_SIZE 17

static char buffer[BUFFER_SIZE];
static artik_bt_agent_capability g_capa = BT_CAPA_KEYBOARDDISPLAY;
static void ask(char *prompt)
{
	printf("%s\n", prompt);
	if (fgets(buffer, BUFFER_SIZE, stdin)  == NULL)
		fprintf(stdout, "\ncmd fgets error\n");
}

static void m_request_pincode(artik_bt_agent_request_handle handle, char *device, void *user_data)
{
	printf("Request pincode (%s)\n", device);
	artik_bluetooth_module *bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");

	ask("Enter PIN Code: ");

	bt->agent_send_pincode(handle, buffer);
	artik_release_api_module(bt);
}

static void m_request_passkey(artik_bt_agent_request_handle handle, char *device, void *user_data)
{
	unsigned int passkey;
	artik_bluetooth_module *bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");

	printf("Request passkey (%s)\n", device);
	ask("Enter passkey (1~999999): ");
	if (sscanf(buffer, "%u", &passkey) == 1)
		bt->agent_send_passkey(handle, passkey);

	artik_release_api_module(bt);
}

static void m_request_confirmation(artik_bt_agent_request_handle handle, char *device, unsigned int passkey,
		void *user_data)
{
	printf("Request confirmation (%s)\nPasskey: %06u\n", device, passkey);
	artik_bluetooth_module *bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");

	ask("Confirm passkey? (yes/no): ");
	if (!strncmp(buffer, "yes", 3))
		bt->agent_send_empty_response(handle);
	else
		bt->agent_send_error(handle, BT_AGENT_REQUEST_REJECTED, "");

	artik_release_api_module(bt);
}

static void m_request_authorization(artik_bt_agent_request_handle handle, char *device, void *user_data)
{
	printf("Request authorization (%s)\n", device);
	artik_bluetooth_module *bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");

	ask("Authorize? (yes/no): ");
	if (!strncmp(buffer, "yes", 3))
		bt->agent_send_empty_response(handle);
	else
		bt->agent_send_error(handle, BT_AGENT_REQUEST_REJECTED, "");

	artik_release_api_module(bt);
}

static void m_authorize_service(artik_bt_agent_request_handle handle, char *device, char *uuid, void *user_data)
{
	printf("Authorize Service (%s, %s)\n", device, uuid);
	artik_bluetooth_module *bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");

	ask("Authorize connection? (yes/no): ");
	if (!strncmp(buffer, "yes", 3))
		bt->agent_send_empty_response(handle);
	else
		bt->agent_send_error(handle, BT_AGENT_REQUEST_REJECTED, "");

	artik_release_api_module(bt);
}

static int uninit(void *user_data)
{
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
				artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
			artik_request_api_module("loop");
	printf("Get module bluetooth success !\n");

	printf("Invoke unregister...\n");
	bt->agent_unregister();
	loop->quit();

	return true;
}

static artik_error test_bluetooth_agent(void)
{
	artik_error ret = S_OK;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
			artik_request_api_module("bluetooth");
	artik_loop_module *loop = (artik_loop_module *)
			artik_request_api_module("loop");
	artik_bt_agent_callbacks *m_callback =
		(artik_bt_agent_callbacks *)malloc(sizeof(artik_bt_agent_callbacks));

	memset(m_callback, 0, sizeof(artik_bt_agent_callbacks));
	m_callback->authorize_service_func = m_authorize_service;
	m_callback->request_authorization_func = m_request_authorization;
	m_callback->request_confirmation_func = m_request_confirmation;
	m_callback->request_passkey_func = m_request_passkey;
	m_callback->request_pincode_func = m_request_pincode;

	bt->set_discoverable(true);

	printf("Invoke register...\n");
	bt->agent_set_callback(m_callback);
	bt->agent_register_capability(g_capa);
	bt->agent_set_default();
	loop->add_signal_watch(SIGINT, uninit, NULL, NULL);
	loop->run();

	artik_release_api_module(loop);
	artik_release_api_module(bt);
	free(m_callback);
	return ret;
}

int main(int argc, char *argv[])
{
	artik_error ret = S_OK;
	int temp_capa = 0;

	if (argv[1] != NULL) {
		temp_capa = argv[1][0] - '0';
		if (temp_capa >= BT_CAPA_KEYBOARDDISPLAY
			|| temp_capa < BT_CAPA_END)
			g_capa = temp_capa;
	}

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		printf("TEST:Bluetooth module is not available, skipping test...\n");
		return -1;
	}

	if (!artik_is_module_available(ARTIK_MODULE_LOOP)) {
		printf("TEST:Loop module is not available, skipping test...\n");
		return -1;
	}

	ret = test_bluetooth_agent();

	if (ret != S_OK)
		printf("Test bluetooth agent failed!\n");

	return (ret == S_OK) ? 0 : -1;
}
