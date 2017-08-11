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
#include <fcntl.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>

#include <CUnit/Basic.h>

#include "unit_test.h"

#define MAX_BUF_LEN		256
#define UNIT_FILE		90
#define BT_ADDRESS_LEN		17
#define UUID_LEN		36

static char remote_mac_addr[BT_ADDRESS_LEN + 1];
static char uuid[UUID_LEN + 1];
static artik_bluetooth_module *bt;
static artik_error ret = S_OK;

static void read_remote_mac(const char *option);

static int init_suite1(void)
{
	return 0;
}

static int clean_suite1(void)
{
	return 0;
}

static void test_bt_scan_on(void)
{
	if (bt->is_scanning()) {
		fprintf(stdout, "  Scanning");
		ret = bt->start_scan();
		CU_ASSERT_NOT_EQUAL(ret, S_OK);
	} else {
		fprintf(stdout, "  Non Scanning");
		ret = bt->start_scan();
		CU_ASSERT_EQUAL(ret, S_OK);
	}
	fprintf(stdout, " --> Scanning  ");

	sleep(25);
}

static void test_bt_scan_off(void)
{
	if (bt->is_scanning()) {
		fprintf(stdout, "  Scanning");
		ret = bt->stop_scan();
		CU_ASSERT_EQUAL(ret, S_OK);
	} else {
		fprintf(stdout, "  Non Scanning");
		ret = bt->stop_scan();
		CU_ASSERT_NOT_EQUAL(ret, S_OK);
	}
	fprintf(stdout, " --> Non Scanning  ");

	sleep(10);
}

static void test_get_adapter_info(void)
{
	artik_bt_adapter adapter;

	ret = bt->get_adapter_info(&adapter);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_start_bond(void)
{
	printf("remote address : %s  ", remote_mac_addr);
	ret = bt->start_bond(remote_mac_addr);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_connect(void)
{
	sleep(15);

	if (bt->is_connected(remote_mac_addr)) {
		ret = bt->connect(remote_mac_addr);
		CU_ASSERT_NOT_EQUAL(ret, S_OK);
	} else {
		ret = bt->connect(remote_mac_addr);
		CU_ASSERT_EQUAL(ret, S_OK);
	}

	sleep(5);
}

static void test_disconnect(void)
{
	if (bt->is_connected(remote_mac_addr)) {
		ret = bt->disconnect(remote_mac_addr);
		CU_ASSERT_EQUAL(ret, S_OK);
	} else {
		ret = bt->disconnect(remote_mac_addr);
		CU_ASSERT_NOT_EQUAL(ret, S_OK);
	}

	sleep(10);
}

static void test_set_trust(void)
{
	ret = bt->set_trust(remote_mac_addr);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_unset_trust(void)
{
	ret = bt->unset_trust(remote_mac_addr);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_set_block(void)
{
	ret = bt->set_block(remote_mac_addr);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_unset_block(void)
{
	ret = bt->unset_block(remote_mac_addr);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_get_devices(void)
{
	artik_bt_device *devices = NULL;
	int device_num = 0;

	ret = bt->get_devices(&devices, &device_num);
	CU_ASSERT_EQUAL(ret, S_OK);

	ret = bt->free_devices(devices, device_num);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_get_paired_devices(void)
{
	artik_bt_device *devices = NULL;
	int device_num = 0;

	ret = bt->get_paired_devices(&devices, &device_num);
	CU_ASSERT_EQUAL(ret, S_OK);

	ret = bt->free_devices(devices, device_num);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_get_connected_devices(void)
{
	artik_bt_device *devices = NULL;
	int device_num = 0;

	ret = bt->get_connected_devices(&devices, &device_num);
	CU_ASSERT_EQUAL(ret, S_OK);

	ret = bt->free_devices(devices, device_num);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_remove_unpaired_devices(void)
{
	ret = bt->remove_unpaired_devices();
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_set_pairable_timeout(void)
{
	int timeout;

	srand(time(NULL));
	timeout = rand() % 10000;

	ret = bt->set_pairableTimeout(timeout);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_set_discoverable_timeout(void)
{
	int timeout;

	srand(time(NULL));
	timeout = rand() % 10000;

	ret = bt->set_discoverableTimeout(timeout);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_set_alias(void)
{
	char *alias = NULL;

	alias = "SamsungArtik";
	ret = bt->set_alias(alias);
	CU_ASSERT_EQUAL(ret, S_OK);

	alias = NULL;
}

static void test_set_pairable(void)
{
	ret = bt->set_pairable(true);
	CU_ASSERT_EQUAL(ret, S_OK);

	ret = bt->set_pairable(false);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_set_discoverable(void)
{
	ret = bt->set_discoverable(false);
	CU_ASSERT_EQUAL(ret, S_OK);

	ret = bt->set_discoverable(true);
	CU_ASSERT_EQUAL(ret, S_OK);
}

static void test_connect_profile(void)
{
	read_remote_mac("BLE");

	printf("  remote add : %s\n", remote_mac_addr);
	printf("  uuid : %s  ", uuid);

	ret = bt->connect_profile(remote_mac_addr, uuid);
	CU_ASSERT_EQUAL(ret, S_OK);
}

CU_TestInfo test_array[] = {
		{"1.  test of bt_scan_on", test_bt_scan_on },
		{"2.  test of bt_scan_off", test_bt_scan_off },
		{"3.  test of get_adapter_info", test_get_adapter_info },
		{"4.  test of start_bond", test_start_bond },
		{"5.  test of set_trust", test_set_trust },
		{"6.  test of set_alias", test_set_alias },
		{"7.  test of connect", test_connect },
		{"8.  test of disconnect", test_disconnect },
		{"9.  test of unset_trust", test_unset_trust },
		{"10. test of set_block", test_set_block },
		{"11. test of unset_block", test_unset_block },
		{"12. test of get_devices", test_get_devices },
		{"13. test of get_paired_devices", test_get_paired_devices },
		{"14. test of get_connected_devices", test_get_connected_devices },
		{"15. test of test of connect_profile", test_connect_profile },
		{"16. test of set_pairable_timeout", test_set_pairable_timeout },
		{"17. test of set_discoverable_timeout", test_set_discoverable_timeout },
		{"18. test of set_alias", test_set_alias },
		{"19. test of set_pairable", test_set_pairable },
		{"20. test of set_discoverable", test_set_discoverable },
		{"21. test of remove_unpaired_devices", test_remove_unpaired_devices },

		CU_TEST_INFO_NULL, };

static CU_SuiteInfo suites[] = {
		{ "bt_suite_test1", init_suite1, clean_suite1,
		NULL, NULL, test_array }, CU_SUITE_INFO_NULL, };

static void read_remote_mac(const char *option)
{
	FILE *fl = NULL;
	char buf[MAX_BUF_LEN];
	char *tmp = NULL;
	int file_end = 0;
	char *ret = NULL;

	fl = fopen("/etc/bluetooth/test/unit_test", "r");
	if (fl) {
		if (fseek(fl, 0L, SEEK_END)) {
			fprintf(stdout, "seek failed\n");
			fclose(fl);
			return;
		}
		file_end = ftell(fl);
		if (fseek(fl, 0L, SEEK_SET)) {
			fprintf(stdout, "seek failed\n");
			fclose(fl);
			return;
		}

		while (!feof(fl) && ftell(fl) < file_end) {
			ret = fgets(buf, MAX_BUF_LEN, fl);
			if (!ret) {
				fprintf(stdout, "fgets failed\n");
				goto exit;
			}
			tmp = strtok(buf, " ");

			if (!strcmp(tmp, option)) {
				tmp = strtok(NULL, " ");

				if (strlen(tmp) == BT_ADDRESS_LEN) {
					strcpy(remote_mac_addr, tmp);

					if (!strcmp(option, "BT/EDR"))
						goto exit;
				}
			} else if (!strcmp(tmp, "UUID")) {
				tmp = strtok(NULL, " ");

				if (strlen(tmp) == UUID_LEN) {
					strcpy(uuid, tmp);
					goto exit;
				}
			}
		}
exit: fclose(fl);
	}
}

static void write_mac_info(void)
{
	char *tutorial = "unit test formation\n"
			"BT/EDR xx:xx:xx:xx:xx:xx\n"
			"BLE xx:xx:xx:xx:xx:xx\n"
			"UUID 0000xxxx-0000-1000-8000-00805f9b34fb\n";
	char *result;
	char input[UUID_LEN];

	int fd;

	fd = creat("/etc/bluetooth/test/unit_test", 0644);
	if (fd < 0) {
		fprintf(stdout, "file create failed\n");
		return;
	}
	result = (char *) malloc(sizeof(char) * (strlen(tutorial) + 5));
	fprintf(stdout, "%s\n\n", tutorial);

	while (1) {
		fprintf(stdout, "BT/EDR : ");
		if (fscanf(stdin, "%s", input) == -1) {
			fprintf(stdout, "\ncmd fscanf error!\n");
			break;
		}

		if (strlen(input) == BT_ADDRESS_LEN)
			break;

		fprintf(stdout, "\n17 characters are expected, please input again\n");
	}

	strcpy(result, "UNIT TEST Formation\nBT/EDR");
	strcat(result, input);

	strcat(result, "\nBLE ");

	while (1) {
		fprintf(stdout, "BLE : ");
		if (fscanf(stdin, "%s", input) == -1) {
			fprintf(stdout, "\ncmd fscanf error!\n");
			break;
		}

		if (strlen(input) == BT_ADDRESS_LEN)
			break;

		fprintf(stdout, "\n17 characters are expected, please input again\n");
	}

	strcat(result, input);
	strcat(result, "\nUUID 0000");

	while (1) {
		fprintf(stdout, "UUID 0000xxxx-0000-1000-8000-00805f9b34fb :");
		if (fscanf(stdin, "%s", input) == -1) {
			fprintf(stdout, "\ncmd fscanf error!\n");
			break;
		}
		if (strlen(input) == 4)
			break;

		fprintf(stdout,
				"\nOnly 4 characters are expected, please input again\n");
	}

	strcat(result, input);
	strcat(result, "-0000-1000-8000-00805f9b34fb\n");

	printf("\n%s\n", result);
	if (write(fd, result, strlen(result)) == -1)
		fprintf(stdout, "\ncmd write error!\n");

	close(fd);
	free(result);
}

static void create_test_file(void)
{
	FILE *fp;

	fp = fopen("/etc/bluetooth/test/unit_test", "r");

	if (fp == NULL) {
		write_mac_info();
	} else {
		if (fseek(fp, 0L, SEEK_END)) {
			fprintf(stdout, "seek failed\n");
			fclose(fp);
			return;
		}

		if (ftell(fp) < UNIT_FILE) {
			fclose(fp);
			write_mac_info();
		} else
			fclose(fp);
	}
}

int main(int argc, char *argv[])
{
	CU_initialize_registry();

	if (CU_register_suites(suites) != CUE_SUCCESS) {
		fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
		exit(EXIT_FAILURE);
	}

	bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");

	create_test_file();
	read_remote_mac("BT/EDR");

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	artik_release_api_module(bt);

	return 0;
}
