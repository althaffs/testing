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
#include "artik_spi.h"
#include "artik_log.h"

#include <devices/K6DS3.h>

#define K6DS3_FACTORY_ID	0x69

#define K6DS3_REG_FUNC_CFG_ACC	0x01
#define K6DS3_REG_SYNC_TIME	0x04
#define K6DS3_REG_SYNC_EN	0x05
#define K6DS3_REG_FIFO_CTRL	0x06		/* length: 5-bytes */
#define K6DS3_REG_ORIENT_CFG_G	0x0B
#define K6DS3_REG_INT_CTRL	0x0D		/* length: 2-bytes */
#define K6DS3_REG_WHO_AM_I	0x0F

#define K6DS3_REG_CTRL1_XL	0x10
#define K6DS3_REG_CTRL2_G	0x11

#define K6DS3_REG_MASTER_CFG	0x1A
#define K6DS3_REG_WAKE_UP_SRC	0x1B
#define K6DS3_REG_TAP_SRC	0x1C
#define K6DS3_REG_D6D_SRC	0x1D
#define K6DS3_REG_STATUS	0x1E
#define K6DS3_REG_OUT_TEMP	0x20

#define K6DS3_REG_OUTX_G	0x22		/* length: 2-bytes */
#define K6DS3_REG_OUTY_G	0x24		/* length: 2-bytes */
#define K6DS3_REG_OUTZ_G	0x26		/* length: 2-bytes */
#define K6DS3_REG_OUTX_XL	0x28		/* length: 2-bytes */
#define K6DS3_REG_OUTY_XL	0x2A		/* length: 2-bytes */
#define K6DS3_REG_OUTZ_XL	0x2C		/* length: 2-bytes */

#define K6DS3_REG_SENSOR_HUB_L	0x2E		/* length: 12-bytes */
#define K6DS3_REG_FIFO_STATUS	0x3A		/* length: 4-bytes */
#define K6DS3_REG_FIFO_DATA	0x3E		/* length: 2-bytes */
#define K6DS3_REG_TIMESTAMP	0x40		/* length: 3-bytes */
#define K6DS3_REG_STEP_COUNTER	0x4B		/* length: 2-bytes */
#define K6DS3_REG_SENSOR_HUB_H	0x4D		/* length: 6-bytes */
#define K6DS3_REG_FUNC_SRC	0x53

#define K6DS3_REG_TAB_CFG	0x58
#define K6DS3_REG_TAB_THS_6D	0x59
#define K6DS3_REG_INT_DUR2	0x5A
#define K6DS3_REG_WAKEUP_THS	0x5B
#define K6DS3_REG_WAKEUP_DUR	0x5C
#define K6DS3_REG_FREE_FALL	0x5D
#define K6DS3_REG_MD_CFG	0x5E		/* length: 2-bytes */

struct k6ds3_config_s {
	artik_list node;
	artik_spi_module *spi;
	artik_spi_handle hdl;
	int bus;
	int number_of_instances;
};

static artik_error request(artik_sensor_handle *handle,
		artik_sensor_config *config);
static artik_error release(artik_sensor_handle handle);
static artik_error get_speed_x(artik_sensor_handle handle, int *store);
static artik_error get_speed_y(artik_sensor_handle handle, int *store);
static artik_error get_speed_z(artik_sensor_handle handle, int *store);

static artik_error get_gyro_pitch(artik_sensor_handle handle, int *store);
static artik_error get_gyro_roll(artik_sensor_handle handle, int *store);
static artik_error get_gyro_yaw(artik_sensor_handle handle, int *store);

artik_sensor_accelerometer k6ds3_xl_sensor = { request, release,
		get_speed_x, get_speed_y, get_speed_z };

artik_sensor_gyro k6ds3_gyro_sensor = { request, release,
		get_gyro_yaw, get_gyro_roll, get_gyro_pitch };

static artik_list *k6ds3_list = NULL;

static int check_exist(struct k6ds3_config_s *elem, int bus)
{
	if (elem->bus == bus)
		return 1;
	return 0;
}

static artik_error initialize(artik_spi_module *spi, artik_spi_handle handle)
{
	artik_error ret;
	unsigned char buffer[3]    = { 0,};

	buffer[0] = K6DS3_REG_WHO_AM_I | 0x80;
	ret = spi->read_write(handle, (char *)&buffer[0], (char *)&buffer[1],
									2);
	if (ret != S_OK)
		return ret;

	if (buffer[2] != K6DS3_FACTORY_ID) {
		log_dbg("unsupported device(%x)\n", buffer[2]);
		return E_NOT_SUPPORTED;
	}

	buffer[0] = K6DS3_REG_CTRL1_XL;
	buffer[1] = 0x80;
	ret = spi->write(handle, (char *)buffer, 2);
	if (ret != S_OK)
		return ret;

	buffer[0] = K6DS3_REG_CTRL2_G;
	buffer[1] = 0x80;
	ret = spi->write(handle, (char *)buffer, 2);
	if (ret != S_OK)
		return ret;

	return S_OK;
}

static artik_error request(artik_sensor_handle *handle,
		artik_sensor_config *config)
{
	artik_spi_module *spi;
	struct k6ds3_config_s *elem;

	if (!config)
		return E_BAD_ARGS;
	elem = (struct k6ds3_config_s *) artik_list_get_by_check(k6ds3_list,
			(ARTIK_LIST_FUNCB) check_exist,
			(void *)(intptr_t) ((artik_spi_config *) config->config)->bus);
	int ret;

	if (elem) {

		/* This is multi device sensor and it has already been
		 * initialized
		 */
		*handle = (artik_sensor_handle)elem->node.handle;
		elem->number_of_instances++;
		return S_OK;
	}

	elem = (struct k6ds3_config_s *) artik_list_add(&k6ds3_list, 0,
			sizeof(struct k6ds3_config_s));

	if (elem) {
		elem->node.handle = (ARTIK_LIST_HANDLE) elem;
		elem->number_of_instances = 1;
		spi = (artik_spi_module *) artik_request_api_module("spi");

		if (!spi) {
			release(elem);
			return E_INVALID_VALUE;
		}

		elem->spi = spi;

		ret = spi->request(&elem->hdl,
				(artik_spi_config *) config->config);

		if (ret != S_OK) {
			*handle = NULL;
			release(elem);
			return ret;
		}

		elem->bus = ((artik_spi_config *) config->config)->bus;
		*handle = (artik_sensor_handle)elem->node.handle;

		/* Initalize */

		ret = initialize(spi, elem->hdl);
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
	struct k6ds3_config_s *elem;

	elem = (struct k6ds3_config_s *) artik_list_get_by_handle(k6ds3_list,
			(ARTIK_LIST_HANDLE) handle);

	if (elem) {
		if (!(--elem->number_of_instances)) {
			artik_list_delete_node(&k6ds3_list,
							(artik_list *) elem);
			if (elem->spi) {
				(void)elem->spi->release(elem->hdl);
				artik_release_api_module(elem->spi);
			}
		}
	}

	return S_OK;
}

static artik_error get_data(artik_sensor_handle handle, unsigned char reg,
				int *res)
{
	struct k6ds3_config_s *elem;
	unsigned char rxdata[3] = { 0, };
	unsigned char txdata[2] = { 0, };
	int ret     = S_OK;
	short value = 0;


	elem = (struct k6ds3_config_s *) artik_list_get_by_handle(k6ds3_list,
			(ARTIK_LIST_HANDLE) handle);

	if (!elem)
		return E_NOT_INITIALIZED;

	reg |= 0x80;
	txdata[0] = reg;
	ret = elem->spi->read_write(elem->hdl, (char *)&txdata[0],
							(char *)&rxdata[0], 3);
	if (ret != S_OK)
		return ret;

	*res = 0;
	value = rxdata[2] << 8 | rxdata[1];
	*res = value;

	return S_OK;
}

static artik_error get_speed_x(artik_sensor_handle handle, int *store)
{
	return get_data(handle, K6DS3_REG_OUTX_XL, (int *) store);
}

static artik_error get_speed_y(artik_sensor_handle handle, int *store)
{
	return get_data(handle, K6DS3_REG_OUTY_XL, (int *) store);
}

static artik_error get_speed_z(artik_sensor_handle handle, int *store)
{
	return get_data(handle, K6DS3_REG_OUTZ_XL, (int *) store);
}

static artik_error get_gyro_pitch(artik_sensor_handle handle, int *store)
{
	return get_data(handle, K6DS3_REG_OUTX_G, (int *) store);
}

static artik_error get_gyro_roll(artik_sensor_handle handle, int *store)
{
	return get_data(handle, K6DS3_REG_OUTY_G, (int *) store);
}

static artik_error get_gyro_yaw(artik_sensor_handle handle, int *store)
{
	return get_data(handle, K6DS3_REG_OUTZ_G, (int *) store);
}
