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


#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <artik_i2c.h>
#include "os_i2c.h"

#define	I2C_DEV_MAX_LEN		64

artik_error os_i2c_request(artik_i2c_config *config)
{
	int fd = -1;
	char devname[I2C_DEV_MAX_LEN];
	artik_error ret = S_OK;

	/* Try to open driver and set slave address */
	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	if (ioctl(fd, I2C_SLAVE, config->address) < 0) {
		fprintf(stderr, "Failed to set slave address to  %s (%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
	}

	close(fd);

	return ret;
}

artik_error os_i2c_release(artik_i2c_config *config)
{
	/* Nothing to do here */
	return S_OK;
}

artik_error os_i2c_read(artik_i2c_config *config, char *buf, int len)
{
	int fd = -1;
	char devname[I2C_DEV_MAX_LEN];
	artik_error ret = S_OK;

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	if (ioctl(fd, I2C_SLAVE, config->address) < 0) {
		fprintf(stderr, "Failed to set slave address to  %s (%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
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

artik_error os_i2c_write(artik_i2c_config *config, char *buf, int len)
{
	int fd = -1;
	char devname[I2C_DEV_MAX_LEN];
	artik_error ret = S_OK;

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	if (ioctl(fd, I2C_SLAVE, config->address) < 0) {
		fprintf(stderr, "Failed to set slave address to  %s (%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

	if (write(fd, buf, len) != len) {
		fprintf(stderr, "%s: Failed to write (%d)\n", devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

exit:
	close(fd);

	return ret;
}

artik_error os_i2c_read_register(artik_i2c_config *config, unsigned int reg,
				 char *buf, int len)
{
	int fd = -1;
	char devname[I2C_DEV_MAX_LEN];
	artik_error ret = S_OK;
	struct i2c_rdwr_ioctl_data data;

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	if (ioctl(fd, I2C_SLAVE, config->address) < 0) {
		fprintf(stderr, "Failed to set slave address to  %s (%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

	data.nmsgs = 2;
	data.msgs = malloc(data.nmsgs * sizeof(struct i2c_msg));
	if (!data.msgs) {
		ret = -E_NO_MEM;
		goto exit;
	}

	data.msgs[0].addr = config->address;
	data.msgs[0].flags = 0;
	data.msgs[0].len = config->wordsize;
	data.msgs[0].buf = (unsigned char *)&reg;

	data.msgs[1].addr = config->address;
	data.msgs[1].flags = I2C_M_RD;
	data.msgs[1].len = len * config->wordsize;
	data.msgs[1].buf = (unsigned char *)buf;

	if (ioctl(fd, I2C_RDWR, &data) < 0) {
		fprintf(stderr,
			"%s: Failed to read register at address 0x%04x (%d)\n",
			devname, reg, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

exit:
	if (data.msgs)
		free(data.msgs);
	close(fd);
	return ret;
}

artik_error os_i2c_write_register(artik_i2c_config *config, unsigned int reg,
				  char *buf, int len)
{
	int fd = -1;
	char devname[I2C_DEV_MAX_LEN];
	artik_error ret = S_OK;
	struct i2c_rdwr_ioctl_data data;

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	if (ioctl(fd, I2C_SLAVE, config->address) < 0) {
		fprintf(stderr, "Failed to set slave address to  %s (%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

	data.nmsgs = 1;
	data.msgs = malloc(data.nmsgs * sizeof(struct i2c_msg));
	if (!data.msgs) {
		ret = -E_NO_MEM;
		goto exit;
	}

	data.msgs[0].addr = config->address;
	data.msgs[0].flags = 0;
	data.msgs[0].len = (len + 1) * config->wordsize;
	data.msgs[0].buf = malloc((len + 1) * config->wordsize);
	if (!data.msgs[0].buf) {
		ret = -E_NO_MEM;
		goto exit;
	}
	memcpy(data.msgs[0].buf, &reg, config->wordsize);
	memcpy(data.msgs[0].buf + config->wordsize, buf,
	       len * config->wordsize);

	if (ioctl(fd, I2C_RDWR, &data) < 0) {
		fprintf(stderr,
			"%s: Failed to write register at address 0x%04x (%d)\n",
			devname, reg, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

exit:
	if (data.msgs[0].buf)
		free(data.msgs[0].buf);

	if (data.msgs)
		free(data.msgs);

	close(fd);

	return ret;
}
