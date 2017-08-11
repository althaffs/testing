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

#ifndef __ARTIK_BT_A2DP_H
#define __ARTIK_BT_A2DP_H

#include <stdbool.h>
#include <artik_bluetooth.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define STATE_IDLE		"idle"
#define STATE_PENDING	"pending"
#define STATE_ACTIVE		"active"

typedef enum {
	BT_A2DP_CODEC_SBC = 0x00,
	BT_A2DP_CODEC_MPEG12 = 0x01,
	BT_A2DP_CODEC_MPEG24 = 0x02,
	BT_A2DP_CODEC_ATRAC = 0x03
} artik_bt_a2dp_codec;

typedef struct {
	artik_bt_a2dp_codec codec;
	bool delay_reporting;
	char *endpoint_path;
	char *capabilities;
	int cap_size;
	char *transport_path;
	select_config_callback select_callback;
	set_config_callback set_callback;
	clear_config_callback clear_callback;
} artik_bt_a2dp_source;

artik_error bt_a2dp_source_register(unsigned char codec,
		bool delay_reporting, const char *path,
		const unsigned char *capabilities, int cap_size);
artik_error bt_a2dp_source_unregister(void);
artik_error bt_a2dp_source_acquire(int *fd,
		unsigned short *mtu_read, unsigned short *mtu_write);
artik_error bt_a2dp_source_get_properties(
		artik_bt_a2dp_source_property * *properties);
artik_error bt_a2dp_source_release(void);
artik_error bt_a2dp_source_set_callback(select_config_callback select_func,
		set_config_callback set_func, clear_config_callback clear_func);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_A2DP_H */

