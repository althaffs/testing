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
#include "os_sensor.h"

#define _SENSOR_HDL_LINK(hdl, name, ops)	\
		{ ARTIK_SENSOR_##hdl, name, &ops##_sensor }

struct _artik_sensor_tbl {
	artik_sensor_device_t id;
	void *cfg;
	artik_sensor_ops ops;
};

struct _artik_sensor_tbl tbl[] = {
#ifdef CONFIG_ARTIK_SDK_SYSTEMIO
	_SENSOR_HDL_LINK(TEMPERATURE, "hts221_temp", hts221_temp),
	_SENSOR_HDL_LINK(HUMIDITY, "humidity", hts221_humidity),
	_SENSOR_HDL_LINK(LIGHT, "light", cm3323e),
	_SENSOR_HDL_LINK(BAROMETER, "barometer", lps25hbtr_barometer),
	_SENSOR_HDL_LINK(TEMPERATURE, "lps25hbtr_temp", lps25hbtr_temperature),

	_SENSOR_HDL_LINK(PROXIMITY, "proximity", proximity),
	_SENSOR_HDL_LINK(FLAME, "infrared_flame", flame),
	_SENSOR_HDL_LINK(HALL, "hall", s5712ccdl1),

	_SENSOR_HDL_LINK(ACCELEROMETER, "accelerometer", k6ds3_xl),
	_SENSOR_HDL_LINK(GYRO, "gyro", k6ds3_gyro),
#endif
	{ARTIK_SENSOR_NONE, NULL}
};

static artik_sensor_ops get_sensor_ops(artik_sensor_config *cfg)
{
	struct _artik_sensor_tbl *sensor = tbl;

	for (; sensor->id != ARTIK_SENSOR_NONE; sensor++) {
		if (!strcmp(sensor->cfg, cfg->name) && sensor->id == cfg->type)
			return sensor->ops;
	}

	return (artik_sensor_ops) NULL;
}

artik_error os_sensor_request(artik_sensor_config *config,
		artik_sensor_handle *handle, artik_sensor_ops *sensor)
{
	artik_sensor_ops ops = NULL;
	artik_error err;

	if (!config)
		return E_BAD_ARGS;

	ops = get_sensor_ops(config);
	if (ops == NULL)
		return E_BAD_ARGS;

	switch (config->type) {
	case ARTIK_SENSOR_ACCELEROMETER:
		err = ((artik_sensor_accelerometer *)ops)->request(handle,
									config);
		break;
	case ARTIK_SENSOR_HUMIDITY:
		err = ((artik_sensor_humidity *)ops)->request(handle, config);
		break;
	case ARTIK_SENSOR_LIGHT:
		err = ((artik_sensor_light *)ops)->request(handle, config);
		break;
	case ARTIK_SENSOR_TEMPERATURE:
		err = ((artik_sensor_temperature *)ops)->request(handle,
									config);
		break;
	case ARTIK_SENSOR_PROXIMITY:
		err = ((artik_sensor_proximity *)ops)->request(handle, config);
		break;
	case ARTIK_SENSOR_FLAME:
		err = ((artik_sensor_flame *)ops)->request(handle, config);
		break;
	case ARTIK_SENSOR_HALL:
		err = ((struct artik_sensor_hall_s *)ops)->request(handle,
									config);
		break;
	case ARTIK_SENSOR_BAROMETER:
		err = ((struct artik_sensor_barometer_s *)ops)->request(handle,
									config);
		break;
	case ARTIK_SENSOR_GYRO:
		err = ((struct artik_sensor_gyro_s *)ops)->request(handle,
									config);
		break;
	default:
		return E_BAD_ARGS;
	}

	*sensor = ops;

	return err;
}

artik_sensor_config *os_sensor_get(unsigned int nb, artik_sensor_device_t type)
{
	artik_sensor_config *cfg =
			(artik_sensor_config *)
					artik_api_sensors[artik_get_platform()];
	int ref = 0;

	for (; cfg->type != ARTIK_SENSOR_NONE; cfg++) {
		if (cfg->type == type) {
			if (ref < nb) {
				ref++;
				continue;
			}
			return cfg;
		}
	}

	return NULL;
}
