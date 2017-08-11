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

/* Aritk SPI Physical Loopback Test
 * Connect the wire between MISO and MOSI
 */

#include <stdio.h>

#include <artik_module.h>
#include <artik_platform.h>
#include <artik_spi.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static artik_spi_config config = {
	1,
	0,
	SPI_MODE0,
	8,
	500000,
};

static artik_error spi_test(int platid)
{
	artik_spi_module *spi = (artik_spi_module *)
						artik_request_api_module("spi");
	artik_spi_handle handle;
	artik_error ret;
	unsigned int i = 0;

	unsigned char tx[] = {
		0x12, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
		0xF0, 0x0D,
	};
	unsigned char rx[ARRAY_SIZE(tx)] = {0, };


	fprintf(stdout, "TEST: %s starting\n", __func__);

    /* Do platform specific configuration */
	if (ARTIK710 == platid || ARTIK530 == platid)
		config.bus = 2;

	ret = spi->request(&handle, &config);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to request SPI %d\n", ret);
		goto exit;
	}
	/* Send Transfer command */
	ret = spi->read_write(handle, (char *)tx, (char *) rx, ARRAY_SIZE(tx));
	if (ret != S_OK) {
		fprintf(stderr, "Failed to write SPI %d\n", ret);
		goto exit;
	}

	/* Compare the result */
	for (i = 0; i < ARRAY_SIZE(tx); i++) {
		fprintf(stdout, "Comparing %d: %.2X %.2X\n", i, tx[i], rx[i]);
		if (tx[i] != rx[i]) {
			ret = E_TRY_AGAIN;
			goto exit;
		}
	}

	ret = spi->release(handle);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to release spidev%d.%d (%d)\n",
			config.bus, config.cs, ret);
		goto exit;
	}
exit:
	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	artik_release_api_module(spi);

	return ret;
}

int main(void)
{
	artik_error ret = S_OK;
	int platid = artik_get_platform();

	ret = spi_test(platid);

	return (ret == S_OK) ? 0 : -1;
}
