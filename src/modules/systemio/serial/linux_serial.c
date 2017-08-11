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


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/eventfd.h>
#include <sys/time.h>

#include "artik_serial.h"
#include "os_serial.h"
#include <artik_module.h>
#include <artik_log.h>

#define MAX_PATH	128
#define MAX(a, b)	((a > b) ? a : b)

typedef struct {
	int fd;
	int watch_id;
	fd_set rdfs;
	artik_serial_callback callback;
	void *user_data;
	artik_loop_module *loop;
} os_serial_data;

/* This table must strictly follow platform IDs order */
static char *const plat_port[] = {
	"/dev/ttyS%d",		/* GENERIC */
	"/dev/ttySAC%d",	/* ARTIK520 */
	"/dev/ttySAC%d",	/* ARTIK1020 */
	"/dev/ttySAC%d",	/* ARTIK710 */
	"/dev/ttyAMA%d",        /* ARTIK530 */
};

/* Must strictly follow enum artik_serial_baudrate_t in artik_serial.h */
static const unsigned int baudrate_value[] = { B4800,
						B9600,
						14400,
						B19200,
						B38400,
						B57600,
						B115200 };

artik_error os_serial_request(artik_serial_config *config)
{
	os_serial_data *data_user = NULL;
	int platid = artik_get_platform();
	struct termios tty;
	char entry[MAX_PATH];

	data_user = malloc(sizeof(os_serial_data));
	if (!data_user)
		return -E_NO_MEM;

	config->data_user = data_user;

	snprintf(entry, MAX_PATH, plat_port[platid], config->port_num);
	data_user->fd = open(entry, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (data_user->fd < 0) {
		os_serial_release(config);
		return E_ACCESS_DENIED;
	}
	FD_ZERO(&data_user->rdfs);
	FD_SET(data_user->fd, &data_user->rdfs);

	if (tcgetattr(data_user->fd, &tty)) {
		os_serial_release(config);
		return E_ACCESS_DENIED;
	}
	tty.c_iflag &=
	    ~(IXON | IXOFF | IXANY | CSIZE | CSTOPB | PARENB | PARODD | CBAUD |
	      CRTSCTS);
	tty.c_cflag |= (CREAD | CLOCAL);
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 5;

	/* Configure baudrate */
	cfsetispeed(&tty, baudrate_value[config->baudrate]);
	cfsetospeed(&tty, baudrate_value[config->baudrate]);

	/* UART ports should be use in raw mode */
	cfmakeraw(&tty);

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
	tcflush(data_user->fd, TCIFLUSH);
	/* Apply attributes */
	if (tcsetattr(data_user->fd, TCSANOW, &tty)) {
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
				unsigned char *const buf,
				int *len)
{
	os_serial_data *data_user = config->data_user;
	int ret = 0;

	if (data_user->fd < 0)
		return E_ACCESS_DENIED;
	ret = write(data_user->fd, buf, *len);
	if (ret < 0)
		return E_ACCESS_DENIED;
	*len = ret;
	return S_OK;
}

int os_serial_change_callback(int fd, enum watch_io io, void *user_data)
{
	os_serial_data *data = (os_serial_data *)user_data;
	static struct timeval tv = {1, 0};
	unsigned char buff[255] = "";
	int res = 0;

	if (!data) {
		log_err("invalid user_data provided");
		return 0;
	}
	if (data->fd < 0) {
		log_err("wrong fd");
		return 0;
	}
	res = select(data->fd+1, &data->rdfs, NULL, NULL, &tv);
	if (res && FD_ISSET(data->fd, &data->rdfs))
		res = read(data->fd, buff, 255);
	else if (res > 0)
		res = 0;
	FD_CLR(data->fd, &data->rdfs);
	FD_SET(data->fd, &data->rdfs);
	if (res < 0)
		return 0;
	if (data->callback) {
		data->callback(data->user_data, (!res ? NULL : buff), res);
		res = select(fd+1, &data->rdfs, NULL, NULL, &tv);
		if (res == 0 || (res > 0 && !FD_ISSET(fd, &data->rdfs)))
			data->callback(data->user_data, NULL, 0);
	}
	FD_CLR(fd, &data->rdfs);
	if (res)
		FD_SET(fd, &data->rdfs);
	return 1;
}

artik_error os_serial_set_received_callback(artik_serial_config *config,
				artik_serial_callback callback, void *user_data)
{
	os_serial_data *data = (os_serial_data *)config->data_user;
	artik_error ret = S_OK;

	if (data->fd == -1) {
		log_err("invalid fd provided");
		return E_BUSY;
	}
	data->callback = callback;
	data->user_data = user_data;

	data->loop = (artik_loop_module *)artik_request_api_module("loop");
	if (!data->loop) {
		log_err("Failed to request loop module");
		ret = E_BUSY;
		goto exit;
	}

	ret = data->loop->add_fd_watch(data->fd, WATCH_IO_ERR | WATCH_IO_IN |
			WATCH_IO_HUP | WATCH_IO_NVAL,
			os_serial_change_callback, (void *)data,
			&data->watch_id);
	if (ret != S_OK) {
		log_err("Failed to set fd watch callback");
		goto exit;
	}

exit:
	if (ret != S_OK) {
		log_err("Failed to callback");
		if (data->loop) {
			artik_release_api_module(data->loop);
			data->loop = NULL;
		}
	}
	return ret;
}

artik_error os_serial_unset_received_callback(artik_serial_config *config)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	os_serial_data *data = (os_serial_data *)config->data_user;

	if (data->fd) {
		if (data->loop) {
			data->loop->remove_fd_watch(data->watch_id);
			artik_release_api_module(loop);
		}
		memset(data, 0, sizeof(*data));
	}
	return S_OK;
}
