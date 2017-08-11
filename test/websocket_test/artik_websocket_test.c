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

static const char *echo_websocket_root_ca =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\r\n"
	"EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\r\n"
	"EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\r\n"
	"ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\r\n"
	"NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\r\n"
	"EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\r\n"
	"AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\r\n"
	"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\r\n"
	"E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\r\n"
	"/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\r\n"
	"DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\r\n"
	"GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\r\n"
	"tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\r\n"
	"AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\r\n"
	"FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\r\n"
	"WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\r\n"
	"9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\r\n"
	"gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\r\n"
	"2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\r\n"
	"LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\r\n"
	"4uJEvlz36hz1\r\n"
	"-----END CERTIFICATE-----\n";

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

static artik_error test_websocket_write(char *uri, bool verify)
{
	artik_error ret = S_OK;
	artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_websocket_handle handle;
	artik_websocket_config *config = NULL;

	config = (artik_websocket_config *)malloc(sizeof(
						artik_websocket_config));

	memset(config, 0, sizeof(artik_websocket_config));

	config->uri = uri;
	config->ssl_config.ca_cert.data = strdup(echo_websocket_root_ca);
	config->ssl_config.ca_cert.len = strlen(echo_websocket_root_ca);

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
	artik_error ret = S_OK;
	char uri[26] = "ws://echo.websocket.org/";

	while ((opt = getopt(argc, argv, "tm:v")) != -1) {
		switch (opt) {
		case 't':
			snprintf(uri, 26, "%s", "wss://echo.websocket.org/");
			break;
		case 'm':
			test_message = strndup(optarg, strlen(optarg)+1);
			break;
		case 'v':
			verify = true;
			break;
		default:
			printf("Usage: websocket-test [-t for using TLS]\n"
			"[-m <message>] [-v for verifying CA certificate]\n");
			return 0;
		}
	}

	if (!test_message)
		test_message = strndup("ping", 5);

	ret = test_websocket_write(uri, verify);

	if (test_message)
		free(test_message);

	return (ret == S_OK) ? 0 : -1;
}
