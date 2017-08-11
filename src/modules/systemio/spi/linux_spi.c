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


#include <linux/spi/spidev.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <artik_spi.h>
#include "os_spi.h"

#define	SPI_DEV_MAX_LEN	64

static int spi_setup(int fd, unsigned char mode, unsigned char bits,
		unsigned int speed)
{
	int ret;

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret < 0) {
		fprintf(stderr, "SPI: Could not setup mode\n");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret < 0) {
		fprintf(stderr, "SPI: Could not setup mode\n");
		return ret;
	}

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret < 0) {
		fprintf(stderr, "SPI: Could not setup bits\n");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret < 0) {
		fprintf(stderr, "SPI: Could not setup bits\n");
		return ret;
	}

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret < 0) {
		fprintf(stderr, "SPI: Could not setup speed\n");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret < 0) {
		fprintf(stderr, "SPI: Could not setup speed\n");
		return ret;
	}

	return 0;
}

artik_error os_spi_request(artik_spi_config *config)
{
	int fd = -1;
	char devname[SPI_DEV_MAX_LEN];
	artik_error ret = S_OK;

	if (!config)
		return E_BAD_ARGS;
	else if (config && config->mode == SPI_MODE_INVALID)
		return E_NOT_INITIALIZED;

	/* Try to open driver and set slave address */
	snprintf(devname, SPI_DEV_MAX_LEN, "/dev/spidev%d.%d", config->bus,
		 config->cs);

	fd = open(devname, O_SYNC | O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	ret = spi_setup(fd, config->mode, config->bits_per_word,
			config->max_speed);
	if (ret < 0) {
		fprintf(stderr, "Failed to write spi setup %s(%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
	}

	close(fd);

	return ret;
}

artik_error os_spi_release(artik_spi_config *config)
{
	/* Nothing to do here */
	return S_OK;
}

artik_error os_spi_read(artik_spi_config *config, char *buf, int len)
{
	int fd = -1;
	char devname[SPI_DEV_MAX_LEN];
	artik_error ret = S_OK;

	if (!config)
		return E_BAD_ARGS;
	else if (config && config->mode == SPI_MODE_INVALID)
		return E_NOT_INITIALIZED;

	if (!buf)
		return E_BAD_ARGS;

	if (len <= 0)
		return E_BAD_ARGS;

	snprintf(devname, SPI_DEV_MAX_LEN, "/dev/spidev%d.%d", config->bus,
		 config->cs);

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	if (read(fd, buf, len) != len) {
		fprintf(stderr, "%s: Failed to read (%d)\n", devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

exit:
	close(fd);

	return ret;
}

artik_error os_spi_write(artik_spi_config *config, char *buf, int len)
{
	int fd = -1;
	char devname[SPI_DEV_MAX_LEN];
	artik_error ret = S_OK;
	struct spi_ioc_transfer data;

	if (!config)
		return E_BAD_ARGS;
	else if (config && config->mode == SPI_MODE_INVALID)
		return E_NOT_INITIALIZED;

	if (!buf)
		return E_BAD_ARGS;

	if (len <= 0)
		return E_BAD_ARGS;

	memset(&data, 0, sizeof(data));

	snprintf(devname, SPI_DEV_MAX_LEN, "/dev/spidev%d.%d", config->bus,
		 config->cs);

	fd = open(devname, O_SYNC | O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	data.tx_buf = (unsigned long)buf;
	data.rx_buf = (unsigned long)NULL;
	data.len    = len;


	if (ioctl(fd, SPI_IOC_MESSAGE(1), &data) < 0) {
		fprintf(stderr, "%s: Failed to write (%d)\n", devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

exit:
	close(fd);

	return ret;
}

artik_error os_spi_read_write(artik_spi_config *config, char *tx_buf,
			      char *rx_buf, int len)
{
	int fd = -1;
	char devname[SPI_DEV_MAX_LEN];
	artik_error ret = S_OK;

	struct spi_ioc_transfer data;

	if (!config)
		return E_BAD_ARGS;
	else if (config && config->mode == SPI_MODE_INVALID)
		return E_NOT_INITIALIZED;

	if (!tx_buf || !rx_buf)
		return E_BAD_ARGS;

	if (len <= 0)
		return E_BAD_ARGS;

	snprintf(devname, SPI_DEV_MAX_LEN, "/dev/spidev%d.%d", config->bus,
		 config->cs);

	fd = open(devname, O_SYNC | O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}
	memset(&data, 0, sizeof(data));

	data.tx_buf = (unsigned long)tx_buf;
	data.rx_buf = (unsigned long)rx_buf;
	data.len    = len;
	if (ioctl(fd, SPI_IOC_MESSAGE(1), &data) < 0) {
		fprintf(stderr,
		"%s: Failed to read/write operation at address 0x%04x (%d)\n",
			devname, tx_buf[0], errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

exit:
	close(fd);

	return ret;
}

