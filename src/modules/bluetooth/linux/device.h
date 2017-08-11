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

#ifndef __ARTIK_BT_DEVICE_H
#define __ARTIK_BT_DEVICE_H

#include <artik_bluetooth.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_ADDRESS_LENGTH		18
#define BT_OBJECT_PATH_LENGH	38

artik_error bt_start_bond(const char *remote_address);
artik_error bt_stop_bond(const char *remote_address);
artik_error bt_remove_device(const char *remote_address);
artik_error bt_remove_devices(void);
artik_error bt_remove_unpaired_devices(void);
artik_error bt_connect(const char *remote_address);
artik_error bt_connect_profile(const char *remote_address, const char *uuid);
artik_error bt_disconnect(const char *remote_address);
artik_error bt_set_trust(const char *remote_address);
artik_error bt_unset_trust(const char *remote_address);
artik_error bt_set_block(const char *remote_address);
artik_error bt_unset_block(const char *remote_address);
artik_error bt_get_devices(artik_bt_device **device_list, int *count);
artik_error bt_get_paired_devices(artik_bt_device **device_list, int *count);
artik_error bt_get_connected_devices(artik_bt_device **device_list, int *count);
artik_error bt_free_device(artik_bt_device *device);
artik_error bt_free_devices(artik_bt_device *device_list, int count);
bool bt_is_paired(const char *remote_address);
bool bt_is_connected(const char *remote_address);
bool bt_is_trusted(const char *remote_address);
bool bt_is_blocked(const char *remote_address);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_DEVICE_H */
