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

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_bluetooth.h>
#include "artik_bluetooth_test_commandline.h"

#define MAX_BUFFER_SIZE 1024

static artik_bluetooth_module *bt;
static artik_loop_module *loop;
static int g_quit;

void print_devices(artik_bt_device *devices, int num)
{
	int i = 0, j = 0;

	for (i = 0; i < num; i++) {
		fprintf(stdout, "Address: %s\n",
			devices[i].remote_address ? devices[i].remote_address : "(null)");
		fprintf(stdout, "Name: %s\n",
			devices[i].remote_name ? devices[i].remote_name : "(null)");
		fprintf(stdout, "Bonded: %s\n",
			devices[i].is_bonded ? "true" : "false");
		fprintf(stdout, "Connected: %s\n",
			devices[i].is_connected ? "true" : "false");
		fprintf(stdout, "Authorized: %s\n",
			devices[i].is_authorized ? "true" : "false");
		if (devices[i].uuid_length > 0) {
			fprintf(stdout, "UUIDs:\n");
			for (j = 0; j < devices[i].uuid_length; j++) {
				fprintf(stdout, "\t%s [%s]\n",
					devices[i].uuid_list[j].uuid_name,
					devices[i].uuid_list[j].uuid);
			}
		}

		fprintf(stdout, "\n");
		fprintf(stdout, "Scaning......\n");
	}
}

static void scan_callback(artik_bt_event event, void *data, void *user_data)
{
	artik_bt_device *dev = (artik_bt_device *) data;

	print_devices(dev, 1);
}

static void on_timeout_callback(void *user_data)
{
	fprintf(stdout, "TEST: %s stop scanning, exiting loop\n", __func__);
	bt->stop_scan();
	bt->unset_callback(BT_EVENT_SCAN);

	loop->quit();
}

static void prv_start_scan(char *buffer, void *user_data)
{
	artik_error ret = S_OK;
	int timeout_id = 0;
	artik_bt_device *paired_device = NULL;
	int paired_device_num = 0;

	bt->get_paired_devices(&paired_device, &paired_device_num);
	if (paired_device_num) {
		printf("\nPaired devices:\n");
		print_devices(paired_device, paired_device_num);
		bt->free_devices(paired_device, paired_device_num);
	}

	ret = bt->set_callback(BT_EVENT_SCAN, scan_callback, "avrcp test");
	if (ret != S_OK) {
		printf("Set scan callback failed, Error[%d]\n", ret);
		return;
	}

	loop->add_timeout_callback(&timeout_id, 30000, on_timeout_callback,
				   NULL);

	printf("\nInvoke scan for 30 seconds...\n");
	ret = bt->start_scan();
	if (ret != S_OK)
		printf("Start scan failed\n");
	loop->run();
}

static void prv_connect(char *buffer, void *user_data)
{
	if (buffer == NULL) {
		printf("Please input device mac address you want to connect!\n");
		return;
	}
	artik_error error = S_OK;
	char *address = malloc(strlen(buffer));

	strncpy(address, buffer, strlen(buffer));
	address[strlen(buffer) - 1] = '\0';

	printf("Invoke connect to device [%s]...\n", address);
	error = bt->connect(address);
	if (error == S_OK)
		printf("Connect to device [%s] success\n", address);
	else
		printf("Connect to device [%s] failed\n", address);
}

static void prv_disconnect(char *buffer, void *user_data)
{
	if (buffer == NULL) {
		printf("\nPlease input device mac address you want to disconnect!\n");
		return;
	}
	artik_error error = S_OK;
	char *address = malloc(strlen(buffer));

	strncpy(address, buffer, strlen(buffer));
	address[strlen(buffer) - 1] = '\0';

	printf("Invoke disconnect to device [%s]...\n", address);
	error = bt->disconnect(address);
	if (error == S_OK)
		printf("Disconnect to device [%s] success\n", address);
	else
		printf("Disconnect to device [%s] failed\n", address);
}

static void prv_list_items(char *buffer, void *user_data)
{
	artik_error ret = S_OK;
	int start_item = -1;
	int end_item = -1;

	printf("Invoke list item...\n");

	if (strlen(buffer) > 0) {
		char **argv = NULL;
		char *arg = malloc(strlen(buffer));

		strncpy(arg, buffer, strlen(buffer) - 1);
		arg[strlen(buffer) - 1] = '\0';
		argv = g_strsplit(arg, " ", -1);
		start_item = strtol(argv[0], NULL, 10);
		end_item = strtol(argv[1], NULL, 10);
		g_strfreev(argv);
	}

	artik_bt_avrcp_item *item_list, *node;

	ret = bt->avrcp_controller_list_item(start_item, end_item, &item_list);
	if (ret != S_OK) {
		printf("avrcp list item failed !\n");
		return;
	}

	node = item_list;

	while (node != NULL) {
		printf("item_obj_path : %s\n", node->item_obj_path);
		artik_bt_avrcp_item_property *property = node->property;

		if (property != NULL) {
			printf("Player object path: %s\n", property->player);
			printf("Displayable name: %s\n", property->name);
			printf("Type: %s\n", property->type);
			printf("Folder: %s\n", property->folder);
			printf("Playable: %d\n", property->playable);
			printf("Title: %s\n", property->title);
			printf("Artist: %s\n", property->artist);
			printf("Album: %s\n", property->album);
			printf("Genre: %s\n", property->genre);
			printf("Number of tracks: %d\n", property->number_of_tracks);
			printf("track #%d\n", property->number);
			printf("duration: %d ms\n", property->duration);
		}
		node = node->next_item;
	}
}

static void prv_change_folder(char *buffer, void *user_data)
{
	artik_error ret = S_OK;
	char *folder = malloc(strlen(buffer));

	strncpy(folder, buffer, strlen(buffer));
	folder[strlen(buffer) - 1] = '\0';

	printf("Invoke change folder...%s\n", buffer);
	ret = bt->avrcp_controller_change_folder(folder);
	free(folder);
	if (ret != S_OK)
		printf("avrcp change folder failed !\n");
}

static void prv_get_repeat_mode(char *buffer, void *user_data)
{
	artik_error ret = S_OK;
	artik_bt_avrcp_repeat_mode repeat_mode;

	ret = bt->avrcp_controller_get_repeat_mode(&repeat_mode);

	if (ret != S_OK) {
		printf("avrcp get repeat mode failed !\n");
		return;
	}

	if (repeat_mode == BT_AVRCP_REPEAT_SINGLETRACK)
		printf("Repeat mode is single\n");
	else if (repeat_mode == BT_AVRCP_REPEAT_ALLTRACKS)
		printf("Repeat mode is all\n");
	else if (repeat_mode == BT_AVRCP_REPEAT_GROUP)
		printf("Repeat mode is group\n");
	else if (repeat_mode == BT_AVRCP_REPEAT_OFF)
		printf("Repeat mode is off\n");
}

static void prv_set_repeat_mode(char *buffer, void *user_data)
{
	artik_error ret = S_OK;
	int repeat;
	char *repeat_mode = malloc(strlen(buffer));

	strncpy(repeat_mode, buffer, strlen(buffer));
	repeat_mode[strlen(buffer) - 1] = '\0';
	if (strcmp(repeat_mode, "single") == 0) {
		repeat = 0;
	} else if (strcmp(repeat_mode, "all") == 0) {
		repeat = 1;
	} else if (strcmp(repeat_mode, "group") == 0) {
		repeat = 2;
	} else if (strcmp(repeat_mode, "off") == 0) {
		repeat = 3;
	} else {
		printf("invalid repeat mode\n");
		return;
	}

	ret = bt->avrcp_controller_set_repeat_mode(repeat);

	if (ret != S_OK)
		printf("avrcp set repeat mode failed !\n");
}

static void prv_play_item(char *buffer, void *user_data)
{
	artik_error ret = S_OK;
	char *item = malloc(strlen(buffer));

	strncpy(item, buffer, strlen(buffer));
	item[strlen(buffer) - 1] = '\0';
	ret = bt->avrcp_controller_play_item(item);
	free(item);

	if (ret != S_OK)
		printf("avrcp play_item failed !\n");
}

static void prv_add_to_playing(char *buffer, void *user_data)
{
	artik_error ret = S_OK;
	char *item = malloc(strlen(buffer));

	strncpy(item, buffer, strlen(buffer));
	item[strlen(buffer) - 1] = '\0';
	printf("add to playing item:%s\n", buffer);
	ret = bt->avrcp_controller_add_to_playing(item);
	free(item);
	if (ret != S_OK)
		printf("avrcp add_to_playing failed !\n");
}

static void prv_resume_play(char *buffer, void *user_data)
{
	bt->avrcp_controller_resume_play();
}

static void prv_next(char *buffer, void *user_data)
{
	bt->avrcp_controller_next();
}

static void prv_previous(char *buffer, void *user_data)
{
	bt->avrcp_controller_previous();
}

static void prv_pause(char *buffer, void *user_data)
{
	bt->avrcp_controller_pause();
}

static void prv_stop(char *buffer, void *user_data)
{
	bt->avrcp_controller_stop();
}

static void prv_rewind(char *buffer, void *user_data)
{
	bt->avrcp_controller_rewind();
}

static void prv_fast_forward(char *buffer, void *user_data)
{
	bt->avrcp_controller_fast_forward();
}

static void prv_quit(char *buffer, void *user_data)
{
	g_quit = 1;
	if (bt)
		artik_release_api_module(bt);
	if (loop)
		artik_release_api_module(loop);
}

command_desc_t commands[] = {
		{"scan", "Start to scan bluetooth device around.", "scan",
			prv_start_scan, NULL},
		{"connect", "Connect to certain device address.",
			"connect 54:40:AD:E2:BE:35", prv_connect, NULL},
		{"disconnect", "Connect to certain device address.",
			"disconnect 54:40:AD:E2:BE:35", prv_disconnect, NULL},
		{"list-item", "List items of current folder.",
			"list-item or list-item 1 2", prv_list_items, NULL},
		{"change-folder", "Change the folder.",
			"change-folder /org/bluez/hci0/dev_54_40_AD_E2_BE_35/player0"\
			"/Filesystem/item3/item1", prv_change_folder, NULL},
		{"get-repeat", "Get the repeat mode", "get-repeat",
			prv_get_repeat_mode, NULL},
		{"set-repeat", "Set the repeat mode",
			"set-repeat single/set-repeat all/set-repeat group/set-repeat off",
			prv_set_repeat_mode, NULL},
		{"play-item", "Play the item",
			"play-item /org/bluez/hci0/dev_54_40_AD_E2_BE_35/player0/"\
			"Filesystem/item3/item1/item1",
			prv_play_item, NULL},
		{"addtoplay", "Add the item to playlist.",
			"addtoplay /org/bluez/hci0/dev_54_40_AD_E2_BE_35/player0/"\
			"Filesystem/item3/item1/item1",
			prv_add_to_playing, NULL},
		{"resume-play", "Resume play.", "resume-play", prv_resume_play, NULL},
		{"next", "Play the next music.", "next", prv_next, NULL},
		{"previous", "Play the previous music.", "previous",
			prv_previous, NULL},
		{"pause", "Pause the playing music.", "pause", prv_pause, NULL},
		{"stop", "Stop playing music.", "stop", prv_stop, NULL},
		{"rewind", "Rewind the playing music.", "rewind", prv_rewind, NULL},
		{"fast-forward", "Fast-forward the playing music.", "fast-forward",
			prv_fast_forward, NULL},
		{"quit", "Quit application.", "quit", prv_quit, NULL},

		COMMAND_END_LIST
};

int main(void)
{
	artik_error ret = S_OK;
	int i = 0;

	if (!artik_is_module_available(ARTIK_MODULE_BLUETOOTH)) {
		printf("TEST:Bluetooth module is not available, skipping test...\n");
		return -1;
	}

	bt = (artik_bluetooth_module *) artik_request_api_module("bluetooth");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	for (i = 0; commands[i].name != NULL; i++)
		printf("Command:[%s]\tDescription: %s\tExample: %s\n",
			commands[i].name,
			commands[i].short_desc,
			commands[i].long_desc);

	printf("\nPlease input command:\n");

	while (!g_quit) {
		fd_set readfds;
		int num_bytes;
		int result;
		char buffer[MAX_BUFFER_SIZE];
		struct timeval tv;

		tv.tv_sec = 60;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
		if (result < 0) {
			if (errno != EINTR) {
				fprintf(stderr, "Error in select(): %d %s\r\n", errno,
						strerror(errno));
			}
		} else if (result > 0) {
			num_bytes = read(STDIN_FILENO, buffer, MAX_BUFFER_SIZE - 1);

			if (num_bytes > 1) {
				buffer[num_bytes] = 0;
				handle_command(commands, (char *) buffer);
			}
			if (g_quit == 0) {
				fprintf(stdout, "\r\n> ");
				fflush(stdout);
			} else {
				fprintf(stdout, "\r\n");
			}
		}

	}

	if (ret != S_OK)
		printf("Test AVRCP failed!\n");

	return (ret == S_OK) ? 0 : -1;
}
