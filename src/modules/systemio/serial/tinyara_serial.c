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
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/select.h>

#include "artik_serial.h"
#include "os_serial.h"

typedef struct {
	int fd;
	fd_set rdfs;
	artik_serial_callback callback;
	void *user_data;
} os_serial_data;

artik_error os_serial_request(artik_serial_config *config)
{
	os_serial_data *data_user = NULL;
	struct termios tty;

	data_user = malloc(sizeof(os_serial_data));
	if (!data_user)
		return -E_NO_MEM;

	config->data_user = data_user;

	char path_str[PATH_MAX];

	snprintf(path_str, PATH_MAX, "/dev/ttyS%d", config->port_num);
	data_user->fd = open(path_str, O_RDWR | O_NOCTTY);

	if (data_user->fd < 0) {
		os_serial_release(config);
		return E_ACCESS_DENIED;
	}
	FD_ZERO(&data_user->rdfs);
	FD_SET(data_user->fd, &data_user->rdfs);

	if (tcgetattr(data_user->fd, &tty) < 0) {
		os_serial_release(config);
		return E_ACCESS_DENIED;
	}

	/* Configure baudrate */
	switch (config->baudrate) {
	case ARTIK_SERIAL_BAUD_4800:
		tty.c_speed = 4800;
		break;
	case ARTIK_SERIAL_BAUD_9600:
		tty.c_speed = 9600;
		break;
	case ARTIK_SERIAL_BAUD_14400:	/*Not Support 14400 Baud*/
		break;
	case ARTIK_SERIAL_BAUD_19200:
		tty.c_speed = 19200;
		break;
	case ARTIK_SERIAL_BAUD_38400:
		tty.c_speed = 38400;
		break;
	case ARTIK_SERIAL_BAUD_57600:
		tty.c_speed = 57600;
		break;
	case ARTIK_SERIAL_BAUD_115200:
		tty.c_speed = 115200;
		break;
	default:
		break;
	}

	/* Configure flow control */
	switch (config->flowctrl) {
	case ARTIK_SERIAL_FLOWCTRL_HARD:
		tty.c_cflag |= CRTSCTS;
	case ARTIK_SERIAL_FLOWCTRL_SOFT:
		tty.c_cflag |= (IXON | IXOFF | IXANY);
	default:
		break;
	}

	/* Configure parity */
	switch (config->parity) {
	case ARTIK_SERIAL_PARITY_ODD:
		tty.c_cflag |= (PARENB | PARODD);
	case ARTIK_SERIAL_PARITY_EVEN:
		tty.c_cflag |= PARENB;
	default:
		break;
	}

	/* Configure stop bits */
	if (config->stop_bits == ARTIK_SERIAL_STOP_2BIT)
		tty.c_cflag |= CSTOPB;

	/* Configure data bits */
	switch (config->data_bits) {
	case ARTIK_SERIAL_DATA_7BIT:
		tty.c_cflag |= CS7;
	case ARTIK_SERIAL_DATA_8BIT:
		tty.c_cflag |= CS8;
	default:
		break;
	}

	/* flush port before applying attributes */
	if (tcflush(data_user->fd, TCIFLUSH) < 0) {
		os_serial_release(config);
		return E_ACCESS_DENIED;
	}

	/* Apply attributes */
	if (tcsetattr(data_user->fd, TCSANOW, &tty) < 0) {
		os_serial_release(config);
		return E_ACCESS_DENIED;
	}
	return S_OK;
}

artik_error os_serial_release(artik_serial_config *config)
{
	os_serial_data *data_user = config->data_user;

	if (data_user != NULL) {
		FD_CLR(data_user->fd, &data_user->rdfs);
		if (data_user->fd >= 0)
			close(data_user->fd);
		free(data_user);
	}
	return S_OK;
}

artik_error os_serial_read(artik_serial_config *config, unsigned char *buf,
			int *len)
{
	os_serial_data *data_user = config->data_user;
	int ret = 0;

	if (data_user->fd < 0)
		return E_ACCESS_DENIED;

	if (buf == NULL)
		return E_INVALID_VALUE;

	ret = read(data_user->fd, buf, *len);
	if (ret < 0) {
		if (errno == EAGAIN)
			return E_TRY_AGAIN;
		else
			return E_ACCESS_DENIED;
	}
	*len = ret;
	return S_OK;
}

artik_error os_serial_write(artik_serial_config *config,
			unsigned char *const buf, int *len)
{
	os_serial_data *data_user = config->data_user;
	int ret = 0;

	if (data_user->fd < 0)
		return E_ACCESS_DENIED;

	if (buf == NULL)
		return E_INVALID_VALUE;

	ret = write(data_user->fd, buf, *len);
	if (ret < 0)
		return E_ACCESS_DENIED;
	*len = ret;
	return S_OK;
}

artik_error os_serial_set_received_callback(artik_serial_config *config,
			artik_serial_callback callback, void *user_data)
{
	return S_OK;
}

artik_error os_serial_unset_received_callback(artik_serial_config *config)
{
	return S_OK;
}
