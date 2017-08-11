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
#include "artik_log.h"

#include <devices/HTS221.h>

#define	_AUTO_INC		0x80
#define	HTS221_DEVICE_ID	0xBC

#define HTS221_REG_WHO_AM_I     0x0F
#define HTS221_REG_AV_CONF      0x10
#define HTS221_REG_CTRL_REG1    0x20
#define HTS221_REG_CTRL_REG2    0x21
#define HTS221_REG_H_OUT_L      0x28
#define HTS221_REG_H_OUT_H      0x29
#define HTS221_REG_T_OUT_L      0x2A
#define HTS221_REG_T_OUT_H      0x2B
#define HTS221_REG_H0_RH_X2     0x30
#define HTS221_REG_H1_RH_X2     0x31
#define HTS221_REG_T0_DEGC_X8   0x32
#define HTS221_REG_T1_DEGC_X8   0x33
#define HTS221_REG_T1_T0_MSB    0x35
#define HTS221_REG_H0_T0_OUT_L  0x36
#define HTS221_REG_H0_T0_OUT_H  0x37
#define HTS221_REG_H1_T0_OUT_L  0x3A
#define HTS221_REG_H1_T0_OUT_H  0x3B
#define HTS221_REG_T0_OUT_L     0x3C
#define HTS221_REG_T0_OUT_H     0x3D
#define HTS221_REG_T1_OUT_L     0x3E
#define HTS221_REG_T1_OUT_H     0x3F

struct hts221_config_s {
	artik_list node;
	artik_i2c_module *i2c;
	artik_i2c_handle hdl;
	int id;
	int number_of_instances;
};

static artik_error request(artik_sensor_handle *handle,
		artik_sensor_config *config);
static artik_error release(artik_sensor_handle handle);
static artik_error get_humidity(artik_sensor_handle handle, int *store);
static artik_error get_celsius(artik_sensor_handle handle, int *store);
static artik_error get_fahrenheit(artik_sensor_handle handle, int *store);

artik_sensor_humidity hts221_humidity_sensor = {
	request,
	release,
	get_humidity
};

artik_sensor_temperature hts221_temp_sensor = {
	request,
	release,
	get_celsius,
	get_fahrenheit
};

static artik_list *hts221_list = NULL;

static int check_exist(struct hts221_config_s *elem, int val_id)
{
	if (elem->id == val_id)
		return 1;
	return 0;
}

static artik_error get_data(artik_sensor_handle handle, int reg, char *data,
				int len)
{
	struct hts221_config_s *hts221;

	if (!data)
		return E_BAD_ARGS;

	hts221 = (struct hts221_config_s *) artik_list_get_by_handle(
				hts221_list, (ARTIK_LIST_HANDLE) handle);

	if (!hts221)
		return E_INVALID_VALUE;

	reg = (len > 1) ? reg | _AUTO_INC : reg;

	return hts221->i2c->read_register(hts221->hdl, reg, data, len);
}

static artik_error initialize(artik_i2c_module *i2c, artik_i2c_handle handle)
{
	artik_error ret;
	char buffer[3];

	ret = i2c->read_register(handle, HTS221_REG_WHO_AM_I, buffer, 1);
	if (ret != S_OK)
		return ret;

	if (buffer[0] != HTS221_DEVICE_ID)
		return E_NOT_SUPPORTED;

	/* 0xAVGT : 110(NOISE 0.01), AVGH : 100(NOISE 0.1)  */

	buffer[0] = 0x36;
	ret = i2c->write_register(handle, HTS221_REG_AV_CONF, buffer, 1);
	if (ret != S_OK)
		return ret;

	/*
	 * POWER ACTIVE MODE
	 * Block Update (continuously)
	 * ODRx = 01 ( OUTPUT DATA RATE: 1Hz)
	 */

	buffer[0] = (0x01 << 7) | (0x00 << 2) | (0x02);
	ret = i2c->write_register(handle, HTS221_REG_CTRL_REG1, buffer, 1);
	if (ret != S_OK)
		return ret;

	return S_OK;
}

static artik_error request(artik_sensor_handle *handle,
		artik_sensor_config *config)
{
	artik_i2c_module *i2c;
	struct hts221_config_s *elem;

	elem = (struct hts221_config_s *) artik_list_get_by_check(hts221_list,
			(ARTIK_LIST_FUNCB) check_exist,
			(void *)(intptr_t) ((artik_i2c_config *) config->config)->id);
	int ret;

	if (elem) {

		/* This is multi device sensor and it has already been
		 * initialized
		 */
		*handle = (artik_sensor_handle)elem->node.handle;
		elem->number_of_instances++;
		return S_OK;
	}

	elem = (struct hts221_config_s *) artik_list_add(&hts221_list, 0,
			sizeof(struct hts221_config_s));

	if (elem) {
		elem->node.handle = (ARTIK_LIST_HANDLE) elem;
		elem->number_of_instances = 1;
		i2c = (artik_i2c_module *) artik_request_api_module("i2c");

		if (!i2c) {
			release(elem);
			return E_INVALID_VALUE;
		}

		elem->i2c = i2c;

		ret = i2c->request(&elem->hdl,
				(artik_i2c_config *) config->config);

		if (ret != S_OK) {
			*handle = NULL;
			release(elem);
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
	struct hts221_config_s *elem;

	elem = (struct hts221_config_s *) artik_list_get_by_handle(hts221_list,
			(ARTIK_LIST_HANDLE) handle);

	if (elem) {
		if (!(--elem->number_of_instances)) {
			artik_list_delete_node(&hts221_list,
							(artik_list *) elem);
			if (elem->i2c) {
				(void)elem->i2c->release(elem->hdl);
				artik_release_api_module(elem->i2c);
			}
		}
	}

	return S_OK;
}

static artik_error get_humidity(artik_sensor_handle handle, int *store)
{
	int ret;
	unsigned short h0_rh = 0, h1_rh = 0;
	short h_out = 0, h0_t0_out = 0, h1_t0_out = 0;
	double humidity;

	if (!store)
		return E_BAD_ARGS;

	ret = get_data(handle, HTS221_REG_H_OUT_L, (char *)&h_out, 2);
	if (ret != S_OK)
		return ret;

	log_dbg("h_out(%d:%04x)\n", h_out, h_out & 0xffff);

	ret = get_data(handle, HTS221_REG_H0_RH_X2, (char *)&h0_rh, 1);
	if (ret != S_OK)
		return ret;

	log_dbg("h0_rh(%d:%04x)\n", h0_rh, h0_rh & 0xffff);

	ret = get_data(handle, HTS221_REG_H1_RH_X2, (char *)&h1_rh, 1);
	if (ret != S_OK)
		return ret;

	log_dbg("h1_rh(%d:%04x)\n", h1_rh, h1_rh * 0xffff);

	ret = get_data(handle, HTS221_REG_H0_T0_OUT_L, (char *)&h0_t0_out, 2);
	if (ret != S_OK)
		return ret;

	log_dbg("h0_t0_out(%d:%04x)\n", h0_t0_out, h0_t0_out & 0xffff);

	ret = get_data(handle, HTS221_REG_H1_T0_OUT_L, (char *)&h1_t0_out, 2);
	if (ret != S_OK)
		return ret;

	log_dbg("h1_t0_out(%d:%04x)\n", h1_t0_out, h1_t0_out & 0xffff);

	humidity = (double) (h1_rh - h0_rh) / (h1_t0_out - h0_t0_out);
	humidity *= (h_out - h0_t0_out);
	humidity += h0_rh;
	humidity /= 2;

	log_dbg("h(%04d.%d)\n", (int) (humidity * 100) / 100,
			(int) (humidity * 100) % 100);

	*store = (int)humidity;

	return S_OK;
}

static artik_error get_celsius(artik_sensor_handle handle, int *store)
{
	unsigned char mask = 0;
	unsigned short t0_deg = 0, t1_deg = 0;
	short t0_out = 0, t1_out = 0, t_out = 0;

	double temperature;
	int ret;

	if (!store)
		return E_BAD_ARGS;

	ret = get_data(handle, HTS221_REG_T_OUT_L, (char *)&t_out, 2);
	if (ret != S_OK)
		return ret;

	log_dbg("t_out(%d:%04x)\n", t_out, t_out & 0xffff);

	ret = get_data(handle, HTS221_REG_T0_DEGC_X8, (char *)&t0_deg, 1);
	if (ret != S_OK)
		return ret;

	log_dbg("t0_deg(%d:%04x)\n", t0_deg, t0_deg & 0xffff);

	ret = get_data(handle, HTS221_REG_T1_DEGC_X8, (char *)&t1_deg, 1);
	if (ret != S_OK)
		return ret;

	log_dbg("t1_deg(%d:%04x)\n", t1_deg, t1_deg & 0xffff);

	ret = get_data(handle, HTS221_REG_T1_T0_MSB, (char *)&mask, 1);
	if (ret != S_OK)
		return ret;

	t0_deg |= ((mask & 0x03) << 8);
	t1_deg |= (((mask & 0x0C) >> 2) << 8);

	log_dbg("t0_deg(%d:%04x), t1_deg(%d:%04x)\n", t0_deg, t0_deg & 0xffff,
			t1_deg, t1_deg * 0xffff);

	ret = get_data(handle, HTS221_REG_T0_OUT_L, (char *)&t0_out, 2);
	if (ret != S_OK)
		return ret;

	log_dbg("t0_out(%d:%04x)\n", t0_out, t0_out & 0xffff);

	ret = get_data(handle, HTS221_REG_T1_OUT_L, (char *)&t1_out, 2);
	if (ret != S_OK)
		return ret;

	log_dbg("t1_out(%d:%04x)\n", t1_out, t1_out & 0xffff);

	temperature  = (double)(t1_deg - t0_deg) / (t1_out - t0_out);
	temperature *= (t_out - t0_out);
	temperature += t0_deg;

	temperature /= 8;

	log_dbg("t(%04d.%d)\n", (int) (temperature * 100) / 100,
			(int) (temperature * 100) % 100);

	*store = (int)temperature;

	return S_OK;
}

static artik_error get_fahrenheit(artik_sensor_handle handle, int *store)
{
	int ret;
	double data;

	if (!store)
		return E_BAD_ARGS;

	ret = get_celsius(handle, store);
	if (ret < 0)
		return ret;

	data = (double)(*store);
	data *= 1.8;

	*store = (int)data + 32;

	return S_OK;
}
