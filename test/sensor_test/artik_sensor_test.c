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

/*! \file artik_sensor_test.c
 *
 *  \brief Sensor Test example in C
 *
 *  Instance of usage Sensor module with
 *  a program developed in C.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <artik_module.h>
#include <artik_sensor.h>

static int end = 0;

static void signal_handler(int signum)
{
	end = 1;
}

int main(void)
{
	artik_sensor_module *sensor              =
		(artik_sensor_module *)artik_request_api_module("sensor");
	artik_sensor_config *config_acce         =
		sensor->get_accelerometer_sensor(0);
	artik_sensor_config *config_humid        =
		sensor->get_humidity_sensor(0);
	artik_sensor_config *config_photolight   =
		sensor->get_light_sensor(0);
	artik_sensor_config *config_envtemp      =
		sensor->get_temperature_sensor(0);
	artik_sensor_config *config_pressure     =
		sensor->get_pressure_sensor(0);
	artik_sensor_config *config_gyro         = sensor->get_gyro_sensor(0);
	artik_sensor_config *config_hall         = sensor->get_hall_sensor(0);
	artik_sensor_handle handle_acce          = NULL;
	artik_sensor_handle handle_humid         = NULL;
	artik_sensor_handle handle_photolight    = NULL;
	artik_sensor_handle handle_envtemp       = NULL;
	artik_sensor_handle handle_pressure      = NULL;
	artik_sensor_handle handle_gyro          = NULL;
	artik_sensor_handle handle_hall          = NULL;
	artik_sensor_accelerometer *sensor_acce  = NULL;
	artik_sensor_humidity *sensor_humid      = NULL;
	artik_sensor_light *sensor_photolight    = NULL;
	artik_sensor_temperature *sensor_envtemp = NULL;
	artik_sensor_pressure *sensor_pressure   = NULL;
	artik_sensor_gyro *sensor_gyro           = NULL;
	artik_sensor_hall *sensor_hall           = NULL;
	artik_sensor_config *all_conf            = sensor->list();

	int k = 5;
	int i   = 0;
	int res = 0;

	signal(SIGINT, signal_handler);
	while (all_conf && all_conf[i].type) {
		printf("CONF[%s] (%d)\n", all_conf[i].name, all_conf[i].type);
		i++;
	}
	printf("ret : %s\n", error_msg(sensor->request(config_acce,
			&handle_acce, (artik_sensor_ops *) &sensor_acce)));
	printf("ret : %s\n", error_msg(sensor->request(config_humid,
			&handle_humid, (artik_sensor_ops *) &sensor_humid)));
	printf("ret : %s\n", error_msg(sensor->request(config_photolight,
			&handle_photolight, (artik_sensor_ops *)
			&sensor_photolight)));
	printf("ret : %s\n", error_msg(sensor->request(config_envtemp,
			&handle_envtemp, (artik_sensor_ops *)
			&sensor_envtemp)));
	printf("ret : %s\n", error_msg(sensor->request(config_pressure,
			&handle_pressure, (artik_sensor_ops *)
			&sensor_pressure)));
	printf("ret : %s\n", error_msg(sensor->request(config_gyro,
			&handle_gyro, (artik_sensor_ops *) &sensor_gyro)));
	printf("ret : %s\n", error_msg(sensor->request(config_hall,
			&handle_hall, (artik_sensor_ops *) &sensor_hall)));
	while (!end && k) {
		printf("===================================================\n");

		if (sensor_envtemp && handle_envtemp) {
			sensor_envtemp->get_celsius(handle_envtemp, &res);
			printf("Temperature data  celsius: %d°C\n", res);
			sensor_envtemp->get_fahrenheit(handle_envtemp, &res);
			printf("Temperature data  fahrenheit: %d°F\n", res);
			res = 0;
		}
		if (sensor_acce && handle_acce) {
			sensor_acce->get_speed_x(handle_acce, &res);
			printf("Accelerometer data  speed_x: %d\n", res);
			res = 0;
			sensor_acce->get_speed_y(handle_acce, &res);
			printf("Accelerometer data  speed_y: %d\n", res);
			res = 0;
			sensor_acce->get_speed_z(handle_acce, &res);
			printf("Accelerometer data  speed_z: %d\n", res);
			res = 0;
		}
		if (sensor_gyro && handle_gyro) {
			sensor_gyro->get_yaw(handle_gyro, &res);
			printf("Gyrometer data  Yaw: %d\n", res);
			res = 0;
			sensor_gyro->get_pitch(handle_gyro, &res);
			printf("Gyrometer data  Pitch: %d\n", res);
			res = 0;
			sensor_gyro->get_roll(handle_gyro, &res);
			printf("Gyrometer data  Roll: %d\n", res);
			res = 0;
		}
		if (sensor_photolight && handle_photolight) {
			sensor_photolight->get_intensity(handle_photolight,
									&res);
			printf("Light data  intensity: %d%%\n", res);
			res = 0;
		}
		if (sensor_humid && handle_humid) {
			sensor_humid->get_humidity(handle_humid, &res);
			printf("Humidity data humidity : %d%%\n", res);
			res = 0;
		}
		if (sensor_pressure && handle_pressure) {
			sensor_pressure->get_pressure(handle_pressure, &res);
			printf("Pressure data : %d\n", res);
			res = 0;
		}
		if (sensor_hall && handle_hall) {
			sensor_hall->get_detection(handle_hall, &res);
			printf("Hall Sensor Detected : %d\n", res);
			res = 0;
		}
		printf("===================================================\n");
		--k;
		sleep(2);
	}
	if (sensor_acce)
		sensor_acce->release(handle_acce);
	if (sensor_humid)
		sensor_humid->release(handle_humid);
	if (sensor_photolight)
		sensor_photolight->release(handle_photolight);
	if (sensor_envtemp)
		sensor_envtemp->release(handle_envtemp);
	if (sensor_pressure)
		sensor_pressure->release(handle_pressure);
	if (sensor_hall)
		sensor_hall->release(handle_hall);
	artik_release_api_module(sensor);

	return 0;
}
