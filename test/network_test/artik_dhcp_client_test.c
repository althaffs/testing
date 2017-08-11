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
#include <signal.h>

#include <artik_module.h>
#include <artik_network.h>
#include <artik_loop.h>

static void sig_handler(int sig)
{
	artik_loop_module *loop = (artik_loop_module *)artik_request_api_module(
									"loop");

	loop->quit();
}

static artik_error test_dhcp_client(artik_network_interface_t interface)
{
	artik_network_module *network = (artik_network_module *)
					artik_request_api_module("network");
	artik_loop_module *loop = (artik_loop_module *)artik_request_api_module(
									"loop");
	artik_error ret;
	artik_network_dhcp_client_handle handle;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	fprintf(stdout, "Starting DHCP Client\n");

	/* Start DHCP Client */
	ret = network->dhcp_client_start(&handle, interface);

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	signal(SIGINT, sig_handler);

	loop->run();

	fprintf(stdout, "Stopping DHCP Client\n");

	/* Stop DHCP Client */
	ret = network->dhcp_client_stop(handle);


	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

exit:
	artik_release_api_module(network);
	artik_release_api_module(loop);

	return ret;
}

int main(int argc, char *argv[])
{
	artik_error ret = S_OK;
	int opt;
	artik_network_interface_t interface = ARTIK_WIFI;

	if (!artik_is_module_available(ARTIK_MODULE_NETWORK)) {
		fprintf(stdout,
			"TEST: NETWORK module is not available,"\
			" skipping test...\n");
		return -1;
	}

	while ((opt = getopt(argc, argv, "e")) != -1) {
		switch (opt) {
		case 'e':
			interface = ARTIK_ETHERNET;
			break;
		default:
			printf("Usage : artik-dhcp-client-test"\
				" [-e for ethernet] (wifi by default)\n");
			return 0;

		}
	}

	ret = test_dhcp_client(interface);

	return (ret == S_OK) ? 0 : -1;
}
