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

#include <artik_adc.h>

#include "os_adc.h"

#include <tinyara/analog/adc.h>
#include <tinyara/analog/ioctl.h>

typedef struct {
	int fd;
	char *path;
} artik_adc_user_data_t;

#define MAX_SIZE 128
#define S5J_ADC_MAX_CHANNELS	4

artik_error os_adc_request(artik_adc_config *config)
{
	artik_adc_user_data_t *user_data = NULL;

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

	snprintf(user_data->path, MAX_SIZE, "/dev/adc0");

	config->user_data = user_data;
	user_data->fd = open(user_data->path, O_RDONLY);

	return (user_data->fd < 0) ? E_BUSY : S_OK;
}

artik_error os_adc_release(artik_adc_config *config)
{
	artik_adc_user_data_t *user_data = NULL;

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
	struct adc_msg_s sample[S5J_ADC_MAX_CHANNELS];
	size_t readsize;
	ssize_t nbytes;

	user_data = (artik_adc_user_data_t *)config->user_data;

	if (!config)
		return E_BAD_ARGS;

	if (ioctl(user_data->fd, ANIOC_TRIGGER, 0) < -1) {
		close(user_data->fd);
		return E_BUSY;
	}

	readsize = S5J_ADC_MAX_CHANNELS * sizeof(struct adc_msg_s);
	nbytes = read(user_data->fd, sample, readsize);

	if (nbytes < 0) {
		close(user_data->fd);
		return E_BUSY;
	}

	int nsamples = nbytes / sizeof(struct adc_msg_s);

	if (nsamples * sizeof(struct adc_msg_s) == nbytes)
		*value = sample[config->pin_num].am_data;

	close(user_data->fd);

	return S_OK;
}
