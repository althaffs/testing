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

#ifndef _ARTIK_ZIGBEE_TEST_COMMON_H_
#define _ARTIK_ZIGBEE_TEST_COMMON_H_

#include <artik_zigbee.h>

#define ENDPOINT_ID_MIN		1
#define ENDPOINT_ID_MAX		239
#define KEYBOARD_INPUT_SIZE	100
#define DEFAULT_TEST_CHANNEL	25
#define DEFAULT_TEST_TX_POWER	ARTIK_ZIGBEE_TX_POWER_2
#define DEFAULT_TEST_PANID	0x1234
#define CHANNEL_MIN		11
#define CHANNEL_MAX		26
#define TX_MIN			(-9)
#define TX_MAX			8
#define INVALID_TX		(-100)

enum timer_command {
	TIMER_CS_TARGET_TIMEOUT
};

enum cs_target_command {
	CS_TARGET_START,
	CS_TARGET_STOP,
	CS_TARGET_TIMEOUT,
	CS_TARGET_SUCCESS,
	CS_TARGET_FAILED
};

enum cs_initiator_command {
	CS_INITIATOR_START,
	CS_INITIATOR_STOP,
	CS_INITIATOR_SUCCESS,
	CS_INITIATOR_FAILED
};

extern char *get_device_name(int device_id);
typedef void(*func)(char *input, int max_size);
typedef void(*timer_func)(enum timer_command cmd, int id, void *user_data);

struct st_timer {
	enum timer_command command;
	int id;
	timer_func func;
	void *user_data;
};

struct device_type {
	ARTIK_ZIGBEE_PROFILE profile;
	ARTIK_ZIGBEE_DEVICEID device_id;
	int default_endpoint_id;
	char *name;
	char *arg_name;
};

struct test_device {
	artik_zigbee_endpoint_handle handle;
	ARTIK_ZIGBEE_PROFILE profile;
	ARTIK_ZIGBEE_DEVICEID device_id;
	int endpoint_id;

	artik_error(*ezmode_commissioning_initiator_start)(
					artik_zigbee_endpoint_handle handle);
	artik_error(*ezmode_commissioning_initiator_stop)(
					artik_zigbee_endpoint_handle handle);
	artik_error(*ezmode_commissioning_target_start)(
					artik_zigbee_endpoint_handle handle);
	artik_error(*ezmode_commissioning_target_stop)(
					artik_zigbee_endpoint_handle handle);
	artik_error(*identify_request)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int duration);
	artik_error(*identify_get_remaining_time)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int *time);
	artik_error(*groups_get_local_name_support)(
					artik_zigbee_endpoint_handle handle,
					int endpoint);
	artik_error(*groups_set_local_name_support)(
					artik_zigbee_endpoint_handle handle,
					bool support);
	artik_error(*onoff_command)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					artik_zigbee_onoff_status target_status
					);
	artik_error(*onoff_get_value)(artik_zigbee_endpoint_handle handle,
					artik_zigbee_onoff_status * status);
	artik_error(*level_control_request)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					const artik_zigbee_level_control_command
						*command);
	artik_error(*level_control_get_value)(
					artik_zigbee_endpoint_handle handle,
					int *value);
	artik_error(*illum_set_measured_value_range)(
					artik_zigbee_endpoint_handle handle,
					int min, int max);
	artik_error(*illum_set_measured_value)(
					artik_zigbee_endpoint_handle handle,
					int value);
	artik_error(*illum_get_measured_value)(
					artik_zigbee_endpoint_handle handle,
					int *value);
	artik_error(*reset_to_factory_default)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint);
	artik_error(*request_reporting)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					artik_zigbee_reporting_type report_type,
					int min_interval, int max_interval,
					int change_threshold);
	artik_error(*stop_reporting)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					artik_zigbee_reporting_type report_type
					);
};

int read_int(char *input, int max_size, int default_value);
bool read_q(char *input, int max_size);
bool read_e(char *input, int max_size);
bool read_lf(char *input, int max_size);
int read_input_int(int default_value);
void show(const char *format, ...);
void showln(const char *format, ...);
void show_hyphen(void);
void show_retry(void);
void show_range(int min, int max);
void show_select(void);
void show_network_status(int status);
void show_node_type(int type);
void show_device(artik_zigbee_device *device);
void show_device_info(artik_zigbee_device_info *device_info);
artik_error read_channel(char *input, int max_size, int *channel);
artik_error read_tx(char *input, int max_size, artik_zigbee_tx_power *tx);
int read_pan_id(char *input, int max_size, int *pan_id);
void show_request_channel(void);
void show_request_tx(void);
void show_request_pan_id(void);
struct test_device *add_test_device(ARTIK_ZIGBEE_PROFILE profile,
					ARTIK_ZIGBEE_DEVICEID device_id,
					int endpoint_id);
void delete_test_device(int endpoint_id);
void release_all_test_devices(artik_zigbee_module *zb);
int get_test_device_count(void);
bool check_test_device_endpoint_id(int endpoint_id);
void get_test_device_list(artik_zigbee_endpoint_handle *handle_list,
			int max_size, int *size);
struct test_device *get_test_device(int index);
struct test_device *get_test_device_by_endpoint_id(int endpoint_id);
int add_timer(timer_func func, enum timer_command cmd, int second,
			  void *user_data);
void *remove_timer(int id);
void exit_loop(void);

#endif /* _ARTIK_ZIGBEE_TEST_COMMON_H_ */
