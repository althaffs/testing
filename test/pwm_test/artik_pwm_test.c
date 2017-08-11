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
#include <string.h>
#include <unistd.h>

#include <artik_module.h>
#include <artik_platform.h>
#include <artik_pwm.h>

static artik_pwm_config config = {
	1,
	"pwm",
	400000,
	200000,
	ARTIK_PWM_POLR_NORMAL,
	NULL
};

static artik_error pwm_test_frequency(int platid)
{
	artik_pwm_handle handle;
	artik_error ret = S_OK;
	artik_pwm_module *pwm = (artik_pwm_module *)
						artik_request_api_module("pwm");

	if (platid == ARTIK520)
		config.pin_num = ARTIK_A520_PWM1;
	else if (platid == ARTIK1020)
		config.pin_num = ARTIK_A1020_PWM1;
	else if (platid == ARTIK710)
		config.pin_num = ARTIK_A710_PWM0;
	else
		config.pin_num = ARTIK_A530_PWM0;

	fprintf(stdout, "TEST: %s\n", __func__);

	ret = pwm->request(&handle, &config);
	if (ret != S_OK) {
		fprintf(stderr, "TEST: %s - Failed to request PWM (err=%d)\n",
								__func__, ret);
		return ret;
	}

	/* Wait for X seconds */
	usleep(3 * 1000 * 1000);

	pwm->release(handle);

	fprintf(stdout, "TEST: %s succeeded\n", __func__);

	artik_release_api_module(pwm);

	return ret;
}

int main(void)
{
	artik_error ret = S_OK;
	int platid = artik_get_platform();

	if ((platid == ARTIK520) || (platid == ARTIK1020)  ||
		(platid == ARTIK710) || (platid == ARTIK530))
		ret = pwm_test_frequency(platid);

	return (ret == S_OK) ? 0 : -1;
}
