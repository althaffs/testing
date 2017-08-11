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

#include <artik_gpio.h>
#include "os_gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <sys/select.h>

#include <tinyara/fs/ioctl.h>

#define GPIO_DIRECTION_OUT	1
#define GPIO_DIRECTION_IN	2
#define GPIO_DRIVE_PULLUP	1
#define GPIO_DRIVE_PULLDOWN	2
#define GPIO_CMD_SET_DIRECTION	_GPIOIOC(0x0001)

typedef struct {
	int fd;
	fd_set rdfs;
	void *user_data;
} os_gpio_data;

artik_error os_gpio_request(artik_gpio_config *config)
{
	os_gpio_data *user_data = NULL;

	user_data = malloc(sizeof(os_gpio_data));
	if (!user_data)
		return -E_NO_MEM;

	config->user_data = user_data;

	char path_str[PATH_MAX];

	snprintf(path_str, PATH_MAX, "/dev/gpio%d", config->id);
	user_data->fd = open(path_str, O_RDWR);

	if (user_data->fd < 0) {
		os_gpio_release(config);
		return E_ACCESS_DENIED;
	}
	FD_ZERO(&user_data->rdfs);
	FD_SET(user_data->fd, &user_data->rdfs);

	return S_OK;
}

artik_error os_gpio_release(artik_gpio_config *config)
{
	os_gpio_data *user_data = config->user_data;

	close(user_data->fd);

	return S_OK;
}

int os_gpio_read(artik_gpio_config *config)
{
	os_gpio_data *user_data = config->user_data;
	char buf[4];

	if (ioctl(user_data->fd, GPIO_CMD_SET_DIRECTION, GPIO_DIRECTION_IN) < 0)
		return E_ACCESS_DENIED;

	if (read(user_data->fd, (void *)&buf, sizeof(buf)) < 0)
		return E_ACCESS_DENIED;

	return buf[0] == '1';
}

artik_error os_gpio_write(artik_gpio_config *config, int value)
{
	os_gpio_data *user_data = config->user_data;
	char str[4];

	if (ioctl(user_data->fd, GPIO_CMD_SET_DIRECTION,
		GPIO_DIRECTION_OUT) < 0)
		return E_ACCESS_DENIED;

	size_t str_size =  snprintf(str, 4, "%d", value != 0) + 1;

	if (write(user_data->fd, (void *)str, str_size) < 0)
		return E_ACCESS_DENIED;

	return S_OK;
}

artik_error os_gpio_set_change_callback(artik_gpio_config *config,
				artik_gpio_callback callback, void *user_data)
{
	return S_OK;
}

void os_gpio_unset_change_callback(artik_gpio_config *config)
{

}
