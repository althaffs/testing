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

#include <devices/S5712CCDL1_I4T1U.h>

static artik_error request(artik_sensor_handle *,
				     artik_sensor_config*);
static artik_error release(artik_sensor_handle);
static artik_error get_detection(artik_sensor_handle, int *);

artik_sensor_hall s5712ccdl1_sensor = { request, release, get_detection };

struct s5712ccdl1_handle_s {
	artik_list node;
	artik_gpio_module *module_gpio;
	artik_gpio_handle handle_sensor;
	int pin_gpio;
};

static artik_list *requested_node = NULL;

static int check_exist(struct s5712ccdl1_handle_s *elem, int val_pin)
{
	if (elem->pin_gpio == val_pin)
		return 1;

	return 0;
}

static artik_error request(artik_sensor_handle *handle, artik_sensor_config
									*config)
{
	struct s5712ccdl1_handle_s *elem;
	artik_error res = S_OK;

	elem = (struct s5712ccdl1_handle_s *) artik_list_get_by_check(
			requested_node, (ARTIK_LIST_FUNCB) check_exist,
			(void *)(intptr_t) ((artik_gpio_config *) config->config)->id);

	if (elem)
		return E_BUSY;

	elem = (struct s5712ccdl1_handle_s *) artik_list_add(&requested_node, 0,
			sizeof(struct s5712ccdl1_handle_s));

	if (elem) {
		elem->node.handle = (ARTIK_LIST_HANDLE)elem;
		elem->module_gpio =
			(artik_gpio_module *)artik_request_api_module("gpio");

		if (elem->module_gpio) {
			res = elem->module_gpio->request(&elem->handle_sensor,
					(artik_gpio_config *)config->config);
			elem->pin_gpio =
				((artik_gpio_config *)config->config)->id;
			*handle = (artik_sensor_handle)elem->node.handle;

			if (res != S_OK) {
				*handle = NULL;
				release(elem);
			}

			return res;
		}

		release(elem);
		return E_INVALID_VALUE;
	}

	return E_NO_MEM;
}

static artik_error release(artik_sensor_handle handle)
{
	struct s5712ccdl1_handle_s *data_user;

	data_user = (struct s5712ccdl1_handle_s *) artik_list_get_by_handle(
			requested_node, (ARTIK_LIST_HANDLE) handle);

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

static artik_error get_detection(artik_sensor_handle handle, int *store)
{
	struct s5712ccdl1_handle_s *data_user;

	data_user = (struct s5712ccdl1_handle_s *) artik_list_get_by_handle(
			requested_node, (ARTIK_LIST_HANDLE) handle);

	int res = 0;

	if (!store)
		return E_BAD_ARGS;

	if (data_user) {
		res = data_user->module_gpio->read(
				(artik_gpio_handle)data_user->handle_sensor);
		if (res)
			*store = 0;
		else
			*store = 1;

		return S_OK;
	}

	*store = -1;
	return E_INVALID_VALUE;
}
