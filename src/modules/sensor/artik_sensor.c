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

#include "artik_module.h"
#include "artik_platform_sensors.h"
#include "artik_sensor.h"
#include "os_sensor.h"

#define TAB_SIZE(x) (sizeof(x)/sizeof(*x))

static artik_error artik_sensor_request(artik_sensor_config *,
					artik_sensor_handle *,
					artik_sensor_ops *);
static artik_sensor_config *artik_sensor_list(void);
static artik_sensor_config *artik_sensor_get_sensor(unsigned int,
						    artik_sensor_device_t);
static artik_sensor_config *artik_sensor_get_accelerometer_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_humidity_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_light_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_temperature_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_proximity_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_flame_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_pressure_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_gyro_sensor(unsigned int);
static artik_sensor_config *artik_sensor_get_hall_sensor(unsigned int);

artik_sensor_module sensor_module = {
	artik_sensor_request,
	artik_sensor_list,
	artik_sensor_get_sensor,
	artik_sensor_get_accelerometer_sensor,
	artik_sensor_get_humidity_sensor,
	artik_sensor_get_light_sensor,
	artik_sensor_get_temperature_sensor,
	artik_sensor_get_proximity_sensor,
	artik_sensor_get_flame_sensor,
	artik_sensor_get_pressure_sensor,
	artik_sensor_get_gyro_sensor,
	artik_sensor_get_hall_sensor
};

static artik_error artik_sensor_request(artik_sensor_config *config,
					artik_sensor_handle *handle,
					artik_sensor_ops *sensor)
{
	return os_sensor_request(config, handle, sensor);
}

static artik_sensor_config *artik_sensor_list(void)
{
	return artik_api_sensors[artik_get_platform()];
}

static artik_sensor_config *artik_sensor_get_sensor(unsigned int nb,
						artik_sensor_device_t type)
{
	return os_sensor_get(nb, type);
}

static artik_sensor_config *artik_sensor_get_accelerometer_sensor(unsigned int
									nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_ACCELEROMETER);
}

static artik_sensor_config *artik_sensor_get_humidity_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_HUMIDITY);
}

static artik_sensor_config *artik_sensor_get_light_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_LIGHT);
}

static artik_sensor_config *artik_sensor_get_temperature_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_TEMPERATURE);
}

static artik_sensor_config *artik_sensor_get_proximity_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_PROXIMITY);
}

static artik_sensor_config *artik_sensor_get_flame_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_FLAME);
}

static artik_sensor_config *artik_sensor_get_pressure_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_BAROMETER);
}

static artik_sensor_config *artik_sensor_get_gyro_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_GYRO);
}
static artik_sensor_config *artik_sensor_get_hall_sensor(unsigned int nb)
{
	return artik_sensor_get_sensor(nb, ARTIK_SENSOR_HALL);
}
