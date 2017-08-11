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
#include <unistd.h>
#include <ctype.h>

#include <artik_module.h>
#include <artik_platform.h>
#include <artik_loop.h>
#include <artik_lwm2m.h>
#include <artik_log.h>

#include "artik_lwm2m_test_common.h"

#define UNUSED __attribute__((unused))

#define URI_MAX_LEN	128
#define UUID_MAX_LEN	64
#define DATA_MAX_LEN	1024
#define MAX_PACKET_SIZE 1024

artik_loop_module *loop;
artik_lwm2m_module *lwm2m;

static int g_quit;
static char akc_device_id[UUID_MAX_LEN] = "< DM enabled Artik"\
					" Cloud device ID >";
static char akc_device_token[UUID_MAX_LEN] = "< DM enabled Artik"\
					" Cloud device token >";
static char akc_uri[URI_MAX_LEN] = "coaps://coaps-api.artik.cloud:5686";

static void prv_change_obj(char *buffer, void *user_data)
{
	artik_lwm2m_handle handle = (artik_lwm2m_handle) user_data;
	artik_error result;
	char uri[URI_MAX_LEN];
	char data[DATA_MAX_LEN];
	command cmd;

	prv_init_command(&cmd, buffer);

	result = prv_read_uri(&cmd, uri);
	if (result != S_OK)
		goto syntax_error;
	printf("URI: %s\n", uri);
	result = prv_read_data(&cmd, data);

	if (result != S_OK)
		goto syntax_error;

	result = lwm2m->client_write_resource(handle, uri,
			(unsigned char *)data,
			strlen(data));
	if (result != S_OK)
		log_err("client change object failed (%s)", error_msg(result));
	else
		fprintf(stdout, "OK");

	return;

syntax_error:
	fprintf(stdout, "Syntax error !");
}

void prv_read_obj(char *buffer, void *user_data)
{
	artik_lwm2m_handle handle = (artik_lwm2m_handle) user_data;
	artik_error result;
	char uri[URI_MAX_LEN];
	char data[256]; int len = 256;
	command cmd;

	prv_init_command(&cmd, buffer);

	result = prv_read_uri(&cmd, uri);
	if (result != S_OK)
		goto syntax_error;

	result = lwm2m->client_read_resource(handle, uri, (unsigned char *)data,
							&len);
	if (result != S_OK) {
		log_err("read change object failed (%s)", error_msg(result));
		return;
	}

	data[len] = '\0';
	fprintf(stdout, "URI: %s - Value: %s\r\n> ", uri, data);

	return;

syntax_error:
	fprintf(stdout, "Syntax error !");
}

static void prv_quit(UNUSED char *buffer, UNUSED void *user_data)
{
	g_quit = 1;
	loop->quit();
}

struct command_desc_t commands[] = {
	{ "change", "Change the value of a resource.", NULL, prv_change_obj,
									NULL },
	{ "read", "Read the value of a resource", NULL, prv_read_obj, NULL },
	{ "q", "Quit the client.", NULL, prv_quit, NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static int on_keyboard_received(int fd, enum watch_io io,
		UNUSED void *user_data)
{
	char buffer[MAX_PACKET_SIZE];

	if (!(fd == STDIN_FILENO))
		log_err("%s STDIN_FILENO failed\n", __func__);

	if (!(io == WATCH_IO_IN || io == WATCH_IO_ERR || io == WATCH_IO_HUP
			|| io == WATCH_IO_NVAL)) {
		log_err("%s io failed\n", __func__);
	}

	if (fgets(buffer, MAX_PACKET_SIZE, stdin) == NULL)
		return 1;

	handle_command(commands, buffer);
	fprintf(stdout, "\r\n");

	if (g_quit == 0) {
		fprintf(stdout, "> ");
		fflush(stdout);
	} else {
		fprintf(stdout, "\r\n");
	}

	return 1;
}

static void on_error(void *data, void *user_data)
{
	artik_error err = (artik_error)(intptr_t)data;

	fprintf(stdout, "LWM2M error: %s\r\n", error_msg(err));
}

static void on_execute_resource(void *data, void *user_data)
{
	char *uri = (char *)data;

	fprintf(stdout, "LWM2M resource execute: %s\r\n", uri);
}

static void on_changed_resource(void *data, void *user_data)
{
	char *uri = (char *)data;

	fprintf(stdout, "LWM2M resource changed: %s\r\n", uri);
}

static void test_serialization(artik_lwm2m_handle handle)
{
	int test_int[2] = {0, 1};
	char *test_str[2] = {"192.168.1.27", "192.168.1.67"};
	artik_error res = S_OK;
	unsigned char *buffer_int = NULL, *buffer_str = NULL;
	int len_int = 0, len_str = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);
	res = lwm2m->serialize_tlv_int(test_int, 2, &buffer_int, &len_int);
	if (res == S_OK) {
		fprintf(stdout, "Send to 'Error Code' (/3/0/11)"\
			" multiple integer [0, 1]\n");
		res = lwm2m->client_write_resource(handle, "/3/0/11",
							buffer_int, len_int);
		fprintf(stdout, "result of serialization int sent : %s\n",
								error_msg(res));
		if (buffer_int)
			free(buffer_int);
	} else
		fprintf(stdout, "Failed to serialize array of int : %s\n",
								error_msg(res));
	res = lwm2m->serialize_tlv_string(test_str, 2, &buffer_str, &len_str);
	if (res == S_OK) {
		fprintf(stdout, "Send to 'Address' (/4/0/4) multiple string"\
					" ['192.168.1.27', '192.168.1.67']\n");
		res = lwm2m->client_write_resource(handle, "/4/0/4", buffer_str,
								len_str);
		fprintf(stdout, "result of serialization string sent : %s\n",
								error_msg(res));
		if (buffer_int)
			free(buffer_str);
	} else
		fprintf(stdout, "Failed to serialize array of string : %s\n",
								error_msg(res));
}

artik_error test_lwm2m_default(void)
{
	artik_error ret = S_OK;
	artik_lwm2m_handle client_h = NULL;
	artik_lwm2m_config config;
	char *ips[2] = {"192.168.1.27", NULL};
	char *routes[2] = {"192.168.1.1", NULL};
	int i = 0;
	int watch_id;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	memset(&config, 0, sizeof(config));
	config.server_id = 123;
	config.server_uri = akc_uri;
	config.name = akc_device_id;
	config.tls_psk_identity = akc_device_id;
	config.tls_psk_key = akc_device_token;
	config.lifetime = 30;

	fprintf(stdout, "TEST: %s uri=%s\n", __func__, config.server_uri);
	fprintf(stdout, "TEST: %s id=%s\n", __func__, config.tls_psk_identity);
	fprintf(stdout, "TEST: %s key=%s\n", __func__, config.tls_psk_key);

	/* Fill up objects */
	config.objects[ARTIK_LWM2M_OBJECT_CONNECTIVITY_MONITORING] =
		lwm2m->create_connectivity_monitoring_object(0, 0, 12, 1, 2,
						(const char **)ips,
						2, (const char **)routes, 0,
						"SAMI2_5G",
						2345, 189, 33);
	config.objects[ARTIK_LWM2M_OBJECT_DEVICE] =
		lwm2m->create_device_object("Samsung", "Artik", "1234567890",
					"1.0", "1.0", "1.0", "HUB", 0,
					5000, 1500, 100, 1000000, 200000,
					"Europe/Paris", "+01:00", "U");

	ret = lwm2m->client_connect(&client_h, &config);
	test_serialization(client_h);
	if (ret != S_OK)
		goto exit;

	for (i = 0; commands[i].name != NULL; i++)
		commands[i].user_data = (void *) client_h;

	lwm2m->set_callback(client_h, ARTIK_LWM2M_EVENT_ERROR, on_error,
			(void *)client_h);
	lwm2m->set_callback(client_h, ARTIK_LWM2M_EVENT_RESOURCE_EXECUTE,
			on_execute_resource,
			(void *)client_h);
	lwm2m->set_callback(client_h, ARTIK_LWM2M_EVENT_RESOURCE_CHANGED,
			on_changed_resource,
			(void *)client_h);

	fprintf(stdout, "TEST: %s add watch\n", __func__);

	loop->add_fd_watch(STDIN_FILENO,
			(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP |
								WATCH_IO_NVAL),
			on_keyboard_received, client_h, &watch_id);

	printf(">");

	loop->run();

exit:
	lwm2m->client_disconnect(client_h);
	lwm2m->free_object(config.objects[ARTIK_LWM2M_OBJECT_DEVICE]);
	fprintf(stdout, "TEST: %s %s\n", __func__,
			(ret == S_OK) ? "succeeded" : "failed");
	return ret;
}

int main(UNUSED int argc, UNUSED char *argv[])
{
	int opt;
	artik_error ret = S_OK;


	while ((opt = getopt(argc, argv, "u:i:k:")) != -1) {
		switch (opt) {
		case 'u':
			strncpy(akc_uri, optarg, URI_MAX_LEN);
			break;
		case 'i':
			strncpy(akc_device_id, optarg, UUID_MAX_LEN);
			break;
		case 'k':
			strncpy(akc_device_token, optarg, UUID_MAX_LEN);
			break;
		default:
			fprintf(stdout, "Usage: lwm2m-test <options>\r\n");
			fprintf(stdout, "\tOptions:\r\n");
			fprintf(stdout, "\t\t-u URI of server (e.g. \""\
				"coaps://lwm2mserv.com:5683\")\r\n");
			fprintf(stdout, "\t\t-i PSK Public identity\r\n");
			fprintf(stdout, "\t\t-k PSK Secret key\r\n");
			return 0;
		}
	}

	if (!artik_is_module_available(ARTIK_MODULE_LOOP)) {
		fprintf(stdout,
				"TEST: Loop module is not available,"\
				" skipping test...\n");
		return -1;
	}

	if (!artik_is_module_available(ARTIK_MODULE_LWM2M)) {
		fprintf(stdout,
				"TEST: LWM2M module is not available,"\
				" skipping test...\n");
		return -1;
	}

	loop = (artik_loop_module *) artik_request_api_module("loop");
	lwm2m = (artik_lwm2m_module *) artik_request_api_module("lwm2m");

	ret = test_lwm2m_default();

	return (ret == S_OK) ? 0 : -1;
}
