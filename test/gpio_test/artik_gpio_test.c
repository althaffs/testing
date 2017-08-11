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
#include <unistd.h>
#include <signal.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_platform.h>
#include <artik_gpio.h>

enum {
	R = 0,
	G,
	B
};

struct led_gpios {
	artik_gpio_handle handle;
	artik_gpio_config config;
};

static artik_error test_rgb_led(int platid)
{
	artik_gpio_module *gpio = (artik_gpio_module *)
					artik_request_api_module("gpio");
	unsigned int i;
	artik_error ret = S_OK;

	struct led_gpios leds[] = {
		{ NULL, { ARTIK_A520_GPIO_XEINT0, "red", GPIO_OUT,
						GPIO_EDGE_NONE, 0, NULL }
		},	/* Connected to J26-2 */
		{ NULL, { ARTIK_A520_GPIO_XEINT1, "green", GPIO_OUT,
						GPIO_EDGE_NONE, 0, NULL }
		},	/* Connected to J26-3 */
		{ NULL, { ARTIK_A520_GPIO_XEINT2, "blue", GPIO_OUT,
						GPIO_EDGE_NONE, 0, NULL }
		}	/* Connected to J26-4 */
	};

	if (platid == ARTIK520) {
		leds[0].config.id = ARTIK_A520_GPIO_XEINT0;
		leds[1].config.id = ARTIK_A520_GPIO_XEINT1;
		leds[2].config.id = ARTIK_A520_GPIO_XEINT2;
	} else if (platid == ARTIK1020) {
		leds[0].config.id = ARTIK_A1020_GPIO_XEINT0;
		leds[1].config.id = ARTIK_A1020_GPIO_XEINT1;
		leds[2].config.id = ARTIK_A1020_GPIO_XEINT2;
	} else if (platid == ARTIK710) {
		leds[0].config.id = ARTIK_A710_GPIO0;
		leds[1].config.id = ARTIK_A710_GPIO1;
		leds[2].config.id = ARTIK_A710_GPIO2;
	} else {
		leds[0].config.id = ARTIK_A530_GPIO0;
		leds[1].config.id = ARTIK_A530_GPIO2;
		leds[2].config.id = ARTIK_A530_GPIO3;
	}

	fprintf(stdout, "TEST: %s\n", __func__);

	/* Register GPIOs for LEDs */
	for (i = 0; i < (sizeof(leds) / sizeof(*leds)); i++) {
		ret = gpio->request(&leds[i].handle, &leds[i].config);
		if (ret != S_OK)
			return ret;
	}

	/* Play around with all possible colors */
	gpio->write(leds[R].handle, 0);	/* R */
	gpio->write(leds[G].handle, 0);	/* G */
	gpio->write(leds[B].handle, 0);	/* B */
	usleep(1000 * 1000);

	gpio->write(leds[R].handle, 1);	/* R */
	usleep(1000 * 1000);
	gpio->write(leds[G].handle, 1);	/* RG */
	usleep(1000 * 1000);
	gpio->write(leds[B].handle, 1);	/* RGB */
	usleep(1000 * 1000);
	gpio->write(leds[R].handle, 0);	/* GB */
	usleep(1000 * 1000);
	gpio->write(leds[B].handle, 0);	/* G */
	usleep(1000 * 1000);
	gpio->write(leds[G].handle, 0);
	usleep(1000 * 1000);
	gpio->write(leds[B].handle, 1);	/* B */
	usleep(1000 * 1000);
	gpio->write(leds[R].handle, 1);	/* RB */
	usleep(1000 * 1000);
	gpio->write(leds[G].handle, 1);	/* RBG */
	usleep(1000 * 1000);

	/* Release GPIOs for LEDs */
	for (i = 0; i < (sizeof(leds) / sizeof(*leds)); i++)
		gpio->release(leds[i].handle);

	ret = artik_release_api_module(gpio);
	if (ret != S_OK) {
		fprintf(stderr, "TEST: %s failed, could not release module\n",
								__func__);
		return ret;
	}

	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	return ret;
}

static void button_event(void *param, int value)
{
	static int count = 10;

	fprintf(stdout, "Button event: %d\n", value);

	if (--count < 0) {
		artik_gpio_module *gpio = (artik_gpio_module *)
					artik_request_api_module("gpio");
		artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
		artik_gpio_handle button = (artik_gpio_handle)param;

		gpio->unset_change_callback(button);
		loop->quit();

		artik_release_api_module(gpio);
		artik_release_api_module(loop);
	}
}

static artik_error test_button_interrupt(int platid)
{
	artik_gpio_module *gpio = (artik_gpio_module *)
					artik_request_api_module("gpio");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_error ret = S_OK;
	artik_gpio_handle button;
	artik_gpio_config config;

	if (platid == ARTIK520)
		config.id = ARTIK_A520_GPIO_XEINT3;
	else if (platid == ARTIK1020)
		config.id = ARTIK_A1020_GPIO_XEINT3;
	else if (platid == ARTIK710)
		config.id = ARTIK_A710_GPIO4;
	else
		config.id = ARTIK_A530_GPIO4;

	config.name = "button";
	config.dir = GPIO_IN;
	config.edge = GPIO_EDGE_BOTH;
	config.initial_value = 0;

	fprintf(stdout, "TEST: %s\n", __func__);

	ret = gpio->request(&button, &config);
	if (ret != S_OK) {
		fprintf(stderr, "TEST: %s failed, could not request"\
			" GPIO (%d)\n", __func__, ret);
		return ret;
	}

	ret = gpio->set_change_callback(button, button_event, (void *)button);
	if (ret != S_OK) {
		fprintf(stderr, "TEST: %s failed, could not set GPIO change"\
			" callback (%d)\n", __func__, ret);
		goto exit;
	}

	loop->run();

exit:
	gpio->unset_change_callback(button);
	gpio->release(button);

	fprintf(stdout, "TEST: %s %s\n", __func__, ret == S_OK ? "succeeded" :
								"failed");

	artik_release_api_module(gpio);

	return ret;
}

int main(void)
{
	artik_error ret = S_OK;
	int platid = artik_get_platform();

	if ((platid == ARTIK520) || (platid == ARTIK1020) ||
			(platid == ARTIK710) || (platid == ARTIK530)) {
		ret = test_button_interrupt(platid);
		if (ret != S_OK)
			goto exit;

		ret = test_rgb_led(platid);
	}

exit:
	return (ret == S_OK) ? 0 : -1;
}
