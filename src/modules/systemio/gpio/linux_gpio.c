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


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/eventfd.h>

#include <artik_module.h>
#include <artik_log.h>
#include <artik_loop.h>
#include <artik_gpio.h>
#include "os_gpio.h"

#define MAX_VAL_STRING	128

typedef struct {
	int watch_id;
	int fd;
	artik_gpio_callback callback;
	void *user_data;
	artik_loop_module *loop;
} os_gpio_data;

static int write_sysfs_entry(char *entry, char *value)
{
	int fd = open(entry, O_WRONLY);

	if (fd == -1)
		return -errno;

	if (write(fd, value, strlen(value)) == -1) {
		close(fd);
		return -errno;
	}

	close(fd);

	return 0;
}

static int read_sysfs_entry(char *entry, char *value)
{
	int fd = open(entry, O_RDONLY);

	if (!fd)
		return -errno;

	lseek(fd, 0, SEEK_SET);
	if (read(fd, value, MAX_VAL_STRING) == -1) {
		close(fd);
		return -errno;
	}

	close(fd);

	return 0;
}

artik_error os_gpio_request(artik_gpio_config *config)
{
	char *export_path = "/sys/class/gpio/export";
	char *unexport_path = "/sys/class/gpio/unexport";
	char direction_path[MAX_VAL_STRING];
	char value_path[MAX_VAL_STRING];
	char gpio_num[MAX_VAL_STRING];
	char gpio_dir[MAX_VAL_STRING];
	char gpio_value[MAX_VAL_STRING];
	os_gpio_data *data;
	int ret;

	log_dbg("");

	/* Check input parameters */
	if (((int)config->id < 0) ||
			(config->dir >= GPIO_DIR_INVALID) ||
			(config->edge >= GPIO_EDGE_INVALID))
		return E_BAD_ARGS;

	snprintf(gpio_num, MAX_VAL_STRING, "%d", config->id);
	snprintf(gpio_dir, MAX_VAL_STRING, "%s", (config->dir == GPIO_OUT) ?
								"out" : "in");
	snprintf(gpio_value, MAX_VAL_STRING, "%s", config->initial_value ?
								"1" : "0");
	snprintf(direction_path, MAX_VAL_STRING,
				"/sys/class/gpio/gpio%d/direction", config->id);
	snprintf(value_path, MAX_VAL_STRING,
				"/sys/class/gpio/gpio%d/value", config->id);

	/* Export GPIO */
	ret = write_sysfs_entry(export_path, gpio_num);
	if (ret < 0) {
		printf("1. cannot open pin here error while writing to out\n");
		goto cleanup;
	}
	/* Set GPIO direction */
	ret = write_sysfs_entry(direction_path, gpio_dir);
	if (ret < 0)
		goto cleanup;

	/* Set initial value if output only */
	if (config->dir == GPIO_OUT) {
		ret = write_sysfs_entry(value_path, gpio_value);
		if (ret < 0)
			goto cleanup;
	}

	/* Set edge if input */
	if (config->dir == GPIO_IN) {
		char edge_path[MAX_VAL_STRING];
		char edge_value[MAX_VAL_STRING];

		snprintf(edge_path, MAX_VAL_STRING,
				"/sys/class/gpio/gpio%d/edge", config->id);

		switch (config->edge) {
		case GPIO_EDGE_RISING:
			snprintf(edge_value, MAX_VAL_STRING, "%s", "rising");
			break;
		case GPIO_EDGE_FALLING:
			snprintf(edge_value, MAX_VAL_STRING, "%s", "falling");
			break;
		case GPIO_EDGE_BOTH:
			snprintf(edge_value, MAX_VAL_STRING, "%s", "both");
			break;
		default:
			snprintf(edge_value, MAX_VAL_STRING, "%s", "none");
			break;
		}

		ret = write_sysfs_entry(edge_path, edge_value);
		if (ret < 0)
			goto cleanup;
	}

	data = malloc(sizeof(os_gpio_data));
	if (!data) {
		ret = E_NO_MEM;
		goto cleanup;
	}

	memset(data, 0, sizeof(*data));
	config->user_data = (void *)data;

	return S_OK;

cleanup:
	/* Unexport GPIO */
	write_sysfs_entry(unexport_path, gpio_num);

	return (ret == -EACCES) ? E_ACCESS_DENIED : E_BUSY;
}

artik_error os_gpio_release(artik_gpio_config *config)
{
	char *unexport_path = "/sys/class/gpio/unexport";
	char gpio_num[MAX_VAL_STRING];

	log_dbg("");

	snprintf(gpio_num, MAX_VAL_STRING, "%d", config->id);
	write_sysfs_entry(unexport_path, gpio_num);

	write_sysfs_entry(unexport_path, gpio_num);

	if (config->user_data) {
		free(config->user_data);
		config->user_data = NULL;
	}

	return S_OK;
}

int os_gpio_read(artik_gpio_config *config)
{
	char value_path[MAX_VAL_STRING];
	char gpio_value[MAX_VAL_STRING];
	int ret = -1;

	log_dbg("");

	if (config->dir != GPIO_IN)
		return E_ACCESS_DENIED;

	snprintf(value_path, MAX_VAL_STRING,
				"/sys/class/gpio/gpio%d/value", config->id);

	if (read_sysfs_entry(value_path, gpio_value) < 0)
		return -1;

	if (!strncmp(gpio_value, "0", 1))
		ret = 0;
	else if (!strncmp(gpio_value, "1", 1))
		ret = 1;

	return ret;
}

artik_error os_gpio_write(artik_gpio_config *config, int value)
{
	char value_path[MAX_VAL_STRING];
	char gpio_value[MAX_VAL_STRING];

	log_dbg("");

	if (config->dir != GPIO_OUT)
		return E_ACCESS_DENIED;

	snprintf(value_path, MAX_VAL_STRING,
				"/sys/class/gpio/gpio%d/value", config->id);
	snprintf(gpio_value, MAX_VAL_STRING, "%s", value ? "1" : "0");

	if (write_sysfs_entry(value_path, gpio_value) < 0)
		return E_BUSY;

	return S_OK;
}

int os_gpio_change_callback(int fd, enum watch_io io, void *user_data)
{
	os_gpio_data *data = (os_gpio_data *)user_data;
	char gpio_value;
	int val = 1;

	log_dbg("");

	if (!data) {
		log_err("invalid user_data provided");
		return 0;
	}

    /* Read new state for clearing interrupt and return value */
	lseek(data->fd, 0, SEEK_SET);
	if (read(data->fd, &gpio_value, sizeof(gpio_value)) < 0)
		return 0;

	if (gpio_value == '0')
		val = 0;
	else
		val = 1;

	log_dbg("IO: %d, state=%d", io, val);

	if (data->callback)
		data->callback(data->user_data, val);

	return 1;
}

artik_error os_gpio_set_change_callback(artik_gpio_config *config,
				artik_gpio_callback callback, void *user_data)
{
	char value_path[MAX_VAL_STRING];
	artik_error ret = S_OK;
	char gpio_value;
	os_gpio_data *data = (os_gpio_data *)config->user_data;

	log_dbg("");

	/* Must be an input */
	if (config->dir != GPIO_IN)
		return E_BAD_ARGS;

	snprintf(value_path, MAX_VAL_STRING,
				"/sys/class/gpio/gpio%d/value", config->id);

	data->fd = open(value_path, O_RDONLY);
	if (!data->fd)
		return E_BUSY;

	data->callback = callback;
	data->user_data = user_data;

	/* Read value first to clear interrupts */
	lseek(data->fd, 0, SEEK_SET);
	if (read(data->fd, &gpio_value, sizeof(gpio_value)) < 0) {
		ret = E_ACCESS_DENIED;
		log_err("Failed to read gpio value");
		goto exit;
	}

	data->loop = (artik_loop_module *)artik_request_api_module("loop");
	if (!data->loop) {
		log_err("Failed to request loop module");
		ret = E_BUSY;
		goto exit;
	}

	ret = data->loop->add_fd_watch(data->fd, WATCH_IO_ERR | WATCH_IO_HUP |
								WATCH_IO_NVAL,
			os_gpio_change_callback, (void *)data, &data->watch_id);
	if (ret != S_OK) {
		log_err("Failed to set fd watch callback");
		goto exit;
	}

exit:
	if (ret != S_OK) {
		if (data->loop) {
			artik_release_api_module(data->loop);
			data->loop = NULL;
		}
		close(data->fd);
		data->fd = 0;
	}

	return ret;
}

void os_gpio_unset_change_callback(artik_gpio_config *config)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	os_gpio_data *data = (os_gpio_data *)config->user_data;

	log_dbg("");

	if (data->fd) {
		close(data->fd);

		if (data->loop) {
			data->loop->remove_fd_watch(data->watch_id);
			artik_release_api_module(loop);
		}
		memset(data, 0, sizeof(*data));
	}
}
