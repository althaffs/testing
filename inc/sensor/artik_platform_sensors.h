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

#ifndef ARTIK_PLATFORM_SENSORS_H_
#define ARTIK_PLATFORM_SENSORS_H_

/*! \file artik_platform_sensors.h
 *
 *  \brief Platform sensors definition
 *
 *  Includes the header file corresponding to the
 *  platform the API has been built for. Supported
 *  platforms sensors are defined here
 */

#include "platform/artik_a520_platform_sensors.h"
#include "platform/artik_a710_platform_sensors.h"
#include "platform/artik_a1020_platform_sensors.h"
#include "platform/artik_a530_platform_sensors.h"
#include "platform/artik_generic_platform_sensors.h"

/*!
 *  \brief Pointers to each platform supported sensors config array
 */
static artik_sensor_config *artik_api_sensors[] = {
	artik_api_generic_sensors,
	artik_api_a520_sensors,
	artik_api_a1020_sensors,
	artik_api_a710_sensors,
	artik_api_a530_sensors
};

#endif /* ARTIK_PLATFORM_SENSORS_H_ */
