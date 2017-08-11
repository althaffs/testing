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
#include "artik_i2c.h"
#include "artik_sensor.h"
#include <devices/LPS25HBTR.h>

#define LPS25HBTR_DEVICE_ID		0xBD

#define LPS25HBTR_REG_WHO_AM_I		0x0F
#define LPS25HBTR_REG_CTRL_REG1		0x20
#define LPS25HBTR_REG_PRESS_OUT_XL	0x28
#define LPS25HBTR_REG_PRESS_OUT_L	0x29
#define LPS25HBTR_REG_PRESS_OUT_H	0x2A
#define LPS25HBTR_REG_TEMP_OUT_L	0x2B
#define LPS25HBTR_REG_TEMP_OUT_H	0x2C

#define AUTO_INC			0x80

struct lps25hbtr_handle_s {
	artik_list node;
	artik_i2c_module *i2c;
	artik_i2c_handle hdl;
	int id;
};

static artik_error request(artik_sensor_handle *, artik_sensor_config *);
static artik_error release(artik_sensor_handle);
static artik_error get_pressure(artik_sensor_handle, int *);

static artik_error get_celsius(artik_sensor_handle handle, int *store);
static artik_error get_fahrenheit(artik_sensor_handle handle, int *store);

artik_sensor_pressure lps25hbtr_barometer_sensor = { request,
		release, get_pressure };

artik_sensor_temperature lps25hbtr_temperature_sensor = { request, release,
		get_celsius, get_fahrenheit };

static artik_list *lps25hbtr_list = NULL;

static int check_exist(struct lps25hbtr_handle_s *elem, int id)
{
	if (elem->id == id)
		return 1;
	return 0;
}

static artik_error initialize(artik_i2c_module *i2c, artik_i2c_handle handle)
{
	artik_error ret;
	char buffer;

	ret = i2c->read_register(handle, LPS25HBTR_REG_WHO_AM_I, &buffer, 1);
	if (ret != S_OK)
		return ret;

	if (buffer != LPS25HBTR_DEVICE_ID)
		return E_NOT_SUPPORTED;

	/*
	 * PD: active power down mode
	 * ODR: 011, 12.5kHz
	 * !DIFF_EN: disable interrupt circuit
	 * !BDU: continuous update
	 * !RESET_AZ: disable
	 * !SIM: spi 4-wire interface (don't used)
	 */

	buffer = 0xB0;
	ret = i2c->write_register(handle, LPS25HBTR_REG_CTRL_REG1, &buffer, 1);
	if (ret != S_OK)
		return ret;

	return S_OK;
}

static artik_error request(artik_sensor_handle *handle,
						artik_sensor_config *config)
{
	artik_i2c_module *i2c;
	struct lps25hbtr_handle_s *elem;

	elem = (struct lps25hbtr_handle_s *) artik_list_get_by_check(
			lps25hbtr_list, (ARTIK_LIST_FUNCB) check_exist,
			(void *)(intptr_t) ((artik_i2c_config *) config->config)->id);
	int ret;

	if (elem)
		return E_BUSY;

	elem = (struct lps25hbtr_handle_s *) artik_list_add(&lps25hbtr_list, 0,
			sizeof(struct lps25hbtr_handle_s));

	if (elem) {
		elem->node.handle = (ARTIK_LIST_HANDLE) elem;
		i2c = (artik_i2c_module *) artik_request_api_module("i2c");

		if (!i2c) {
			release(elem);
			return E_INVALID_VALUE;
		}

		elem->i2c = i2c;

		ret = i2c->request(&elem->hdl,
				(artik_i2c_config *) config->config);

		if (ret != S_OK) {
			release(elem);
			*handle = NULL;
			return ret;
		}

		elem->id = ((artik_i2c_config *) config->config)->id;
		*handle = (artik_sensor_handle)elem->node.handle;

		/* Initalize */

		ret = initialize(i2c, elem->hdl);
		if (ret != S_OK) {
			*handle = NULL;
			release(elem);
			return ret;
		}

		return S_OK;
	}

	return E_NO_MEM;
}

static artik_error release(artik_sensor_handle handle)
{
	struct lps25hbtr_handle_s *elem;

	elem = (struct lps25hbtr_handle_s *) artik_list_get_by_handle(
			lps25hbtr_list, (ARTIK_LIST_HANDLE) handle);

	if (elem) {
		artik_list_delete_node(&lps25hbtr_list, (artik_list *) elem);
		if (elem->i2c) {
			(void)elem->i2c->release(elem->hdl);
			artik_release_api_module(elem->i2c);
		}
	}

	return S_OK;
}

static artik_error get_pressure(artik_sensor_handle handle, int *store)
{
	char buffer[3];
	struct lps25hbtr_handle_s *lps25hbtr;
	int ret;

	if (!store)
		return E_BAD_ARGS;

	*store = -1;

	lps25hbtr = (struct lps25hbtr_handle_s *) artik_list_get_by_handle(
			lps25hbtr_list, (ARTIK_LIST_HANDLE) handle);

	if (!lps25hbtr)
		return E_INVALID_VALUE;

	ret = lps25hbtr->i2c->read_register(lps25hbtr->hdl,
			LPS25HBTR_REG_PRESS_OUT_XL, &buffer[0], 1);
	if (ret < 0)
		return ret;

	ret = lps25hbtr->i2c->read_register(lps25hbtr->hdl,
			LPS25HBTR_REG_PRESS_OUT_L, &buffer[1], 1);
	if (ret < 0)
		return ret;

	ret = lps25hbtr->i2c->read_register(lps25hbtr->hdl,
			LPS25HBTR_REG_PRESS_OUT_H, &buffer[2], 1);
	if (ret < 0)
		return ret;

	*store = (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];
	*store /= 4096;

	return S_OK;
}

static artik_error get_celsius(artik_sensor_handle handle, int *store)
{
	short data;
	struct lps25hbtr_handle_s *lps25hbtr;
	int ret;

	if (!store)
		return E_BAD_ARGS;

	*store = -1;

	lps25hbtr = (struct lps25hbtr_handle_s *) artik_list_get_by_handle(
			lps25hbtr_list, (ARTIK_LIST_HANDLE) handle);

	if (!lps25hbtr)
		return E_INVALID_VALUE;

	ret = lps25hbtr->i2c->read_register(lps25hbtr->hdl,
			LPS25HBTR_REG_TEMP_OUT_L | AUTO_INC, (char *)&data, 2);
	if (ret < 0)
		return ret;

	*store = data/480 + 42.5;

	return S_OK;
}

static artik_error get_fahrenheit(artik_sensor_handle handle, int *store)
{
	short data;
	struct lps25hbtr_handle_s *lps25hbtr;
	int ret;

	if (!store)
		return E_BAD_ARGS;

	*store = -1;

	lps25hbtr = (struct lps25hbtr_handle_s *) artik_list_get_by_handle(
			lps25hbtr_list, (ARTIK_LIST_HANDLE) handle);

	if (!lps25hbtr)
		return E_INVALID_VALUE;

	ret = lps25hbtr->i2c->read_register(lps25hbtr->hdl,
			LPS25HBTR_REG_TEMP_OUT_L | AUTO_INC, (char *)&data, 2);
	if (ret < 0)
		return ret;

	data /= 480;
	data += 42.5;
	data *= 1.8;
	data += 32;

	*store = (int)data;

	return ret;
}
