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

#ifndef __ARTIK_BT_ASSIGNED_NUMBERS_H
#define __ARTIK_BT_ASSIGNED_NUMBERS_H

#ifdef __cplusplus
extern "C" {
#endif

#define UUID_BODY_SENSOR_LOCATION "00002a38-0000-1000-8000-00805f9b34fb"
#define UUID_HEART_RATE_MEASUREMENT "00002a37-0000-1000-8000-00805f9b34fb"

const char *_get_company_name(const int id);
const char *_get_uuid_name(const char *uuid);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_ASSIGNED_NUMBERS_H */
