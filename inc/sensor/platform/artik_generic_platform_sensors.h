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

#ifndef ARTIK_GENERIC_PLATFORM_SENSORS_H_
#define ARTIK_GENERIC_PLATFORM_SENSORS_H_

/*! \file artik_generic_platform_sensors.h
 *
 *  \brief Hardware specific definitions for the Generic platform sensors
 *
 */

/* Include use module headers needed for the platform sensor configuration*/
#include <artik_log.h>
#include <artik_loop.h>
#include <artik_i2c.h>
#include <artik_gpio.h>
#include <artik_serial.h>
#include <artik_pwm.h>
#include <artik_adc.h>
#include <artik_spi.h>
#include <artik_sensor.h>

/* No sensors available on the generic platform */
static artik_sensor_config artik_api_generic_sensors[] = {
		{ ARTIK_SENSOR_NONE, NULL, NULL, NULL }
};

#endif /* ARTIK_GENERIC_PLATFORM_SENSORS_H_ */
