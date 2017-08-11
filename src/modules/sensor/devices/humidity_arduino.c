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
#include <stdint.h>

#include "artik_module.h"
#include "artik_adc.h"
#include "artik_sensor.h"

#include <devices/humidity_arduino.h>

static artik_error humidity_request(artik_sensor_handle *,
				    artik_sensor_config*);
static artik_error humidity_release(artik_sensor_handle);
static artik_error humidity_get_humidity(artik_sensor_handle, int *);

artik_sensor_humidity humidity_arduino_sensor = {
	humidity_request,
	humidity_release,
	humidity_get_humidity
};

typedef struct {
	artik_list node;
	artik_adc_module *module_adc;
	artik_adc_handle handle_sensor;
	int pin_adc;
	int wet;
} sensor_humidity;

static artik_list *requested_node = NULL;

static int check_exist(sensor_humidity *elem, int val_pin)
{
	if (elem->pin_adc == val_pin)
		return 1;
	return 0;
}

static artik_error humidity_request(artik_sensor_handle *handle,
						artik_sensor_config *config)
{
	sensor_humidity *elem =
		(sensor_humidity *)artik_list_get_by_check(requested_node,
			(ARTIK_LIST_FUNCB)&check_exist,
			(void *)(intptr_t)((artik_adc_config *)config->config)->pin_num);
	artik_error res = S_OK;

	if (elem)
		return E_BUSY;
	elem = (sensor_humidity *)artik_list_add(&requested_node, 0,
						sizeof(sensor_humidity));
	if (elem) {
		elem->node.handle = (ARTIK_LIST_HANDLE)elem;
		elem->module_adc =
			(artik_adc_module *)artik_request_api_module("adc");
		if (elem->module_adc) {
			res = elem->module_adc->request(&elem->handle_sensor,
					(artik_adc_config *)config->config);
			*handle = (artik_sensor_handle)elem->node.handle;
			elem->pin_adc =
				((artik_adc_config *)config->config)->pin_num;
			if (res != S_OK)
				humidity_release(elem);
			return res;
		}
		humidity_release(elem);
		return E_INVALID_VALUE;
	}
	return E_NO_MEM;
}

static artik_error humidity_release(artik_sensor_handle handle)
{
	sensor_humidity *data_user =
		(sensor_humidity *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE)handle);

	if (data_user) {
		if (data_user->module_adc) {
			data_user->module_adc->release(
						data_user->handle_sensor);
			artik_release_api_module(data_user->module_adc);
		}
		artik_list_delete_node(&requested_node,
						(artik_list *)data_user);
	}

	return S_OK;
}

static artik_error humidity_get_humidity(artik_sensor_handle handle, int *store)
{
	sensor_humidity *data_user =
		(sensor_humidity *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE)handle);
	artik_error ret = S_OK;
	int res = 0;

	if (!store)
		return E_BAD_ARGS;

	if (data_user) {
		ret = data_user->module_adc->get_value((artik_adc_handle)
						data_user->handle_sensor, &res);
		if (ret != S_OK)
			return ret;

		data_user->wet = (((((float)res * 5.0f) / 1024.0f) - 0.8f) /
									0.31f);
		data_user->wet = (res > 0 ? data_user->wet : 0);
		*store = data_user->wet;

		return S_OK;
	}

	*store = -1;

	return E_INVALID_VALUE;
}
