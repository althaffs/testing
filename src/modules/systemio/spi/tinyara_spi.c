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

#include <artik_spi.h>
#include "os_spi.h"

#include <stdio.h>
#include <errno.h>
#include <tinyara/spi/spi.h>

#define _SPI_MAX_FREQUENCY	12000000
#define _SPI_READ_MASK		0x80

static struct spi_dev_s *sdev;

artik_error os_spi_request(artik_spi_config *config)
{
	struct spi_dev_s *spi;

	if (!config)
		return E_BAD_ARGS;

	if (config->bits_per_word != 8)
		return E_BAD_ARGS;

	if (config->max_speed > _SPI_MAX_FREQUENCY)
		return E_BAD_ARGS;

	spi = up_spiinitialize(config->bus);
	if (spi == NULL)
		return E_BAD_ARGS;

	SPI_LOCK(spi, TRUE);

	SPI_SETMODE(spi, config->mode);

	SPI_SETBITS(spi, config->bits_per_word);

	SPI_SETFREQUENCY(spi, config->max_speed);

	sdev = spi;
	SPI_LOCK(spi, FALSE);

	return S_OK;
}

artik_error os_spi_release(artik_spi_config *config)
{
	if (!sdev)
		return E_NOT_INITIALIZED;

	sdev = NULL;

	return S_OK;
}

artik_error os_spi_read(artik_spi_config *config, char *buf, int len)
{
	if (!sdev)
		return E_NOT_INITIALIZED;

	buf[0] |= _SPI_READ_MASK;

	SPI_LOCK(sdev, TRUE);

	SPI_SELECT(sdev, config->cs, TRUE);

	SPI_RECVBLOCK(sdev, buf, len);

	SPI_SELECT(sdev, config->cs, FALSE);

	SPI_LOCK(sdev, FALSE);
	return S_OK;
}

artik_error os_spi_write(artik_spi_config *config, char *buf, int len)
{
	if (!sdev)
		return E_NOT_INITIALIZED;

	SPI_LOCK(sdev, TRUE);

	SPI_SELECT(sdev, config->cs, TRUE);

	SPI_SNDBLOCK(sdev, buf, len);

	SPI_SELECT(sdev, config->cs, FALSE);

	SPI_LOCK(sdev, FALSE);
	return S_OK;
}

artik_error os_spi_read_write(artik_spi_config *config, char *tx_buf,
		char *rx_buf, int len)
{
	if (!sdev)
		return E_NOT_INITIALIZED;

	rx_buf[0] |= _SPI_READ_MASK;
	SPI_LOCK(sdev, TRUE);

	SPI_SELECT(sdev, config->cs, TRUE);

	SPI_SNDBLOCK(sdev, tx_buf, len);
	SPI_RECVBLOCK(sdev, rx_buf, len);

	SPI_SELECT(sdev, config->cs, FALSE);

	SPI_LOCK(sdev, FALSE);
	return S_OK;
}
