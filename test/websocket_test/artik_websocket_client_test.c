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
#include <pthread.h>
#include <signal.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_websocket.h>

static char *test_message = NULL;

static void connection_callback(void *user_data, void *result)
{

	intptr_t connected = (intptr_t)result;

	if (connected == ARTIK_WEBSOCKET_CONNECTED) {
		fprintf(stdout, "Websocket connected\n");

		artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");

		fprintf(stdout, "Writing: %s\n", test_message);

		websocket->websocket_write_stream((artik_websocket_handle)
						user_data, test_message);
		artik_release_api_module(websocket);
	} else if (connected == ARTIK_WEBSOCKET_CLOSED) {
		fprintf(stdout, "Websocket closed\n");

		artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
		loop->quit();
		artik_release_api_module(loop);
	} else {
		fprintf(stderr, "TEST failed, handshake error\n");

		artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
		loop->quit();
		artik_release_api_module(loop);
	}
}

static void receive_callback(void *user_data, void *result)
{

	char *buffer = (char *)result;

	if (buffer == NULL) {
		fprintf(stdout, "Received failed\n");
		return;
	}

	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

	printf("Received: %s\n", (char *)result);
	free(result);

	loop->quit();
	artik_release_api_module(loop);
}

static artik_error test_websocket_write(char *root_ca, char *client_cert,
		char *client_key, char *host, int port, bool use_tls,
		bool verify)
{

	artik_error ret = S_OK;
	artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_websocket_handle handle;
	artik_websocket_config *config = NULL;

	char *protocol = use_tls ? "wss" : "ws";
	char _port[6];

	if (!host) {
		printf("Error: You must define the hostname or ip address.\n");
		printf("Usage: websocket-client-test [-r <file of Root CA\n"
			"certificate>] ");
		printf("[-c <file of client certificate>]\n"
			"[-k <file of client key>] ");
		printf("[-i <ip address of the server>]\n"
			"[-p <port of the server>] ");
		printf("[-t for using TLS] [-m <message>]\n"
			"[-v for verifying CA certificate]\r\n");

		ret = E_BAD_ARGS;
		return ret;
	}

	config = (artik_websocket_config *)malloc(sizeof(
						artik_websocket_config));

	memset(config, 0, sizeof(artik_websocket_config));

	snprintf(_port, 6, "%d", port);

	int len = strlen(protocol) + 3 + strlen(host) + 1 + strlen(_port) + 2;

	config->uri = malloc(len);

	snprintf(config->uri, len, "%s://%s:%s/", protocol, host, _port);

	if (root_ca) {
		config->ssl_config.ca_cert.data = strdup(root_ca);
		config->ssl_config.ca_cert.len = strlen(root_ca);
	}

	if (client_cert) {
		config->ssl_config.client_cert.data = strdup(client_cert);
		config->ssl_config.client_cert.len = strlen(client_cert);
	}

	if (client_key) {
		config->ssl_config.client_key.data = strdup(client_key);
		config->ssl_config.client_key.len = strlen(client_key);
	}

	if (verify)
		config->ssl_config.verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
	else
		config->ssl_config.verify_cert = ARTIK_SSL_VERIFY_NONE;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = websocket->websocket_request(&handle, config);

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	ret = websocket->websocket_open_stream(handle);

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	ret = websocket->websocket_set_connection_callback(handle,
		connection_callback, (void *)handle);

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	ret = websocket->websocket_set_receive_callback(handle,
		receive_callback, (void *)handle);

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	loop->run();

	websocket->websocket_close_stream(handle);

	fprintf(stdout, "TEST: %s finished\n", __func__);

exit:
	artik_release_api_module(websocket);
	artik_release_api_module(loop);

	return ret;
}

int main(int argc, char *argv[])
{

	int opt;
	bool verify = false;
	bool use_tls = false;
	artik_error ret = S_OK;

	char *root_ca = NULL; // Root CA certificate
	char *client_cert = NULL; // Client certificate
	char *client_key = NULL; // Client key
	char *host = NULL; // IP Address of server
	int port = 0;
	long fsize;
	FILE *f;

	while ((opt = getopt(argc, argv, "r:c:k:i:p:m:vt")) != -1) {
		switch (opt) {
		case 'r':
			f = fopen(optarg, "rb");
			if (!f) {
				printf("File not found for parameter -r\n");
				return -1;
			}
			fseek(f, 0, SEEK_END);
			fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			root_ca = malloc(fsize + 1);
			fread(root_ca, fsize, 1, f);
			fclose(f);
			break;
		case 'c':
			f = fopen(optarg, "rb");
			if (!f) {
				printf("File not found for parameter -c\n");
				return -1;
			}
			fseek(f, 0, SEEK_END);
			fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			client_cert = malloc(fsize + 1);
			fread(client_cert, fsize, 1, f);
			fclose(f);
			break;
		case 'k':
			f = fopen(optarg, "rb");
			if (!f) {
				printf("File not found for parameter -k\n");
				return -1;
			}
			fseek(f, 0, SEEK_END);
			fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			client_key = malloc(fsize + 1);
			fread(client_key, fsize, 1, f);
			fclose(f);
			break;
		case 'i':
			host = malloc(strlen(optarg)+1);
			host = strdup(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'm':
			test_message = strndup(optarg, strlen(optarg)+1);
			break;
		case 'v':
			verify = true;
			break;
		case 't':
			use_tls = true;
			break;
		default:
			printf("Usage: websocket-client-test\n"
				"[-r <file of Root CA certificate>] ");
			printf("[-c <file of client certificate>]\n"
				"[-k <file of client key>] ");
			printf("[-i <ip address of the server>]\n"
				"[-p <port of the server>] ");
			printf("[-t for using TLS] [-m <message>]\n"
				"[-v for verifying CA certificate]\r\n");
			return 0;
		}
	}

	if (!test_message)
		test_message = strndup("ping", 5);

	ret = test_websocket_write(root_ca, client_cert, client_key, host,
				port, use_tls, verify);

	if (test_message)
		free(test_message);

	return (ret == S_OK) ? 0 : -1;
}
