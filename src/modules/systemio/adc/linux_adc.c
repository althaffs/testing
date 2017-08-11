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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include <artik_log.h>
#include <artik_adc.h>

#include "os_adc.h"

typedef struct {
	int fd;
	char *path;
} artik_adc_user_data_t;

#define ADC_SYSFS "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw"
#define MAX_SIZE 128

static artik_error os_adc_open(artik_adc_config *config)
{
	artik_adc_user_data_t *user_data = NULL;

	log_dbg("");

	if (!config)
		return E_BAD_ARGS;

	user_data = (artik_adc_user_data_t *)config->user_data;

	log_dbg("Opening %s", user_data->path);

	user_data->fd = open(user_data->path, O_SYNC | O_RDONLY);

	return (user_data->fd < 0) ? E_BUSY : S_OK;
}

artik_error os_adc_request(artik_adc_config *config)
{
	artik_adc_user_data_t *user_data = NULL;
	int val = -1;

	log_dbg("");

	if (!config)
		return E_BAD_ARGS;

	user_data = malloc(sizeof(artik_adc_user_data_t));
	if (!user_data)
		return E_NO_MEM;

	user_data->path = malloc(MAX_SIZE + 1);
	if (!user_data->path) {
		free(user_data);
		return E_NO_MEM;
	}

	snprintf(user_data->path, MAX_SIZE, ADC_SYSFS, config->pin_num);

	config->user_data = user_data;

	return os_adc_get_value(config, &val);
}

artik_error os_adc_release(artik_adc_config *config)
{
	artik_adc_user_data_t *user_data = NULL;

	log_dbg("");

	if (!config)
		return E_BAD_ARGS;

	user_data = (artik_adc_user_data_t *)config->user_data;

	if (user_data) {
		if (user_data->path)
			free(user_data->path);
		free(user_data);
	}

	return S_OK;
}

artik_error os_adc_get_value(artik_adc_config *config, int *value)
{
	artik_adc_user_data_t *user_data = NULL;
	unsigned long int result = 0;
	char value_str[MAX_SIZE];
	char *endptr = NULL;

	log_dbg("");

	user_data = (artik_adc_user_data_t *)config->user_data;

	if (!config)
		return E_BAD_ARGS;

	if (!value)
		return E_BAD_ARGS;

	if (os_adc_open(config) != S_OK)
		return E_BAD_ARGS;

	if (read(user_data->fd, value_str, sizeof(value_str)) == -1)
		return E_BUSY;

	close(user_data->fd);
	result = strtoul(value_str, &endptr, 0);

	if (value_str == endptr || result == ULONG_MAX)
		return E_BUSY;

	*value = result;

	return S_OK;
}
