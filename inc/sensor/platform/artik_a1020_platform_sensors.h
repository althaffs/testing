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

#ifndef ARTIK_A1020_PLATFORM_SENSORS_H_
#define ARTIK_A1020_PLATFORM_SENSORS_H_

/*! \file artik_a1020_platform_sensors.h
 *
 *  \brief Hardware specific definitions for the Artik 1020 platform sensors
 *
 */

/* Include use module headers needed for the platform sensor configuration*/
#include <platform/artik_a1020_platform.h>
#include <artik_log.h>
#include <artik_loop.h>
#include <artik_i2c.h>
#include <artik_gpio.h>
#include <artik_serial.h>
#include <artik_pwm.h>
#include <artik_adc.h>
#include <artik_spi.h>
#include <artik_sensor.h>

static artik_i2c_config accelerometer_a1020_config = {
		7, 1000, I2C_8BIT, 0x0018
	};
static artik_adc_config humidity_a1020_config = {
		ARTIK_A1020_A1, (char *)"adc",
		NULL
	};
static artik_adc_config light_a1020_config = {
		ARTIK_A1020_A5, (char *)"adc",
		NULL
	};
static artik_adc_config temperature_a1020_config = {
		ARTIK_A1020_A2, (char *)"adc",
		NULL
	};
static artik_gpio_config proximity_a1020_config = {
		ARTIK_A1020_GPIO_XEINT0,
		(char *)"gpio",
		GPIO_IN, GPIO_EDGE_BOTH, 0, NULL
	};
static artik_gpio_config infred_flame_a1020_config = {
		ARTIK_A1020_GPIO_XEINT1,
		(char *)"gpio",
		GPIO_IN, GPIO_EDGE_BOTH, 0, NULL
	};
static artik_sensor_config artik_api_a1020_sensors[] = {
	{
		ARTIK_SENSOR_ACCELEROMETER,
		(char *)"accelerometer",
		(void *)&accelerometer_a1020_config,
		NULL
	},
	{
		ARTIK_SENSOR_HUMIDITY,
		(char *)"humidity",
		(void *)&humidity_a1020_config,
		NULL
	},
	{
		ARTIK_SENSOR_LIGHT,
		(char *)"photoresistor",
		(void *)&light_a1020_config,
		NULL
	},
	{
		ARTIK_SENSOR_TEMPERATURE,
		(char *)"env_temperature",
		(void *)&temperature_a1020_config,
		NULL
	},
	{
		ARTIK_SENSOR_PROXIMITY,
		(char *)"proximity",
		(void *)&proximity_a1020_config,
		NULL
	},
	{
		ARTIK_SENSOR_FLAME,
		(char *)"infrared_flame",
		(void *)&infred_flame_a1020_config,
		NULL
	},
	{ ARTIK_SENSOR_NONE, NULL, NULL, NULL }
};

#endif /* ARTIK_A1020_PLATFORM_SENSORS_H_ */
