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

#include <signal.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

#include <artik_module.h>
#include <artik_platform.h>
#include <artik_serial.h>
#include <artik_loop.h>

/*
 * This is a loopback test. On Artik 520 development platform,
 * connect a wire between "TX" and "RX" pins
 * on connector J26.
 */
static artik_serial_config config = {
	ARTIK_A520_SCOM_XSCOM4,
	"UART3",
	ARTIK_SERIAL_BAUD_115200,
	ARTIK_SERIAL_PARITY_NONE,
	ARTIK_SERIAL_DATA_8BIT,
	ARTIK_SERIAL_STOP_1BIT,
	ARTIK_SERIAL_FLOWCTRL_NONE,
	NULL
};

static artik_serial_handle handle = NULL;
static const char *current_test = NULL;
#define MAX_RX_BUF	64

static void signal_handler(int signum)
{
	if (signum == SIGALRM) {
		fprintf(stderr, "TEST: %s failed, timeout expired\n",
			current_test);
		exit(-1);
	}
}

static void set_timeout(const char *test_name, unsigned int seconds)
{
	current_test = test_name;
	signal(SIGALRM, signal_handler);
	alarm(seconds);
}

static void unset_timeout(void)
{
	alarm(0);
	signal(SIGALRM, SIG_DFL);
}

static void forward_data(void *param, unsigned char *buf, int len)
{
	if (buf != NULL)
		fprintf(stdout, "Forward read: %s\n", buf);
	else {
		artik_serial_module *serial = (artik_serial_module *)
					artik_request_api_module("serial");
		artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

		serial->unset_received_callback(handle);
		loop->quit();

		artik_release_api_module(serial);
		artik_release_api_module(loop);
	}
}

static artik_error test_serial_loopback(int platid)
{
	artik_serial_module *serial = (artik_serial_module *)
					artik_request_api_module("serial");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_error ret = S_OK;
	char tx_buf[] = "This is a test buffer containing test data\0";
	int tx_len = strlen(tx_buf);
	char buff[128] = "";
	int len = 0;
	int maxlen = 0;

	if (platid == ARTIK520) {
		config.port_num = ARTIK_A520_SCOM_XSCOM4;
		config.name = "UART3";
	} else if (platid == ARTIK1020) {
		config.port_num = ARTIK_A1020_SCOM_XSCOM2;
		config.name = "UART1";
	} else if (platid == ARTIK710) {
		config.port_num = ARTIK_A710_UART0;
		config.name = "UART4";
	} else {
		config.port_num = ARTIK_A530_UART0;
		config.name = "UART4";
	}

	fprintf(stdout, "TEST: %s\n", __func__);

	serial->request(&handle, &config);
	if (ret != S_OK) {
		fprintf(stderr, "TEST: %s failed to request serial port (%d)\n",
			__func__, ret);
		return ret;
	}

	set_timeout(__func__, 5);
	/* Send test data */
	len = tx_len;
	ret = serial->write(handle, (unsigned char *)tx_buf, &len);
	if (ret != S_OK) {
		fprintf(stderr, "TEST: %s failed to send data (%d)\n", __func__,
			ret);
		goto exit;
	}

	fprintf(stdout, "TEST: with read\n");
	len = 128;
	while (maxlen < tx_len) {
		ret = serial->read(handle, (unsigned char *)buff+maxlen, &len);
		if (ret != S_OK)
			maxlen += len;
	}
	fprintf(stdout, "buff : %s\n", buff);
	fprintf(stdout, "TEST: with callback\n");
	/* Wait for read data to become available */
	char tx_bufs[] = "This is a second test\0";

	tx_len = strlen(tx_bufs);
	ret = serial->write(handle, (unsigned char *)tx_bufs, &tx_len);
	ret = serial->set_received_callback(handle, forward_data, NULL);
	loop->run();
	if (ret != S_OK) {
		fprintf(stderr, "TEST: %s failed while waiting for RX data\n"
			"(%d)\n", __func__, ret);
		goto exit;
	}
	serial->unset_received_callback(handle);
	fprintf(stdout, "TEST: %s succeeded\n", __func__);


exit:
	unset_timeout();
	serial->release(handle);

	artik_release_api_module(serial);
	artik_release_api_module(loop);
	return ret;
}

int main(void)
{
	artik_error ret = S_OK;
	int platid = artik_get_platform();

	if (!artik_is_module_available(ARTIK_MODULE_SERIAL)) {
		fprintf(stdout, "TEST: Serial module is not available,\n"
			"skipping test...\n");
		return -1;
	}

	if ((platid == ARTIK520) || (platid == ARTIK1020) ||
				(platid == ARTIK710) || (platid == ARTIK530)) {
		ret = test_serial_loopback(platid);
	}

	return (ret == S_OK) ? 0 : -1;
}
