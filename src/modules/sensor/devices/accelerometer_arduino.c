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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "artik_module.h"
#include "artik_i2c.h"
#include "artik_sensor.h"

#include <devices/accelerometer_arduino.h>

static artik_error accelerometer_request(artik_sensor_handle *,
							artik_sensor_config*);
static artik_error accelerometer_release(artik_sensor_handle);
static artik_error accelerometer_get_speed_x(artik_sensor_handle, int *);
static artik_error accelerometer_get_speed_y(artik_sensor_handle, int *);
static artik_error accelerometer_get_speed_z(artik_sensor_handle, int *);

artik_sensor_accelerometer accelerometer_arduino_sensor = {
	accelerometer_request,
	accelerometer_release,
	accelerometer_get_speed_x,
	accelerometer_get_speed_y,
	accelerometer_get_speed_z
};

typedef struct {
	artik_list node;
	artik_i2c_module *module_i2c;
	artik_i2c_handle handle_sensor;
	int id_i2c;
	int speed_x;
	int speed_y;
	int speed_z;
} sensor_accelerometer;

static artik_list *requested_node = NULL;

static int check_exist(sensor_accelerometer *elem, int val_id)
{
	if (elem->id_i2c == val_id)
		return 1;
	return 0;
}

static artik_error accelerometer_request(artik_sensor_handle *handle,
						artik_sensor_config *config)
{
	sensor_accelerometer *elem = (sensor_accelerometer *)
				artik_list_get_by_check(requested_node,
					(ARTIK_LIST_FUNCB)&check_exist,
					(void *)(intptr_t)((artik_i2c_config *)
						config->config)->id);
	artik_error res = S_OK;
	char buffer = 0;

	if (elem)
		return E_BUSY;
	elem = (sensor_accelerometer *) artik_list_add(&requested_node, 0,
						sizeof(sensor_accelerometer));
	if (elem) {
		elem->node.handle = (ARTIK_LIST_HANDLE) elem;
		elem->module_i2c = (artik_i2c_module *)
						artik_request_api_module("i2c");
		if (elem->module_i2c) {
			res = elem->module_i2c->request(&elem->handle_sensor,
					(artik_i2c_config *) config->config);
			*handle = (artik_sensor_handle) elem->node.handle;
			if (res == S_OK) {
				elem->id_i2c = ((artik_i2c_config *)
							config->config)->id;
				res = elem->module_i2c->read_register(
					(artik_i2c_handle) elem->handle_sensor,
							0x0F, &buffer, 1);
				if (res == S_OK && buffer == 0x32) {
					buffer = 0x27;
					res = elem->module_i2c->write_register((
						artik_i2c_handle)
						elem->handle_sensor, 0x20,
								&buffer, 1);
					return res;
				}
			}
			accelerometer_release(elem);
			return res;
		}
		accelerometer_release(elem);
		return E_INVALID_VALUE;
	}
	return E_NO_MEM;
}

static artik_error accelerometer_release(artik_sensor_handle handle)
{
	sensor_accelerometer *data_user = (sensor_accelerometer *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE)handle);

	if (data_user) {
		if (data_user->module_i2c) {
			unsigned char buffer = 0x00;

			data_user->module_i2c->write_register((artik_i2c_handle)
				data_user->handle_sensor, 0x20, (char *)&buffer,
									1);
			data_user->module_i2c->release(
						data_user->handle_sensor);
			artik_release_api_module(data_user->module_i2c);
		}
		artik_list_delete_node(&requested_node, (artik_list *)
								data_user);
	}
	return S_OK;
}

static artik_error accelerometer_get_speed_x(artik_sensor_handle handle,
								int *store)
{
	sensor_accelerometer *data_user = (sensor_accelerometer *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE)handle);
	int res = 0;
	short buffer = 0;

	if (!store)
		return E_BAD_ARGS;
	if (data_user) {
		res = data_user->module_i2c->read_register((artik_i2c_handle)
			data_user->handle_sensor, 0x28, (char *)&buffer, 1);
		res += data_user->module_i2c->read_register((artik_i2c_handle)
			data_user->handle_sensor, 0x29, (char *)&buffer + 1, 1);
		data_user->speed_x = (res == S_OK ? (int)buffer : 0);
		*store = data_user->speed_x;
		return S_OK;
	}
	*store = -1;
	return E_INVALID_VALUE;
}

static artik_error accelerometer_get_speed_y(artik_sensor_handle handle,
								int *store)
{
	sensor_accelerometer *data_user = (sensor_accelerometer *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE)handle);
	int res = 0;
	short buffer = 0;

	if (!store)
		return E_BAD_ARGS;
	if (data_user) {
		res = data_user->module_i2c->read_register((artik_i2c_handle)
			data_user->handle_sensor, 0x2A, (char *)&buffer, 1);
		res += data_user->module_i2c->read_register((artik_i2c_handle)
			data_user->handle_sensor, 0x2B, (char *)&buffer + 1, 1);
		data_user->speed_y = (res == S_OK ? (int)buffer : 0);
		*store = data_user->speed_y;
		return S_OK;
	}
	*store = -1;
	return E_INVALID_VALUE;
}

static artik_error accelerometer_get_speed_z(artik_sensor_handle handle,
								int *store)
{
	sensor_accelerometer *data_user = (sensor_accelerometer *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE)handle);
	int res = 0;
	short buffer = 0;

	if (!store)
		return E_BAD_ARGS;
	if (data_user) {
		res = data_user->module_i2c->read_register((artik_i2c_handle)
			data_user->handle_sensor, 0x2C, (char *)&buffer, 1);
		res += data_user->module_i2c->read_register((artik_i2c_handle)
			data_user->handle_sensor, 0x2D, (char *)&buffer + 1, 1);
		data_user->speed_z = (res == S_OK ? (int)buffer : 0);
		*store = data_user->speed_z;
		return S_OK;
	}
	*store = -1;
	return E_INVALID_VALUE;
}
