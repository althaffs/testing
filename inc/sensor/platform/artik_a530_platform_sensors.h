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

#ifndef ARTIK_A530_PLATFORM_SENSORS_H_
#define ARTIK_A530_PLATFORM_SENSORS_H_

/*! \file artik_a530_platform_sensors.h
 *
 *  \brief Hardware specific definitions for the A530 platform sensors
 *
 */

/* Include use module headers needed for the platform sensor configuration*/
#include <platform/artik_a530_platform.h>
#include <artik_log.h>
#include <artik_loop.h>
#include <artik_i2c.h>
#include <artik_gpio.h>
#include <artik_serial.h>
#include <artik_pwm.h>
#include <artik_adc.h>
#include <artik_spi.h>
#include <artik_sensor.h>

#include <devices/HTS221.h>
#include <devices/K6DS3.h>
#include <devices/CM3323E.h>
#include <devices/S5712CCDL1_I4T1U.h>
#include <devices/LPS25HBTR.h>

/* SENSOR BOARD */

static const artik_i2c_config hts221_temp_a530_config = {
	1, 1000, I2C_8BIT, HTS221_ADDR
};
static const artik_i2c_config hts221_humidity_a530_config = {
	1, 1000, I2C_8BIT, HTS221_ADDR
};
static const artik_i2c_config cc3323e_a530_config = {
	1, 1000, I2C_8BIT, CM3323E_ADDR
};
static const artik_i2c_config lps25hbtr_barometer_a530_config = {
	1, 1000, I2C_8BIT, LPS25HBTR_ADDR
};
static const artik_i2c_config lps25hbtr_temp_a530_config = {
	1, 1000, I2C_8BIT, LPS25HBTR_ADDR
};
static const artik_gpio_config s5712ccdl1_a530_config = {
	ARTIK_A530_GPIO2,  (char *)"gpio", GPIO_IN, GPIO_EDGE_BOTH, 0, NULL
};
static const artik_spi_config k6ds3_xl_a530_config = {
	2, 0, SPI_MODE0, 8, 1000000
};
static const artik_spi_config k6ds3_gyro_a530_config = {
	2, 0, SPI_MODE0, 8, 1000000
};

/* Based upon Artik Sensor Board - Rev.1.0 */
static artik_sensor_config artik_api_a530_sensors[] = {

	{
		ARTIK_SENSOR_TEMPERATURE,
		(char *)"hts221_temp",
		(void *)&hts221_temp_a530_config,
		&hts221_temp_sensor
	},
	{
		ARTIK_SENSOR_BAROMETER,
		(char *)"barometer",
		(void *)&lps25hbtr_barometer_a530_config,
		&lps25hbtr_barometer_sensor
	},
	{
		ARTIK_SENSOR_HUMIDITY,
		(char *)"hts221_humidity",
		(void *)&hts221_humidity_a530_config,
		&hts221_humidity_sensor
	},
	{
		ARTIK_SENSOR_LIGHT,
		(char *)"light",
		(void *)&cc3323e_a530_config,
		&cm3323e_sensor
	},
	{
		ARTIK_SENSOR_HALL,
		(char *)"hall",
		(void *)&s5712ccdl1_a530_config,
		&s5712ccdl1_sensor
	},
	{
		ARTIK_SENSOR_ACCELEROMETER,
		(char *)"k6ds3_xl",
		(void *)&k6ds3_xl_a530_config,
		&k6ds3_xl_sensor
	},
	{
		ARTIK_SENSOR_GYRO,
		(char *)"gyro",
		(void *)&k6ds3_gyro_a530_config,
		&k6ds3_gyro_sensor
	},
	{
		ARTIK_SENSOR_NONE,
		NULL,
		NULL,
		NULL
	}
};
#endif /* ARTIK_A530_PLATFORM_SENSORS_H_ */
