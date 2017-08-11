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
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_cloud.h>

#define TEST_TIMEOUT_MS (10*1000)
#define TEST_WRITE_PERIODIC_MS (1*1000)
#define TEST_WRITE_LIMIT (5)
#define MAX_PARAM_LEN (128)
#define SDR_MSG_FORMAT	"{\"sdid\":\"%s\",\"data\":%s}"

static char *sdr_device_type_id = NULL;
static char *sdr_vendor_id = NULL;
static char sdr_access_token[MAX_PARAM_LEN];
static char sdr_device_id[MAX_PARAM_LEN];
static char *sdr_test_message = NULL;

static char *parse_json_object(const char *data, const char *obj)
{
	char *res = NULL;
	char prefix[256];
	char *substr = NULL;

	snprintf(prefix, 256, "\"%s\":\"", obj);

	substr = strstr(data, prefix);
	if (substr != NULL) {
		int idx = 0;

		/* Start after substring */
		substr += strlen(prefix);

		/* Count number of bytes to extract */
		while (substr[idx] != '\"')
			idx++;
		/* Copy the extracted string */
		res = strndup(substr, idx);
	}

	return res;
}

static artik_error test_cloud_sdr_registration(void)
{
	artik_cloud_module *cloud = (artik_cloud_module *)
					artik_request_api_module("cloud");
	artik_error ret = S_OK;
	char *response = NULL;
	char *reg_id = NULL;
	char *reg_nonce = NULL;
	char *reg_pin = NULL;
	char *reg_status = NULL;
	char *device_id = NULL;
	char *access_token = NULL;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	/* Start registration process */
	ret = cloud->sdr_start_registration(sdr_device_type_id, sdr_vendor_id,
								&response);
	if (ret != S_OK) {
		if (response)
			fprintf(stdout,
				"TEST: %s HTTP error, response: %s\n",
				__func__, response);
		goto exit;
	}

	if (response) {
		/* Find rid and nonce strings */
		reg_id = parse_json_object(response, "rid");
		reg_nonce = parse_json_object(response, "nonce");
		reg_pin = parse_json_object(response, "pin");
		free(response);
		response = NULL;
	}

	if (!reg_id || !reg_nonce || !reg_pin) {
		fprintf(stdout,
			"TEST: %s failed to parse result from the JSON"\
			" response\n",
			__func__);
		ret = E_HTTP_ERROR;
		goto exit;
	}

	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed (err=%d)\n", __func__, ret);
		goto exit;
	}

	fprintf(stdout, "TEST: %s Enter PIN %s\n", __func__, reg_pin);

	/* Wait for user to enter the PIN */
	while (true) {
		ret = cloud->sdr_registration_status(reg_id, &response);
		if (ret != S_OK) {
			fprintf(stdout,
				"TEST: %s failed to get status (err=%d)\n",
				__func__, ret);
			goto exit;
		}

		if (response) {
			/* Find status */
			reg_status = parse_json_object(response, "status");
			if (!reg_status) {
				fprintf(stdout,
					"TEST: %s failed to parse status from"\
					" the JSON response\n",
					__func__);
				ret = E_HTTP_ERROR;
				goto exit;
			}

			/* Check if completed */
			if (strncmp
				(reg_status, "PENDING_USER_CONFIRMATION", 128))
				break;
			free(reg_status);
			free(response);
			reg_status = NULL;
			response = NULL;
		}

		usleep(1000 * 1000);

		fprintf(stdout, ".");
		fflush(stdout);
	}

	fprintf(stdout, "\n");

	if (strncmp(reg_status, "PENDING_DEVICE_COMPLETION", 128)) {
		fprintf(stdout,
			"TEST: %s Registration failed, probably because it"\
			" expired\n",
			__func__);
		ret = E_TIMEOUT;
		goto exit;
	}

	/* Finalize the registration */
	ret = cloud->sdr_complete_registration(reg_id, reg_nonce, &response);
	if (ret != S_OK) {
		fprintf(stdout,
			"TEST: %s Complete registration failed (err=%d)\n",
			__func__, ret);
		goto exit;
	}

	if (response) {
		device_id = parse_json_object(response, "did");
		if (!device_id) {
			fprintf(stdout,
				"TEST: %s Complete registration failed,"\
				" could not parse did from JSON response\n",
				__func__);
			ret = E_TIMEOUT;
			goto exit;
		}

		access_token = parse_json_object(response, "accessToken");
		if (!access_token) {
			fprintf(stdout,
				"TEST: %s Complete registration failed,"\
				" could not parse accessToken from JSON"\
				" response\n",
					__func__);
			ret = E_TIMEOUT;
			goto exit;
		}

		fprintf(stdout, "TEST: %s Device registered with ID %s,"\
			" TOKEN %s\n", __func__, device_id, access_token);
	} else {
		fprintf(stdout,
			"TEST: %s Complete registration failed, did not"\
			" receive a response\n",
			__func__);
		ret = E_TIMEOUT;
		goto exit;
	}

	strncpy(sdr_device_id, device_id, MAX_PARAM_LEN);
	strncpy(sdr_access_token, access_token, MAX_PARAM_LEN);

	fprintf(stdout, "TEST: %s succeeded\n", __func__);

exit:
	if (response)
		free(response);

	if (device_id)
		free(device_id);

	if (reg_pin)
		free(reg_pin);

	if (reg_status)
		free(reg_status);

	if (reg_id)
		free(reg_id);

	if (reg_nonce)
		free(reg_nonce);

	artik_release_api_module(cloud);

	return ret;
}

static void on_timeout_callback(void *user_data)
{
	artik_loop_module *loop = (artik_loop_module *) user_data;

	fprintf(stdout, "TEST: %s stop scanning, exiting loop\n", __func__);

	loop->quit();
}

static int count = 0;

static int on_write_periodic_callback(void *user_data)
{
	artik_cloud_module *cloud = (artik_cloud_module *)
					artik_request_api_module("cloud");
	char *msg = NULL;
	int len = strlen(SDR_MSG_FORMAT) + MAX_PARAM_LEN +
						strlen(sdr_test_message) + 1;

	msg = (char *)malloc(len);
	if (!msg) {
		fprintf(stderr, "TEST: %s failed to allocate memory for"\
			" message string\n", __func__);
		return 0;
	}

	snprintf(msg, len, SDR_MSG_FORMAT,  sdr_device_id, sdr_test_message);

	fprintf(stdout, "writing: %s\n", msg);
	cloud->websocket_send_message(*(artik_websocket_handle *)user_data,
									msg);

	artik_release_api_module(cloud);

	count++;

	free(msg);

	if (count > TEST_WRITE_LIMIT)
		return 0;

	return 1;
}

static void websocket_sdr_receive_callback(void *user_data, void *result)
{
	char *buffer = (char *)result;

	if (buffer == NULL) {
		fprintf(stdout, "receive failed\n");
		return;
	}
	printf("received: %s\n", buffer);
	free(result);
}

static artik_error test_websocket_sdr(void)
{
	artik_error ret = S_OK;
	artik_cloud_module *cloud = (artik_cloud_module *)
					artik_request_api_module("cloud");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

	artik_websocket_handle handle;
	int timeout_id = 0;
	int write_periodic_id = 0;
	artik_ssl_config ssl_config;

	char *response = NULL;

	/* Prepare the SSL configuration */
	memset(&ssl_config, 0, sizeof(ssl_config));
	ssl_config.use_se = true;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	if (sdr_test_message == NULL)
		sdr_test_message = strndup("{\"state\":true}", MAX_PARAM_LEN);

	/* Open websocket to ARTIK Cloud and register device to receive message
	 * from cloud
	 */
	ret = cloud->websocket_open_stream(&handle, sdr_access_token,
						sdr_device_id, &ssl_config);
	if (ret != S_OK) {
		fprintf(stderr, "TEST failed, could not open Websocket (%d)\n",
									ret);
		goto exit;
	}

	ret = cloud->websocket_set_receive_callback(handle,
				websocket_sdr_receive_callback, &handle);
	if (ret != S_OK) {
		fprintf(stderr, "TEST failed, could not open Websocket (%d)\n",
									ret);
		goto exit;
	}

	ret = loop->add_timeout_callback(&timeout_id, TEST_TIMEOUT_MS,
						on_timeout_callback,
						(void *)loop);
	if (ret != S_OK) {
		fprintf(stderr, "TEST failed, could not add timeout"\
			" callback (%d)\n", ret);
		goto exit;
	}

	ret = loop->add_periodic_callback(&write_periodic_id,
			TEST_WRITE_PERIODIC_MS, on_write_periodic_callback,
			&handle);
	if (ret != S_OK) {
		fprintf(stderr, "TEST failed, could not add periodic"\
			" callback (%d)\n", ret);
		goto exit;
	}

	loop->run();

	cloud->websocket_close_stream(handle);

	fprintf(stdout, "TEST: %s finished\n", __func__);

exit:
	if (response)
		free(response);

	artik_release_api_module(cloud);
	artik_release_api_module(loop);

	return ret;
}

int main(int argc, char *argv[])
{
	int opt;
	artik_error ret = S_OK;

	if (!artik_is_module_available(ARTIK_MODULE_CLOUD)) {
		fprintf(stdout,
			"TEST: Cloud module is not available,"\
			" skipping test...\n");
		return -1;
	}

	while ((opt = getopt(argc, argv, "t:x:v:g:")) != -1) {
		switch (opt) {
		case 'x':
			sdr_device_type_id = strndup(optarg, strlen(optarg));
			break;
		case 'v':
			sdr_vendor_id = strndup(optarg, strlen(optarg));
			break;
		case 'g':
			sdr_test_message = strndup(optarg, strlen(optarg));
			break;
		default:
			printf("Usage: sdr-test [-x <device type id>]"\
				" [-v <vendor id>] [-g <sdr test message"\
				" in JSON format>]\r\n");
			return 0;
		}
	}

	ret = test_cloud_sdr_registration();
	if (ret != S_OK)
		goto exit;

	ret = test_websocket_sdr();

exit:
	if (sdr_device_type_id != NULL)
		free(sdr_device_type_id);
	if (sdr_vendor_id != NULL)
		free(sdr_vendor_id);
	if (sdr_test_message != NULL)
		free(sdr_test_message);

	return (ret == S_OK) ? 0 : -1;
}
