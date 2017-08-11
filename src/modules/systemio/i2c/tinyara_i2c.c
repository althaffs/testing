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

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <tinyara/i2c.h>


#include <artik_i2c.h>
#include "os_i2c.h"

#define	I2C_DEV_MAX_LEN		64

#ifndef I2C_M_READ
#define I2C_M_READ           0x0001	/* Read data, from slave to master */
#endif /* I2C_M_READ */

#ifndef I2C_M_TEN
#define I2C_M_TEN            0x0002	/* Ten bit address */
#endif /* I2C_M_TEN */

#ifndef I2C_M_NORESTART
#define I2C_M_NORESTART      0x0080	/* Message should not begin with
					 * (re-)start of transfer
					 */
#endif /* I2C_M_NORESTART */

#ifndef I2C_SLAVE
#define I2C_SLAVE            0x0703	/* Use this slave address */
#endif /* I2C_SLAVE */

#ifndef I2C_SLAVE_FORCE
#define I2C_SLAVE_FORCE      0x0706	/* Use this slave address, even if it
					 * is already in use by a driver!
					 */
#endif /* I2C_SLAVE_FORCE */

#ifndef I2C_TENBIT
#define I2C_TENBIT           0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit */
#endif /* I2C_TENBIT */

#ifndef I2C_RDWR
#define I2C_RDWR             0x0707	/* Combined R/W transfer
					 * (one STOP only)
					 */
#endif /* I2C_RDWR */

artik_error os_i2c_request(artik_i2c_config *config)
{
	struct i2c_dev_s *i2c_dev;

	i2c_dev = up_i2cinitialize(config->id);
	if (i2c_dev == NULL) {
		fprintf(stderr, "Failed to send up_i2cinitialize failed\n");
		return E_ACCESS_DENIED;
	}

	return S_OK;
}

artik_error os_i2c_release(artik_i2c_config *config)
{
	return S_OK;
}

artik_error os_i2c_read(artik_i2c_config *config, char *buf, int len)
{
	int fd;
	char devname[I2C_DEV_MAX_LEN];
	int ret;

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);
	fd = open(devname, O_SYNC | O_RDOK);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	ret = ioctl(fd, I2C_SLAVE, (unsigned long)
						((uintptr_t)&config->address));
	if (ret < 0) {
		fprintf(stderr, "Failed to set slave address to  %s (%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

	ret = read(fd, buf, len);
	if (ret < 0) {
		fprintf(stderr, "%s: Failed to read (%d)\n", devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

exit:
	close(fd);

	return S_OK;
}

artik_error os_i2c_write(artik_i2c_config *config, char *buf, int len)
{
	int fd;
	char devname[I2C_DEV_MAX_LEN];
	int ret;

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);
	fd = open(devname, O_SYNC | O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	ret = ioctl(fd, I2C_SLAVE, (unsigned long)
						((uintptr_t)&config->address));
	if (ret < 0) {
		fprintf(stderr, "Failed to set slave address to  %s (%d)\n",
			devname, errno);
		ret = E_ACCESS_DENIED;
		goto exit;
	}

	ret = write(fd, buf, len);
	if (ret < 0) {
		fprintf(stderr, "%s: Failed to write (%d)\n", devname, errno);
		ret = E_ACCESS_DENIED;
	}

exit:
	close(fd);

	return S_OK;
}

artik_error os_i2c_read_register(artik_i2c_config *config, unsigned int reg,
				char *buf, int len)
{
#ifdef CONFIG_I2C_USERIO
	int fd;
	char devname[I2C_DEV_MAX_LEN];
	int ret;

	struct i2c_rdwr_ioctl_data_s packet;
	struct i2c_msg_s msgs[2];

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);
	fd = open(devname, O_SYNC | O_RDOK);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	msgs[0].addr = config->address;
	msgs[0].buffer = (uint8_t *)&reg;
	msgs[0].flags = 0;
	msgs[0].length = config->wordsize;

	msgs[1].addr = config->address;
	msgs[1].buffer = (uint8_t *)buf;
	msgs[1].flags = I2C_M_READ;
	msgs[1].length = len * config->wordsize;

	packet.msgs = msgs;
	packet.nmsgs = 2;/* sizeof(msgs) / sizeof(msgs[0]); */

	ret = ioctl(fd, I2C_RDWR, (unsigned long)&packet);

	if (ret < 0) {
		fprintf(stderr, "%s: Failed to write (%d)\n", devname, errno);
		close(fd);
		return E_ACCESS_DENIED;
	}

	close(fd);

	return S_OK;
#else
	return E_NOT_SUPPORTED;
#endif
}

artik_error os_i2c_write_register(artik_i2c_config *config, unsigned int reg,
				char *buf, int len)
{
#ifdef CONFIG_I2C_USERIO
	int fd;
	char devname[I2C_DEV_MAX_LEN];
	int ret;

	struct i2c_rdwr_ioctl_data_s packet;
	struct i2c_msg_s msgs[2];

	snprintf(devname, I2C_DEV_MAX_LEN, "/dev/i2c-%d", config->id);
	fd = open(devname, O_SYNC | O_RDOK);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s (%d)\n", devname, errno);
		return E_ACCESS_DENIED;
	}

	msgs[0].addr = config->address;
	msgs[0].buffer = (uint8_t *)&reg;
	msgs[0].flags = 0;
	msgs[0].length = config->wordsize;

	msgs[1].addr = config->address;
	msgs[1].buffer = (uint8_t *)buf;
	msgs[1].flags = 0;
	msgs[1].length = len * config->wordsize;

	packet.msgs = msgs;
	packet.nmsgs = 2;

	ret = ioctl(fd, I2C_RDWR, (unsigned long)&packet);

	if (ret < 0) {
		fprintf(stderr, "%s: Failed to write (%d)\n", devname, errno);
		close(fd);
		return E_ACCESS_DENIED;
	}

	close(fd);

	return S_OK;
#else
	return E_NOT_SUPPORTED;
#endif
}
