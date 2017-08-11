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

#include <artik_module.h>
#include <artik_network.h>
#include <artik_loop.h>

typedef struct {
	int deconnection_detected;
	int reconnection_detected;
} watch_online_status_t;

static artik_error test_set_network_config(artik_network_config *config,
					artik_network_interface_t interface)
{
	artik_network_module *network = (artik_network_module *)
					artik_request_api_module("network");
	artik_error ret;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = network->set_network_config(config, interface);
	if (ret == S_OK)
		fprintf(stdout, "TEST: network config set\n");
	else {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	fprintf(stdout, "TEST: %s succeeded\n", __func__);

exit:
	artik_release_api_module(network);

	return ret;
}

static artik_error test_get_network_config(
			artik_network_interface_t interface)
{
	artik_network_module *network = (artik_network_module *)
					artik_request_api_module("network");
	artik_error ret;
	artik_network_config net_config = { 0 };

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = network->get_network_config(&net_config, interface);
	if (ret == S_OK) {
		fprintf(stdout, "IP Address: %s\n"\
				"Netmask Address: %s\n"\
				"Gateway Address: %s\n"\
				"DNS Address 1: %s\n"\
				"DNS Address 2: %s\n"\
				"MAC Address: %s\n",
				net_config.ip_addr.address,
				net_config.netmask.address,
				net_config.gw_addr.address,
				net_config.dns_addr[0].address,
				net_config.dns_addr[1].address,
				net_config.mac_addr);
	} else {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	fprintf(stdout, "TEST: %s succeeded\n", __func__);

exit:
	artik_release_api_module(network);

	return ret;
}

static artik_error test_get_current_public_ip(void)
{
	artik_network_module *network = (artik_network_module *)
					artik_request_api_module("network");
	artik_error ret;
	artik_network_ip current_ip;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	/* Check Current IP */
	ret = network->get_current_public_ip(&current_ip);
	if (ret == S_OK)
		fprintf(stdout, "Your IP Address: %s\n", current_ip.address);
	else {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	fprintf(stdout, "TEST: %s succeeded\n", __func__);

exit:
	artik_release_api_module(network);

	return ret;

}

static artik_error test_get_online_status(void)
{
	artik_network_module *network = (artik_network_module *)
					artik_request_api_module("network");
	artik_error ret;
	bool online_status;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = network->get_online_status(&online_status);
	if (ret < S_OK)
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);

	if (ret == S_OK)
		fprintf(stdout, "TEST: %s succeeded (online status=%d)\n",
						__func__, online_status);

	artik_release_api_module(network);

	return ret;
}

static void disconnect(void *user_data)
{
	system("ifconfig wlan0 down");
}

static void reconnect(void *user_data)
{
	system("ifconfig wlan0 up; sleep 1; pkill dhclient;"\
		" sleep 1; dhclient wlan0");
}

static void quit(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

	loop->quit();
}

static void _callback(bool online_status, void *user_data)
{
	watch_online_status_t *data = user_data;

	if (online_status == 1) {
		data->deconnection_detected = 1;
		fprintf(stdout, "Network Connected\n");
	} else {
		data->reconnection_detected = 1;
		fprintf(stdout, "Network could not be connected\n");
	}
}

static artik_error test_watch_online_status(void)
{
	artik_error ret = S_OK;
	int timeout_disconnect_id, timeout_reconnect_id, timeout_quit_id;
	artik_network_module *network = (artik_network_module *)
					artik_request_api_module("network");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	watch_online_status_handle handle;
	watch_online_status_handle handle2;
	watch_online_status_t data = {0, 0};
	watch_online_status_t data2 = {0, 0};

	fprintf(stdout, "TEST: %s starting\n", __func__);
	ret = network->add_watch_online_status(&handle, _callback, &data);
	ret = network->add_watch_online_status(&handle2, _callback, &data2);

	loop->add_timeout_callback(&timeout_disconnect_id, 1000, disconnect,
									NULL);
	loop->add_timeout_callback(&timeout_reconnect_id, 2000, reconnect,
									NULL);
	loop->add_timeout_callback(&timeout_quit_id, 4000, quit, NULL);

	loop->run();

	if (!data.deconnection_detected && !data.reconnection_detected
	    && !data2.deconnection_detected && !data2.reconnection_detected) {
		ret = -1;
		fprintf(stderr, "TEST: %s failed (deconnection = %d,"\
			" reconnection = %d)\n",
			__func__, data.deconnection_detected,
			data.reconnection_detected);
		goto exit;
	}

	fprintf(stdout, "TEST: %s succeeded\n", __func__);
exit:
	network->remove_watch_online_status(handle);
	network->remove_watch_online_status(handle2);
	artik_release_api_module(loop);
	artik_release_api_module(network);
	return ret;
}

int main(int argc, char *argv[])
{
	artik_error ret = S_OK;
	int opt;
	int execute_all_tests = 0;
	artik_network_interface_t interface = ARTIK_WIFI;
	artik_network_config config;
	bool enable_set_config = false;

	memset(&config, 0, sizeof(config));

	if (!artik_is_module_available(ARTIK_MODULE_NETWORK)) {
		fprintf(stdout,
			"TEST: NETWORK module is not available,"\
			" skipping test...\n");
		return -1;
	}

	while ((opt = getopt(argc, argv, "ai:n:g:c:d:e")) != -1) {
		switch (opt) {
		case 'a':
			execute_all_tests = 1;
			break;
		case 'i':
			strncpy(config.ip_addr.address, optarg,
				MAX_IP_ADDRESS_LEN);
			enable_set_config = true;
			break;
		case 'n':
			strncpy(config.netmask.address, optarg,
				MAX_IP_ADDRESS_LEN);
			enable_set_config = true;
			break;
		case 'g':
			strncpy(config.gw_addr.address, optarg,
				MAX_IP_ADDRESS_LEN);
			enable_set_config = true;
			break;
		case 'c':
			strncpy(config.dns_addr[0].address, optarg,
				MAX_IP_ADDRESS_LEN);
			enable_set_config = true;
			break;
		case 'd':
			strncpy(config.dns_addr[1].address, optarg,
				MAX_IP_ADDRESS_LEN);
			enable_set_config = true;
			break;
		case 'e':
			interface = ARTIK_ETHERNET;
			break;
		default:
			printf("Usage: network-test"\
				" [-a execute all tests] [-e for ethernet]"\
				" (wifi by default) [-i IP address]"\
				" [-n netmask] [-g gateway address]"\
				" [-c DNS address 1] [-d DNS address 2]\n");
			return 0;
		}
	}

	if (!execute_all_tests) {
		printf("Execute only get_online_status and"\
			" get_current_public_ip tests.\n");
		printf("i.e: Use option '-a' to execute"\
			" all tests (test_watch_online_status"\
			" shutdowns the wlan0 interface)\n");
	}

	ret = test_get_network_config(interface);
	if (ret != S_OK)
		goto exit;

	ret = test_get_online_status();
	if (ret != S_OK)
		goto exit;

	ret = test_get_current_public_ip();
	if (ret != S_OK)
		goto exit;

	if (execute_all_tests) {
		ret = test_watch_online_status();
		if (ret != S_OK)
			goto exit;
	}

	if (enable_set_config) {
		ret = test_set_network_config(&config, interface);
		if (ret != S_OK)
			goto exit;

		ret = test_get_network_config(interface);
	}


exit:
	return (ret == S_OK) ? 0 : -1;
}
