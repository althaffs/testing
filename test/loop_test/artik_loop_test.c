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

#include <artik_module.h>
#include <artik_platform.h>
#include <artik_loop.h>

static void on_timeout_callback(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *) user_data;

	fprintf(stdout, "TEST: %s triggered, exiting loop\n", __func__);

	loop->quit();
}

artik_error test_loop_timeout(void)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_error ret = S_OK;
	int id = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);
	loop->add_timeout_callback(&id, 5000, on_timeout_callback,
				   (void *)loop);
	loop->run();

	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	artik_release_api_module(loop);

	return ret;
}

static int count = 0;

static int on_periodic_callback(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *) user_data;

	count++;
	if (count == 2) {
		fprintf(stdout, "TEST: %s triggered, exiting loop\n", __func__);
		loop->quit();
		return 0;
	}

	return 1;
}

artik_error test_loop_periodic(void)
{
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	artik_error ret = S_OK;
	int id = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);
	loop->add_periodic_callback(&id, 1000, on_periodic_callback,
				   (void *)loop);
	loop->run();

	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	artik_release_api_module(loop);

	return ret;
}

int main(void)
{
	artik_error ret = S_OK;

	if (!artik_is_module_available(ARTIK_MODULE_LOOP)) {
		fprintf(stdout,
			"TEST: Loop module is not available,"\
			" skipping test...\n");
		return -1;
	}

	ret = test_loop_timeout();
	if (ret != S_OK)
		goto exit;

	ret = test_loop_periodic();

exit:
	return ((ret == S_OK) ? 0 : -1);
}
