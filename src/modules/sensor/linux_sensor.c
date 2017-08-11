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

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "artik_module.h"
#include "artik_platform_sensors.h"
#include "os_sensor.h"

#define TAB_SIZE(x) (sizeof(x)/sizeof(*x))

artik_error os_sensor_request(artik_sensor_config *config,
					artik_sensor_handle *handle,
					artik_sensor_ops *sensor)
{
	if (!config)
		return E_BAD_ARGS;
	if (!config->data_user)
		return E_NOT_SUPPORTED;
	*sensor = (artik_sensor_ops) (config->data_user);
	switch (config->type) {
	case ARTIK_SENSOR_ACCELEROMETER:
		return ((artik_sensor_accelerometer *) *
			sensor)->request(handle, config);
	case ARTIK_SENSOR_HUMIDITY:
		return ((artik_sensor_humidity *) *sensor)->request(handle,
								     config);
	case ARTIK_SENSOR_LIGHT:
		return ((artik_sensor_light *) *sensor)->request(handle,
								  config);
	case ARTIK_SENSOR_TEMPERATURE:
		return ((artik_sensor_temperature *) *sensor)->request(handle,
									config);
	case ARTIK_SENSOR_PROXIMITY:
		return ((artik_sensor_proximity *) *sensor)->request(handle,
								      config);
	case ARTIK_SENSOR_FLAME:
		return ((artik_sensor_flame *) *sensor)->request(handle,
								  config);
	case ARTIK_SENSOR_BAROMETER:
		return ((artik_sensor_pressure *) *sensor)->request(handle,
								  config);
	case ARTIK_SENSOR_GYRO:
		return ((artik_sensor_gyro *) *sensor)->request(handle,
								  config);
	case ARTIK_SENSOR_HALL:
		return ((artik_sensor_hall *) *sensor)->request(handle,
								  config);
	case ARTIK_SENSOR_NONE:
		return E_BAD_ARGS;
	}
	return E_BAD_ARGS;
}

artik_sensor_config *os_sensor_get(unsigned int nb, artik_sensor_device_t type)
{
	int pos = 0, ref = 0;

	while ((unsigned int)ref <= nb &&
			artik_api_sensors[artik_get_platform()][pos].type) {
		if ((unsigned int)ref < nb
		    && artik_api_sensors[artik_get_platform()][pos].type ==
		    type)
			ref++;
		else if ((unsigned int)ref == nb
		    && artik_api_sensors[artik_get_platform()][pos].type ==
		    type)
			return &artik_api_sensors[artik_get_platform()][pos];
		pos++;
	}
	return NULL;
}
