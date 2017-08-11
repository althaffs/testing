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

#ifndef ARTIK_A520_PLATFORM_SENSORS_H_
#define ARTIK_A520_PLATFORM_SENSORS_H_

/*! \file artik_a520_platform_sensors.h
 *
 *  \brief Hardware specific definitions for the A5 platform sensors
 *
 */

/* Include use module headers needed for the platform sensor configuration*/
#include <platform/artik_a520_platform.h>
#include <artik_log.h>
#include <artik_loop.h>
#include <artik_i2c.h>
#include <artik_gpio.h>
#include <artik_serial.h>
#include <artik_pwm.h>
#include <artik_adc.h>
#include <artik_spi.h>
#include <artik_sensor.h>

#include <devices/accelerometer_arduino.h>
#include <devices/humidity_arduino.h>
#include <devices/light_arduino.h>
#include <devices/temperature_arduino.h>
#include <devices/infrared_flame_arduino.h>
#include <devices/proximity_arduino.h>

static artik_adc_config humidity_a520_config = {
	ARTIK_A520_A1, (char *)"adc", NULL
};
static artik_i2c_config accelerometer_a520_config = {
	3, 1000, I2C_8BIT, ACCELEROMETER_ARD_ADDR
};
static artik_adc_config light_a520_config = {
	ARTIK_A520_A0, (char *)"adc", NULL
};
static artik_adc_config temperature_a520_config = {
	ARTIK_A520_A0, (char *)"adc", NULL
};
static artik_gpio_config proximity_a520_config = {
	ARTIK_A520_GPIO_XEINT0, (char *)"gpio", GPIO_IN, GPIO_EDGE_BOTH, 0, NULL
};
static artik_gpio_config infred_flame_a520_config = {
	ARTIK_A520_GPIO_XEINT1, (char *)"gpio", GPIO_IN, GPIO_EDGE_BOTH, 0, NULL
};
static artik_sensor_config artik_api_a520_sensors[] = {
	{
		ARTIK_SENSOR_ACCELEROMETER, (char *)"accelerometer",
		(void *)&accelerometer_a520_config,
		&accelerometer_arduino_sensor
	},
	{
		ARTIK_SENSOR_HUMIDITY, (char *)"humidity",
		(void *)&humidity_a520_config,
		&humidity_arduino_sensor
	},
	{
		ARTIK_SENSOR_LIGHT, (char *)"photoresistor",
		(void *)&light_a520_config,
		&light_arduino_sensor
	},
	{
		ARTIK_SENSOR_TEMPERATURE, (char *)"env_temperature",
		(void *)&temperature_a520_config,
		&temperature_arduino_sensor
	},
	{
		ARTIK_SENSOR_PROXIMITY, (char *)"proximity",
		(void *)&proximity_a520_config,
		&proximity_arduino_sensor
	},
	{
		ARTIK_SENSOR_FLAME, (char *)"infrared_flame",
		(void *)&infred_flame_a520_config,
		&infrared_flame_arduino_sensor
	},
	{ ARTIK_SENSOR_NONE, NULL, NULL, NULL }
};

#endif /* ARTIK_A520_PLATFORM_SENSORS_H_ */
