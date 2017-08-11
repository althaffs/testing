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

#include <devices/CM3323E.h>

#define	CM3323_REG_CONF		0x00
#define	CM3323_REG_DATA_R	0x08
#define	CM3323_REG_DATA_G	0x09
#define	CM3323_REG_DATA_B	0x0A
#define	CM3323_REG_DATA_W	0x0B

struct cm3323e_config_s {
	artik_list node;
	artik_i2c_module *i2c;
	artik_i2c_handle hdl;
	int id;
};

static artik_error request(artik_sensor_handle *handle,
		artik_sensor_config *config);
static artik_error release(artik_sensor_handle handle);
static artik_error get_intensity(artik_sensor_handle handle, int *store);

artik_sensor_light cm3323e_sensor = {
	request,
	release,
	get_intensity
};

static artik_list *cm3323e_list = NULL;

static int check_exist(struct cm3323e_config_s *elem, int val_id)
{
	if (elem->id == val_id)
		return 1;
	return 0;
}

static artik_error initialize(artik_i2c_module *i2c, artik_i2c_handle handle)
{
	artik_error ret;

	short data;

	ret = i2c->read_register(handle, CM3323_REG_CONF, (char *)&data, 2);
	if (ret < 0)
		return ret;

	log_dbg("CM3323E CONF DATA: %04x\n", data);

	/* IT[2:0] : 0, Trig: 0, AF: 1, SD: 1 */
	data = 0x0030;
	ret = i2c->write_register(handle, CM3323_REG_CONF, (char *) &data, 2);
	if (ret < 0)
		return ret;

	return S_OK;
}

static artik_error request(artik_sensor_handle *handle,
		artik_sensor_config *config)
{
	artik_i2c_module *i2c;
	struct cm3323e_config_s *elem;

	elem = (struct cm3323e_config_s *) artik_list_get_by_check(cm3323e_list,
			(ARTIK_LIST_FUNCB) check_exist,
			(void *)(intptr_t) ((artik_i2c_config *) config->config)->id);
	int ret;

	if (elem)
		return E_BUSY;

	elem = (struct cm3323e_config_s *) artik_list_add(&cm3323e_list, 0,
			sizeof(struct cm3323e_config_s));

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
	struct cm3323e_config_s *elem;

	elem = (struct cm3323e_config_s *) artik_list_get_by_handle(
			cm3323e_list,
			(ARTIK_LIST_HANDLE) handle);

	if (elem) {
		artik_list_delete_node(&cm3323e_list, (artik_list *) elem);
		if (elem->i2c) {
			(void)elem->i2c->release(elem->hdl);
			artik_release_api_module(elem->i2c);
		}
	}

	return S_OK;
}

static artik_error get_intensity(artik_sensor_handle handle, int *store)
{
	artik_error ret;
	struct cm3323e_config_s *cm3323e;
	short buffer = 0;

	if (!store)
		return E_BAD_ARGS;

	cm3323e = (struct cm3323e_config_s *) artik_list_get_by_handle(
			cm3323e_list,
			(ARTIK_LIST_HANDLE) handle);

	if (!cm3323e)
		return E_INVALID_VALUE;

	/* Read Intensity of white light */
	ret = cm3323e->i2c->read_register(cm3323e->hdl, CM3323_REG_DATA_W,
					(char *) &buffer, 2);

	if (ret < 0)
		return ret;

	*store = ((buffer & 0xffff)*100)/65535;

	return S_OK;
}
