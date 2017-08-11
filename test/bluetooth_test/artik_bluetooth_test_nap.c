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

#define DISCOVERABLE_TIMEOUT 120
#define CMD_LENGTH 256
#define PROC_BUFFER_SIZE 1024
#define PATH_PROC_NET_DEV "/proc/net/dev"

typedef void (*signal_fuc)(int);

static artik_bluetooth_module *bt;
static artik_loop_module *loop;
static artik_error err;

static char *interface_name_cut(char *buf, char **name)
{
	char *stat;

	if (!name)
		return NULL;
	/* Skip white space.  Line will include header spaces. */
	while (' ' == *buf)
		buf++;
	*name = buf;
	/* Cut interface name. */
	stat = strrchr(buf, ':');
	*stat++ = '\0';

	return stat;
}

static int check_interface_from_proc(const char *interface)
{
	FILE *fp = NULL;
	char buf[PROC_BUFFER_SIZE];
	char *name = NULL;

	/* Open /proc/net/dev. */
	fp = fopen(PATH_PROC_NET_DEV, "r");
	if (!fp) {
		printf("open proc file error\n");
		return -1;
	}

	/* Drop header lines. */
	if (fgets(buf, PROC_BUFFER_SIZE, fp) == NULL) {
		printf("fgets error\n");
		fclose(fp);
		return -1;
	}
	if (fgets(buf, PROC_BUFFER_SIZE, fp) == NULL) {
		printf("fgets error\n");
		fclose(fp);
		return -1;
	}

	/*
	 * Only allocate interface structure.  Other jobs will be done in
	 * if_ioctl.c.
	 */
	while (!fgets(buf, PROC_BUFFER_SIZE, fp)) {
		interface_name_cut(buf, &name);
		if (!strcmp(interface, name)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

static artik_error test_bluetooth_nap(char *bridge)
{
	artik_error ret = S_OK;

	printf("Invoke pan register...\n");
	ret = bt->pan_register("nap", bridge);

	if (ret == S_OK) {
		bt->set_discoverableTimeout(DISCOVERABLE_TIMEOUT);
		bt->set_discoverable(true);
	} else {
		printf("register pan failed\n");
		return ret;
	}

	return ret;
}

static int uninit(void *user_data)
{
	err = bt->pan_unregister("nap");
	if (err != S_OK)
		printf("Unregister Error:%d!\r\n", err);
	else
		printf("Unregister OK!\r\n");

	loop->quit();

	return true;
}

int main(int argc, char *argv[])
{
	artik_error ret = S_OK;
	char *bridge = NULL;
	int status = -1;
	char buf[CMD_LENGTH];
	char *nap_ip;
	char *nap_netmask;
	char *nap_dhcp_begin;
	char *nap_dhcp_end;

	if (argc < 6) {
		printf("Input parameter Error!\r\n");
		printf("Please input as: ./test-nap bridge ip netmask dhcp_ip_begin dhcp_ip_end!\r\n");
		return -1;
	}
	bridge = argv[1];
	nap_ip = argv[2];
	nap_netmask = argv[3];
	nap_dhcp_begin = argv[4];
	nap_dhcp_end = argv[5];

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		printf("TEST:Bluetooth module is not available,skipping test...\n");
		return -1;
	}

	if (!artik_is_module_available(ARTIK_MODULE_LOOP)) {
		printf("TEST:Loop module is not available, skipping test...\n");
		return -1;
	}

	if (check_interface_from_proc(bridge) > 0) {
		sprintf(buf, "brctl delbr %s", bridge);
		if (system(buf) < 0) {
			printf("cmd system error\n");
			return -1;
		}
	}

	sprintf(buf, "brctl addbr %s", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("config network bridge failed\r\n");
		goto out;
	}

	sprintf(buf, "ip addr add %s dev %s", nap_ip, bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("config %s address failed\r\n", bridge);
		goto out;
	}

	sprintf(buf, "ip link set %s up", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("up %s failed\r\n", bridge);
		goto out;
	}

	status = system("echo 1 > /proc/sys/net/ipv4/ip_forward");
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("enable ip forward failed\r\n");
		goto out;
	}

	sprintf(buf, "ifconfig %s netmask %s up", bridge, nap_netmask);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("up %s failed\r\n", bridge);
		goto out;
	}

	sprintf(buf, "iptables -t nat -A POSTROUTING -s %s/%s "
	       "-j MASQUERADE", nap_ip, nap_netmask);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("configure the iptable failed\r\n");
		goto out;
	}

	sprintf(buf, "iptables -t filter -A FORWARD -i %s -j ACCEPT", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("configure the iptable failed\r\n");
		goto out;
	}

	sprintf(buf, "iptables -t filter -A FORWARD -o %s -j ACCEPT", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("configure the iptable failed\r\n");
		goto out;
	}

	sprintf(buf, "iptables -t filter -A FORWARD -i %s -j ACCEPT", bridge);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("configure the iptable failed\r\n");
		goto out;
	}

	sprintf(buf, "/usr/sbin/dnsmasq --pid-file=/var/run/dnsmasq.%s.pid "
		"--bind-interfaces --dhcp-range=%s,%s,60m "
		"--except-interface=lo --interface=%s "
		"--dhcp-option=option:router,%s", bridge, nap_dhcp_begin,
		nap_dhcp_end, bridge, nap_ip);
	status = system(buf);
	if (-1 == status || !WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
		printf("configure the iptable failed\r\n");
		goto out;
	}

	bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)
			artik_request_api_module("loop");
	if (!bt || !loop)
		goto out;
	err = test_bluetooth_nap(bridge);
	if (err != S_OK) {
		printf("Register return with error: %d!\r\n", err);
		printf("Invoke pan unregister...\n");
		err = bt->pan_unregister("nap");
		return -1;
	}
	printf("<NAP> Rgister return is OK:%d!\r\n", err);
	loop->add_signal_watch(SIGINT, uninit, NULL, NULL);
	sleep(1);

	loop->run();
out:
	if (bt)
		artik_release_api_module(bt);
	if (loop)
		artik_release_api_module(loop);
	sprintf(buf, "ifconfig %s down", bridge);
	if (system(buf) < 0) {
		printf("cmd system error\n");
		return -1;
	}
	sprintf(buf, "brctl delbr %s", bridge);
	if (system(buf) < 0) {
		printf("cmd system error\n");
		return -1;
	}
	if (system("pkill -9 dnsmasq") < 0) {
		printf("cmd system error\n");
		return -1;
	}
	strcpy(buf, "iptables -t nat -D POSTROUTING -s 10.0.0.1/255.255.255.0 "
		"-j MASQUERADE");
	if (system(buf) < 0) {
		printf("cmd system error\n");
		return -1;
	}

	return (ret == S_OK) ? 0 : -1;
}
