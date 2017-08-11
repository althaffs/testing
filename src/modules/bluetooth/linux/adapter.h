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

#ifndef __ARTIK_BT_ADAPTER_H
#define __ARTIK_BT_ADAPTER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

artik_error bt_set_scan_filter(artik_bt_scan_filter *filter);
artik_error bt_set_alias(const char *alias);
artik_error bt_set_discoverable(bool discoverable);
artik_error bt_set_pairable(bool pairable);
artik_error bt_set_pairableTimeout(unsigned int timeout);
artik_error bt_set_discoverableTimeout(unsigned int timeout);
artik_error bt_start_scan(void);
artik_error bt_stop_scan(void);
bool bt_is_scanning(void);
artik_error bt_get_device_property(const char *addr, const char *property,
	char **value);
artik_error bt_get_adapter_info(artik_bt_adapter *adapter);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_ADAPTER_H */
