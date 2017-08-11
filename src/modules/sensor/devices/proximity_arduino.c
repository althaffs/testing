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
#include <stdint.h>
#include <string.h>

#include "artik_module.h"
#include "artik_gpio.h"
#include "artik_sensor.h"

#include <devices/proximity_arduino.h>

static artik_error proximity_request(artik_sensor_handle *,
				     artik_sensor_config*);
static artik_error proximity_release(artik_sensor_handle);
static artik_error proximity_get_presence(artik_sensor_handle, int *);

artik_sensor_proximity proximity_arduino_sensor = {
	proximity_request,
	proximity_release,
	proximity_get_presence
};

typedef struct {
	artik_list node;
	artik_gpio_module *module_gpio;
	artik_gpio_handle handle_sensor;
	int pin_gpio;
	int presence;
} sensor_proximity;

static artik_list *requested_node = NULL;

static int check_exist(sensor_proximity *elem, int val_pin)
{
	if (elem->pin_gpio == val_pin)
		return 1;
	return 0;
}

static artik_error proximity_request(artik_sensor_handle *handle,
						artik_sensor_config *config)
{
	sensor_proximity *elem = (sensor_proximity *)artik_list_get_by_check(
		requested_node, (ARTIK_LIST_FUNCB)&check_exist,
		(void *)(intptr_t)((artik_gpio_config *)config->config)->id);
	artik_error res = S_OK;

	if (elem)
		return E_BUSY;
	elem = (sensor_proximity *)artik_list_add(&requested_node, 0,
		sizeof(sensor_proximity));
	if (elem) {
		elem->node.handle = (ARTIK_LIST_HANDLE)elem;
		elem->module_gpio = (artik_gpio_module *)
					artik_request_api_module("gpio");
		if (elem->module_gpio) {
			res = elem->module_gpio->request(&elem->handle_sensor,
				(artik_gpio_config *)config->config);
			elem->pin_gpio = ((artik_gpio_config *)
					config->config)->id;
			*handle = (artik_sensor_handle)elem->node.handle;
			if (res != S_OK)
				proximity_release(elem);
			return res;
		}
		proximity_release(elem);
		return E_INVALID_VALUE;
	}
	return E_NO_MEM;
}

static artik_error proximity_release(artik_sensor_handle handle)
{
	sensor_proximity *data_user = (sensor_proximity *)
			artik_list_get_by_handle(requested_node,
			(ARTIK_LIST_HANDLE)handle);

	if (data_user) {
		if (data_user->module_gpio) {
			data_user->module_gpio->release(
						data_user->handle_sensor);
			artik_release_api_module(data_user->module_gpio);
		}
	  artik_list_delete_node(&requested_node, (artik_list *)data_user);
	}

	return S_OK;
}

static artik_error proximity_get_presence(artik_sensor_handle handle,
								int *store)
{
	sensor_proximity *data_user = (sensor_proximity *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE)handle);
	int res = 0;

	if (!store)
		return E_BAD_ARGS;
	if (data_user) {
		res = data_user->module_gpio->read((artik_gpio_handle)
						data_user->handle_sensor);
		data_user->presence = (res == 0 ? 1 : 0);
		*store = data_user->presence;
		return S_OK;
	}
	*store = -1;
	return E_INVALID_VALUE;
}
