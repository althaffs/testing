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

#ifndef OS_SENSOR_H_
#define OS_SENSOR_H_

#include "artik_error.h"
#include "artik_sensor.h"

artik_error os_sensor_request(artik_sensor_config * config,
		artik_sensor_handle * handle, artik_sensor_ops * sensor);
artik_sensor_config *os_sensor_get(unsigned int nb,
					artik_sensor_device_t type);


#endif /* OS_SENSOR_H_ */
