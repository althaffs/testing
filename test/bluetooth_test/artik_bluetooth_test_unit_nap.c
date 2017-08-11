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
#include <time.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>
#include <CUnit/Basic.h>

#define BUFFER_LEN			128
#define PROC_BUFFER_SIZE	1024
#define PATH_PROC_NET_DEV	"/proc/net/dev"
#define UUID				"nap"

static char *bridge;
static char *nap_ip;
static char *nap_netmask;
static char *dhcp_begin_ip;
static char *dhcp_end_ip;

static int init_suite1(void)
{
	printf("%s\n", __func__);
	return 0;
}

static int clean_suite1(void)
{
	printf("%s\n", __func__);
	return 0;
}

static char *_interface_name_cut(char *buf, char **name)
{
	char *stat;

	if (!name)
		return NULL;
	while (' ' == *buf)
		buf++;
	*name = buf;
	stat = strrchr(buf, ':');
	*stat++ = '\0';

	return stat;
}

static int _check_interface_from_proc(const char *interface)
{
	FILE *fp = NULL;
	char buf[PROC_BUFFER_SIZE];
	char *name = NULL;

	fp = fopen(PATH_PROC_NET_DEV, "r");
	if (!fp)
		return -1;

	if (fgets(buf, PROC_BUFFER_SIZE, fp) == NULL) {
		fclose(fp);
		return -1;
	}
	if (fgets(buf, PROC_BUFFER_SIZE, fp) == NULL) {
		fclose(fp);
		return -1;
	}

	while (!fgets(buf, PROC_BUFFER_SIZE, fp)) {
		_interface_name_cut(buf, &name);
		if (!strcmp(interface, name)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

static artik_error _pan_uinit(void)
{
	char buf[BUFFER_LEN];

	sprintf(buf, "ifconfig %s down", bridge);
	if (system(buf) < 0) {
		printf("cmd system error\n");
		return E_BT_ERROR;
	}
	sprintf(buf, "brctl delbr %s", bridge);
	if (system(buf) < 0) {
		printf("cmd system error\n");
		return E_BT_ERROR;
	}
	if (system("pkill -9 dnsmasq") < 0) {
		printf("cmd system error\n");
		return E_BT_ERROR;
	}
	strcpy(buf, "iptables -t nat -D POSTROUTING -s 10.0.0.1/255.255.255.0 -j MASQUERADE");
	if (system(buf) < 0) {
		printf("cmd system error\n");
		return E_BT_ERROR;
	}

	return S_OK;
}

static artik_error _pan_init(void)
{
	int status = -1;
	char buf[BUFFER_LEN];

	if (_check_interface_from_proc(bridge) > 0) {
		sprintf(buf, "brctl delbr %s", bridge);
		if (system(buf) < 0)
			return E_BT_ERROR;
	}

	sprintf(buf, "brctl addbr %s", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	sprintf(buf, "ip addr add %s dev %s", nap_ip, bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	sprintf(buf, "ip link set %s up", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	status = system("echo 1 > /proc/sys/net/ipv4/ip_forward");
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	sprintf(buf, "ifconfig %s netmask %s up", bridge, nap_netmask);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	sprintf(buf, "iptables -t nat -A POSTROUTING -s %s/%s -j MASQUERADE",
		nap_ip, nap_netmask);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	sprintf(buf, "iptables -t filter -A FORWARD -i %s -j ACCEPT", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	sprintf(buf, "iptables -t filter -A FORWARD -o %s -j ACCEPT", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	sprintf(buf, "iptables -t filter -A FORWARD -i %s -j ACCEPT", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	char dhcp_cmd[256] = {0};

	strcat(dhcp_cmd, "/usr/sbin/dnsmasq --pid-file=/var/run/dnsmasq.%s.pid ");
	strcat(dhcp_cmd, "--bind-interfaces --dhcp-range=%s,%s,60m ");
	strcat(dhcp_cmd, "--except-interface=lo --interface=%s ");
	strcat(dhcp_cmd, "--dhcp-option=option:router,%s");
	sprintf(buf, dhcp_cmd, bridge, dhcp_begin_ip,
		dhcp_end_ip, bridge, nap_ip);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status))
		return E_BT_ERROR;

	return S_OK;
}

static void pan_register_test(void)
{
	artik_error ret = S_OK;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");
	const char *test_uuid = "test_uuid";

	ret = bt->pan_register(test_uuid, bridge);
	CU_ASSERT(ret != S_OK);

	ret = _pan_init();
	CU_ASSERT(ret == S_OK);

	ret = bt->pan_register(UUID, bridge);
	CU_ASSERT(ret == S_OK);

	ret = bt->pan_unregister(UUID);
	CU_ASSERT(ret == S_OK);

	ret = _pan_uinit();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

static void pan_unregister_test(void)
{
	artik_error ret = S_OK;
	artik_bluetooth_module *bt = (artik_bluetooth_module *)
		artik_request_api_module("bluetooth");

	ret = _pan_init();
	CU_ASSERT(ret == S_OK);

	ret = bt->pan_unregister(UUID);
	CU_ASSERT(ret != S_OK);

	ret = bt->pan_register(UUID, bridge);
	CU_ASSERT(ret == S_OK);

	ret = bt->pan_unregister(UUID);
	CU_ASSERT(ret == S_OK);

	ret = _pan_uinit();
	CU_ASSERT(ret == S_OK);

	artik_release_api_module(bt);
}

artik_error cunit_add_suite(CU_pSuite *psuite)
{
	CU_add_test(*psuite, "pan_register_test",
				pan_register_test);
	CU_add_test(*psuite, "pan_unregister_test",
				pan_unregister_test);

	return S_OK;
}

artik_error cunit_init(CU_pSuite *psuite)
{
	artik_error ret = S_OK;

	fprintf(stdout, "cunit init!\n");

	if (CU_initialize_registry() != CUE_SUCCESS)
		return CU_get_error();
	*psuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
	if (*psuite == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	ret = cunit_add_suite(psuite);

	return ret;
}

artik_error nap_info_get(void)
{
	char input[BUFFER_LEN];
	int ret = 0;

	fprintf(stdout, "bridge: ");
	ret = fscanf(stdin, "%s", input);
	if (ret == -1)
		return E_INVALID_VALUE;
	fprintf(stdout, "remote address: %s-%zd\n",
		input, strlen(input));
	bridge = (char *)malloc(strlen(input) + 1);
	strncpy(bridge, input, strlen(input));
	bridge[strlen(input)] = '\0';

	fprintf(stdout, "NAP IP addr: ");
	ret = fscanf(stdin, "%s", input);
	if (ret == -1)
		return E_INVALID_VALUE;
	nap_ip = (char *)malloc(strlen(input) + 1);
	strncpy(nap_ip, input, strlen(input));
	nap_ip[strlen(input)] = '\0';

	fprintf(stdout, "NAP netmask: ");
	ret = fscanf(stdin, "%s", input);
	if (ret == -1)
		return E_INVALID_VALUE;
	nap_netmask = (char *)malloc(strlen(input) + 1);
	strncpy(nap_netmask, input, strlen(input));
	nap_netmask[strlen(input)] = '\0';

	fprintf(stdout, "DHCP begin IP: ");
	ret = fscanf(stdin, "%s", input);
	if (ret == -1)
		return E_INVALID_VALUE;
	dhcp_begin_ip = (char *)malloc(strlen(input) + 1);
	strncpy(dhcp_begin_ip, input, strlen(input));
	dhcp_begin_ip[strlen(input)] = '\0';

	fprintf(stdout, "DHCP end IP: ");
	ret = fscanf(stdin, "%s", input);
	if (ret == -1)
		return E_INVALID_VALUE;
	dhcp_end_ip = (char *)malloc(strlen(input) + 1);
	strncpy(dhcp_end_ip, input, strlen(input));
	dhcp_end_ip[strlen(input)] = '\0';

	return S_OK;
}

int main(void)
{
	artik_error ret = S_OK;
	CU_pSuite pSuite = NULL;

	ret = cunit_init(&pSuite);
	if (ret != S_OK) {
		fprintf(stdout, "cunit init error!\n");
		goto loop_quit;
	}
	fprintf(stdout, "cunit init success!\n");

	ret = nap_info_get();

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

loop_quit:
	CU_cleanup_registry();
	if (_pan_uinit() != S_OK)
		fprintf(stdout, "pan unit err!\n");

	if (bridge)
		free(bridge);
	if (nap_ip)
		free(nap_ip);
	if (nap_netmask)
		free(nap_netmask);
	if (dhcp_begin_ip)
		free(dhcp_begin_ip);
	if (dhcp_end_ip)
		free(dhcp_end_ip);

	return S_OK;
}

