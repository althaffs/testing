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

#ifndef INCLUDE_ARTIK_GENERIC_PLATFORM_H_
#define INCLUDE_ARTIK_GENERIC_PLATFORM_H_

/*! \file artik_generic_platform.h
 *
 *  \brief Hardware specific definitions for the Generic platform
 *
 */

/* Include all module headers needed for the platform */
/* List of modules available for the platform */
static const artik_api_module artik_api_generic_modules[] = {
	{ARTIK_MODULE_LOG,	 (char *)"log",	      (char *)"base"},
	{ARTIK_MODULE_LOOP,	 (char *)"loop",      (char *)"base"},
	{ARTIK_MODULE_GPIO,	 (char *)"gpio",      (char *)"systemio"},
	{ARTIK_MODULE_I2C,	 (char *)"i2c",	      (char *)"systemio"},
	{ARTIK_MODULE_SERIAL,	 (char *)"serial",    (char *)"systemio"},
	{ARTIK_MODULE_PWM,	 (char *)"pwm",	      (char *)"systemio"},
	{ARTIK_MODULE_ADC,	 (char *)"adc",	      (char *)"systemio"},
	{ARTIK_MODULE_HTTP,	 (char *)"http",      (char *)"connectivity"},
	{ARTIK_MODULE_CLOUD,	 (char *)"cloud",     (char *)"connectivity"},
	{ARTIK_MODULE_WIFI,	 (char *)"wifi",      (char *)"wifi"},
	{ARTIK_MODULE_MEDIA,	 (char *)"media",     (char *)"media"},
	{ARTIK_MODULE_TIME,	 (char *)"time",      (char *)"base"},
	{ARTIK_MODULE_SECURITY,	 (char *)"security",  (char *)"connectivity"},
	{ARTIK_MODULE_SPI,	 (char *)"spi",	      (char *)"systemio"},
	{ARTIK_MODULE_BLUETOOTH, (char *)"bluetooth", (char *)"bluetooth"},
	{ARTIK_MODULE_SENSOR,	 (char *)"sensor",    (char *)"sensor"},
	{ARTIK_MODULE_ZIGBEE,	 (char *)"zigbee",    (char *)"zigbee"},
	{ARTIK_MODULE_NETWORK,	 (char *)"network",   (char *)"connectivity"},
	{ARTIK_MODULE_WEBSOCKET, (char *)"websocket", (char *)"connectivity"},
	{ARTIK_MODULE_LWM2M,	 (char *)"lwm2m",     (char *)"lwm2m"},
	{ARTIK_MODULE_MQTT,	 (char *)"mqtt",      (char *)"mqtt"},
	{(artik_module_id_t)-1,	 NULL,		      NULL},
};

#endif /* INCLUDE_ARTIK_GENERIC_PLATFORM_H_ */
