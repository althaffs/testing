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

static artik_error test_dhcp_server(artik_network_dhcp_server_config *config)
{
	artik_network_module *network = (artik_network_module *)
					artik_request_api_module("network");
	artik_loop_module *loop = (artik_loop_module *)artik_request_api_module(
									"loop");
	artik_error ret;
	artik_network_dhcp_server_handle handle;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	fprintf(stdout, "Starting DHCP Server\n");

	/* Start DHCP Server */
	ret = network->dhcp_server_start(&handle, config);

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	signal(SIGINT, sig_handler);

	loop->run();

	fprintf(stdout, "Stopping DHCP Server\n");

	/* Stop DHCP Server */
	ret = network->dhcp_server_stop(handle);

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	fprintf(stdout, "TEST: %s finished\n", __func__);

exit:
	artik_release_api_module(network);
	artik_release_api_module(loop);

	return ret;
}

int main(int argc, char *argv[])
{
	artik_error ret = S_OK;
	artik_network_dhcp_server_config config;
	int opt;

	memset(&config, 0, sizeof(config));
	config.interface = ARTIK_WIFI;

	while ((opt = getopt(argc, argv, "i:n:g:a:b:s:l:e")) != -1) {
		switch (opt) {
		case 'i':
			strncpy(config.ip_addr.address, optarg,
							MAX_IP_ADDRESS_LEN);
			break;
		case 'n':
			strncpy(config.netmask.address, optarg,
							MAX_IP_ADDRESS_LEN);
			break;
		case 'g':
			strncpy(config.gw_addr.address, optarg,
							MAX_IP_ADDRESS_LEN);
			break;
		case 'a':
			strncpy(config.dns_addr[0].address, optarg,
							MAX_IP_ADDRESS_LEN);
			break;
		case 'b':
			strncpy(config.dns_addr[1].address, optarg,
							MAX_IP_ADDRESS_LEN);
			break;
		case 's':
			strncpy(config.start_addr.address, optarg,
							MAX_IP_ADDRESS_LEN);
			break;
		case 'l':
			config.num_leases = atoi(optarg);
			break;
		case 'e':
			config.interface = ARTIK_ETHERNET;
			break;
		default:
			printf("Usage: network-dhcp-server-test"\
				" [-i IP address of server] ");
			printf("[-n netmask] [-g gateway address] ");
			printf("[-a DNS address 1] [-b DNS address 2] ");
			printf("[-s start IP address] [-l number of leases] ");
			printf("[-e for ethernet] (wifi by default)\n");
			return 0;
		}
	}

	ret = test_dhcp_server(&config);

	return (ret == S_OK) ? 0 : -1;
}
