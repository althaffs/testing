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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <artik_module.h>
#include <artik_http.h>
#include <artik_websocket.h>
#include <artik_cloud.h>
#include <artik_log.h>
#include <artik_list.h>
#include <artik_loop.h>

#define ARTIK_CLOUD_URL_MAX			256
#define ARTIK_CLOUD_URL(x)			("https://api.artik.cloud"\
						"/v1.1/" x)
#define ARTIK_CLOUD_URL_MESSAGES		ARTIK_CLOUD_URL("messages")
#define ARTIK_CLOUD_URL_SELF_USER		ARTIK_CLOUD_URL("users/self")
#define ARTIK_CLOUD_URL_USER_DEVICES		ARTIK_CLOUD_URL("users/%s/"\
						"devices?count=%d&include"\
						"Properties=%s&offset=%d")
#define ARTIK_CLOUD_URL_USER_DEVICE_TYPES	ARTIK_CLOUD_URL("users/%s/"\
						"devicetypes?count=%d&include"\
						"Shared=%s&offset=%d")
#define ARTIK_CLOUD_URL_USER_APP_PROPS		ARTIK_CLOUD_URL("users/%s/"\
						"properties?aid=%s")
#define ARTIK_CLOUD_URL_GET_DEVICE		ARTIK_CLOUD_URL("devices/%s?"\
						"properties=%s")
#define ARTIK_CLOUD_URL_DELETE_DEVICE		ARTIK_CLOUD_URL("devices/%s")
#define ARTIK_CLOUD_URL_ADD_DEVICE		ARTIK_CLOUD_URL("devices")
#define ARTIK_CLOUD_ADD_DEVICE_BODY		"{\"uid\": \"%s\", \"dtid\":"\
						" \"%s\",\"name\": \"%s\", \""\
						"manifestVersionPolicy\": \""\
						"LATEST\"}"
#define ARTIK_CLOUD_URL_GET_DEVICE_TOKEN	ARTIK_CLOUD_URL("devices/%s/"\
						"tokens")
#define ARTIK_CLOUD_URL_UPDATE_DEVICE_TOKEN	ARTIK_CLOUD_URL("devices/%s/"\
						"tokens")
#define ARTIK_CLOUD_URL_DELETE_DEVICE_TOKEN	ARTIK_CLOUD_URL("devices/%s/"\
						"tokens")

#define ARTIK_CLOUD_SECURE_URL(x)		("https://s-api.artik.cloud/"\
						"v1.1/" x)
#define ARTIK_CLOUD_SECURE_URL_REG_DEVICE	ARTIK_CLOUD_SECURE_URL("cert/"\
						"devices/registrations")
#define ARTIK_CLOUD_SECURE_URL_REG_ID		ARTIK_CLOUD_SECURE_URL("cert/"\
						"devices/registrations/%s")
#define ARTIK_CLOUD_SECURE_URL_REG_STATUS	ARTIK_CLOUD_SECURE_URL("cert/"\
						"devices/registrations/%s/"\
						"status")
#define ARTIK_CLOUD_MESSAGE_BODY		"{\"type\": \"message\",\""\
						"sdid\": \"%s\",\"data\": %s}"
#define ARTIK_CLOUD_ACTION_BODY			"{\"type\": \"action\",\""\
						"ddid\": \"%s\",\"data\": %s}"
#define ARTIK_CLOUD_SECURE_REG_DEVICE_BODY	"{\"deviceTypeId\":\"%s\",\""\
						"vendorDeviceId\":\"%s\"}"
#define ARTIK_CLOUD_SECURE_REG_COMPLETE_BODY	"{\"nonce\":\"%s\"}"
#define ARTIK_CLOUD_TOKEN_MAX			128
#define ARTIK_CLOUD_DTID_MAX			64
#define ARTIK_CLOUD_VDID_MAX			64
#define ARTIK_CLOUD_RID_MAX			64
#define ARTIK_CLOUD_NONCE_MAX			64

#define ARTIK_CLOUD_WEBSOCKET_STR_MAX		1024
#define ARTIK_CLOUD_WEBSOCKET_HOST		"api.artik.cloud"
#define ARTIK_CLOUD_WEBSOCKET_PATH		"/v1.1/websocket?ack=true"
#define ARTIK_CLOUD_WEBSOCKET_MESSAGE_BODY	"{\"sdid\":\"%s\",\""\
						"Authorization\":\"bearer "\
						"%s\",\"type\":\"%s\"}"
#define ARTIK_CLOUD_WEBSOCKET_SEND_MESSAGE_BODY	"{\"sdid\":\"%s\",\"type"\
						"\":\"%s\",\"data\":%s}"
#define ARTIK_CLOUD_WEBSOCKET_PORT		443
#define ARTIK_CLOUD_SECURE_WEBSOCKET_HOST	"s-api.artik.cloud"

#define ARRAY_SIZE(a)				(sizeof(a) / sizeof((a)[0]))


typedef struct artik_cloud_reg_data_t {
	artik_websocket_handle handle;
	char registration_message[ARTIK_CLOUD_WEBSOCKET_STR_MAX];
	char *access_token;
	char *device_id;
	artik_websocket_callback callback;
	void *callback_data;
} artik_cloud_reg_data;

typedef struct {
	artik_list node;
	artik_cloud_reg_data data;
} cloud_node;

static artik_list *requested_node = NULL;

static artik_error send_message(const char *access_token, const char *device_id,
	const char *message, char **response,
	artik_ssl_config *ssl_config);
static artik_error send_action(const char *access_token, const char *device_id,
	const char *action, char **response,
	artik_ssl_config *ssl_config);
static artik_error get_current_user_profile(const char *access_token,
	char **response,
	artik_ssl_config *ssl_config);
static artik_error get_user_devices(const char *access_token, int count,
	bool properties, int offset,
	const char *user_id, char **response,
	artik_ssl_config *ssl_config);
static artik_error get_user_device_types(const char *access_token, int count,
	bool shared, int offset,
	const char *user_id, char **response,
	artik_ssl_config *ssl_config);
static artik_error get_user_application_properties(const char *access_token,
	const char *user_id,
	const char *app_id,
	char **response,
	artik_ssl_config *ssl_config);
static artik_error get_device(const char *access_token, const char *device_id,
	bool properties, char **response,
	artik_ssl_config *ssl_config);
static artik_error get_device_token(const char *access_token,
	const char *device_id, char **response,
	artik_ssl_config *ssl_config);
static artik_error add_device(const char *access_token,
	const char *user_id, const char *dt_id,
	const char *name, char **response,
	artik_ssl_config *ssl_config);
static artik_error update_device_token(const char *access_token,
	const char *device_id, char **response,
	artik_ssl_config *ssl_config);
static artik_error delete_device_token(const char *access_token,
	const char *device_id, char **response,
	artik_ssl_config *ssl_config);
static artik_error delete_device(const char *access_token,
	const char *device_id, char **response,
	artik_ssl_config *ssl_config);
static artik_error sdr_start_registration(const char *device_type_id,
	const char *vendor_id,
	char **response);
static artik_error sdr_registration_status(const char *reg_id, char **response);
static artik_error sdr_complete_registration(const char *reg_id,
	const char *reg_nonce,
	char **response);
static artik_error websocket_open_stream(artik_websocket_handle *handle,
	const char *access_token,
	const char *device_id,
	artik_ssl_config *ssl_config);
static artik_error websocket_send_message(artik_websocket_handle handle,
	char *message);
static artik_error websocket_set_receive_callback(artik_websocket_handle handle,
	artik_websocket_callback callback,
	void *user_data);
static artik_error websocket_set_connection_callback(
	artik_websocket_handle handle,
	artik_websocket_callback callback,
	void *user_data);
static artik_error websocket_close_stream(artik_websocket_handle handle);

const artik_cloud_module cloud_module = {
	send_message,
	send_action,
	get_current_user_profile,
	get_user_devices,
	get_user_device_types,
	get_user_application_properties,
	get_device,
	get_device_token,
	add_device,
	update_device_token,
	delete_device_token,
	delete_device,
	sdr_start_registration,
	sdr_registration_status,
	sdr_complete_registration,
	websocket_open_stream,
	websocket_send_message,
	websocket_set_receive_callback,
	websocket_set_connection_callback,
	websocket_close_stream
};


artik_error get_current_user_profile(const char *access_token, char **response,
	artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Perform the request */
	ret =
	    http->get(ARTIK_CLOUD_URL_SELF_USER, &headers, response, &status,
		      ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200)
		ret = E_HTTP_ERROR;

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error get_user_devices(const char *access_token, int count,
			     bool properties, int offset, const char *user_id,
			     char **response, artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !user_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_USER_DEVICES,
		 user_id, count, properties ? "true" : "false", offset);

	/* Perform the request */
	ret = http->get(url, &headers, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_dbg("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error get_user_device_types(const char *access_token, int count,
				  bool shared, int offset, const char *user_id,
				  char **response, artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !user_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_USER_DEVICE_TYPES,
		 user_id, count, shared ? "true" : "false", offset);

	/* Perform the request */
	ret = http->get(url, &headers, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error get_user_application_properties(const char *access_token,
					    const char *user_id,
					    const char *app_id, char **response,
					    artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !user_id || !app_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_USER_APP_PROPS,
		 user_id, app_id);

	/* Perform the request */
	ret = http->get(url, &headers, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error get_device(const char *access_token, const char *device_id,
		       bool properties, char **response,
		       artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !device_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_GET_DEVICE,
		 device_id, properties ? "true" : "false");

	/* Perform the request */
	ret = http->get(url, &headers, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error get_device_token(const char *access_token, const char *device_id,
			     char **response, artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !device_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_GET_DEVICE_TOKEN,
		 device_id);

	/* Perform the request */
	ret = http->get(url, &headers, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error add_device(const char *access_token, const char *user_id,
			const char *dt_id, const char *name, char **response,
			artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	char *body = NULL;
	int body_len;
	int status;
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !user_id || !dt_id || !name) {
		log_err("Bad arguments\n");
		return E_BAD_ARGS;
	}

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up message body */
	body_len =
	    strlen(ARTIK_CLOUD_ADD_DEVICE_BODY) + strlen(user_id) +
	    strlen(dt_id) + strlen(name) + 1;

	body = (char *)malloc(body_len);
	if (!body) {
		log_err("Failed to allocate memory");
		return E_NO_MEM;
	}

	snprintf(body, body_len, ARTIK_CLOUD_ADD_DEVICE_BODY, user_id, dt_id,
		name);

	/* Perform the request */
	ret = http->post(ARTIK_CLOUD_URL_ADD_DEVICE, &headers, body, response,
		&status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	free(body);

	artik_release_api_module(http);

	return ret;
}

artik_error send_message(const char *access_token, const char *device_id,
			const char *message, char **response,
			artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	char *body = NULL;
	int body_len;
	int status;
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !device_id || !message) {
		log_err("Bad arguments\n");
		return E_BAD_ARGS;
	}

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up message body */
	body_len =
	    strlen(ARTIK_CLOUD_MESSAGE_BODY) + strlen(device_id) +
	    strlen(message) + 1;

	body = (char *)malloc(body_len);
	if (!body) {
		log_err("Failed to allocate memory");
		return E_NO_MEM;
	}

	snprintf(body, body_len, ARTIK_CLOUD_MESSAGE_BODY, device_id, message);

	/* Perform the request */
	ret =
	    http->post(ARTIK_CLOUD_URL_MESSAGES, &headers, body, response,
		       &status, ssl_config);
	if (ret != S_OK) {
		log_err("POST request failed (err=%d)", ret);
		goto exit;
	}

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	free(body);

	artik_release_api_module(http);

	return ret;
}

artik_error send_action(const char *access_token, const char *device_id,
			const char *action, char **response,
			artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	char *body = NULL;
	int body_len;
	int status;
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !device_id || !action)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up message body */
	body_len =
	    strlen(ARTIK_CLOUD_ACTION_BODY) + strlen(device_id) +
	    strlen(action) + 1;
	body = (char *)malloc(body_len);
	if (!body)
		return E_NO_MEM;

	snprintf(body, body_len, ARTIK_CLOUD_ACTION_BODY, device_id, action);

	/* Perform the request */
	ret =
	    http->post(ARTIK_CLOUD_URL_MESSAGES, &headers, body, response,
		       &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	free(body);

	artik_release_api_module(http);

	return ret;
}

artik_error update_device_token(const char *access_token, const char *device_id,
				char **response, artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	char body[] = "{}";
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !device_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_UPDATE_DEVICE_TOKEN,
		 device_id);

	/* Perform the request */
	ret = http->put(url, &headers, body, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error delete_device_token(const char *access_token, const char *device_id,
				char **response, artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !device_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_DELETE_DEVICE_TOKEN,
		 device_id);

	/* Perform the request */
	ret = http->del(url, &headers, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

artik_error delete_device(const char *access_token, const char *device_id,
				char **response, artik_ssl_config *ssl_config)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	char url[ARTIK_CLOUD_URL_MAX];
	char bearer[ARTIK_CLOUD_TOKEN_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Authorization", NULL},
		{"Content-Type", "application/json"},
	};

	log_dbg("");

	if (!access_token || !device_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up authorization header */
	snprintf(bearer, ARTIK_CLOUD_TOKEN_MAX, "Bearer %s", access_token);
	fields[0].data = bearer;

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_URL_DELETE_DEVICE,
		 device_id);

	/* Perform the request */
	ret = http->del(url, &headers, response, &status, ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

static artik_error sdr_start_registration(const char *device_type_id,
					  const char *vendor_id,
					  char **response)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	char *body = NULL;
	int body_len = 0;
	int status = 0;
	artik_ssl_config ssl_config;
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Accept", "application/json"},
		{"Content-Type", "application/json"},
		{"charsets:", "utf-8"},
	};

	log_dbg("");

	if (!device_type_id || !vendor_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up message body */
	body_len =
	    strlen(ARTIK_CLOUD_SECURE_REG_DEVICE_BODY) +
	    strlen(device_type_id) + strlen(vendor_id) + 1;
	body = (char *)malloc(body_len);
	if (!body)
		return E_NO_MEM;

	snprintf(body, body_len, ARTIK_CLOUD_SECURE_REG_DEVICE_BODY,
		 device_type_id, vendor_id);

	/* Prepare the SSL configuration */
	memset(&ssl_config, 0, sizeof(ssl_config));
	ssl_config.verify_cert = ARTIK_SSL_VERIFY_NONE;
	ssl_config.use_se = true;

	/* Perform the request */
	ret =
	    http->post(ARTIK_CLOUD_SECURE_URL_REG_DEVICE, &headers, body,
		       response, &status, &ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = (status == 409) ? E_BUSY : E_HTTP_ERROR;
	}

exit:
	free(body);

	artik_release_api_module(http);

	return ret;
}

static artik_error sdr_registration_status(const char *reg_id, char **response)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	int status;
	artik_ssl_config ssl_config;
	char url[ARTIK_CLOUD_URL_MAX];
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Accept", "application/json"},
	};

	log_dbg("");

	if (!reg_id)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_SECURE_URL_REG_STATUS,
		 reg_id);

	/* Prepare the SSL configuration */
	memset(&ssl_config, 0, sizeof(ssl_config));
	ssl_config.verify_cert = ARTIK_SSL_VERIFY_NONE;
	ssl_config.use_se = true;

	/* Perform the request */
	ret = http->get(url, &headers, response, &status, &ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	artik_release_api_module(http);

	return ret;
}

static artik_error sdr_complete_registration(const char *reg_id,
					     const char *reg_nonce,
					     char **response)
{
	artik_http_module *http = (artik_http_module *)
					artik_request_api_module("http");
	artik_error ret = S_OK;
	char url[ARTIK_CLOUD_URL_MAX];
	char *body = NULL;
	int body_len;
	int status;
	artik_ssl_config ssl_config;
	artik_http_headers headers;
	artik_http_header_field fields[] = {
		{"Accept", "application/json"},
		{"Content-Type", "application/json"},
		{"charsets:", "utf-8"}
	};

	log_dbg("");

	if (!reg_id || !reg_nonce)
		return E_BAD_ARGS;

	headers.fields = fields;
	headers.num_fields = ARRAY_SIZE(fields);

	/* Build up url with parameters */
	snprintf(url, ARTIK_CLOUD_URL_MAX, ARTIK_CLOUD_SECURE_URL_REG_ID,
		 reg_id);

	/* Build up message body */
	body_len =
	    strlen(ARTIK_CLOUD_SECURE_REG_COMPLETE_BODY) + strlen(reg_nonce) +
	    1;
	body = (char *)malloc(body_len);
	if (!body)
		return E_NO_MEM;

	snprintf(body, body_len, ARTIK_CLOUD_SECURE_REG_COMPLETE_BODY,
		 reg_nonce);

	/* Prepare the SSL configuration */
	memset(&ssl_config, 0, sizeof(ssl_config));
	ssl_config.verify_cert = ARTIK_SSL_VERIFY_NONE;
	ssl_config.use_se = true;

	/* Perform the request */
	ret = http->put(url, &headers, body, response, &status, &ssl_config);
	if (ret != S_OK)
		goto exit;

	/* Check HTTP status code */
	if (status != 200) {
		log_err("HTTP error %d", status);
		ret = E_HTTP_ERROR;
	}

exit:
	free(body);

	artik_release_api_module(http);

	return ret;
}

void websocket_connection_callback(void *user_data, void *result)
{
	artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");
	artik_cloud_reg_data *data = (artik_cloud_reg_data *)user_data;
	artik_websocket_handle handle = data->handle;

	log_dbg("");

	if (!websocket)
		return;

	switch ((intptr_t) result) {
	case ARTIK_WEBSOCKET_CONNECTED:
		websocket->websocket_write_stream(handle,
						data->registration_message);
		if (data->callback)
			data->callback(data->callback_data,
				(void *)ARTIK_WEBSOCKET_CONNECTED);
		break;
	case ARTIK_WEBSOCKET_CLOSED:
		log_dbg("Connection closed");
		if (data->callback)
			data->callback(data->callback_data,
				(void *)ARTIK_WEBSOCKET_CLOSED);
		break;
	case ARTIK_WEBSOCKET_HANDSHAKE_ERROR:
		log_dbg("Handshake error");
		if (data->callback)
			data->callback(data->callback_data,
				(void *)ARTIK_WEBSOCKET_HANDSHAKE_ERROR);
		break;
	default:
		break;
	}

	artik_release_api_module(websocket);
}

artik_error websocket_open_stream(artik_websocket_handle *handle,
				  const char *access_token,
				  const char *device_id,
				  artik_ssl_config *ssl_config)
{
	artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");
	artik_error ret = S_OK;
	artik_websocket_config config;
	cloud_node *node;
	char port[4];
	char *host = ssl_config->use_se ? ARTIK_CLOUD_SECURE_WEBSOCKET_HOST :
						ARTIK_CLOUD_WEBSOCKET_HOST;
	char *path = ARTIK_CLOUD_WEBSOCKET_PATH;

	log_dbg("");

	if (!websocket)
		return E_NOT_SUPPORTED;

	memset(&config, 0, sizeof(artik_websocket_config));

	snprintf(port, 4, "%d", ARTIK_CLOUD_WEBSOCKET_PORT);

	int len = 6 + strlen(host) + 1 + strlen(port) + strlen(path) + 1;

	config.uri = malloc(len);

	snprintf(config.uri, len, "wss://%s:%s%s", host, port, path);

	config.ssl_config = *ssl_config;

	ret = websocket->websocket_request(handle, &config);
	if (ret != S_OK)
		goto exit;

	ret = websocket->websocket_open_stream(*handle);
	if (ret != S_OK)
		goto exit;

	node = (cloud_node *)artik_list_add(&requested_node,
				(ARTIK_LIST_HANDLE)*handle, sizeof(cloud_node));
	if (!node)
		return E_NO_MEM;

	node->data.access_token = strndup(access_token, strlen(access_token));
	node->data.device_id = strndup(device_id, strlen(device_id));

	snprintf(node->data.registration_message, ARTIK_CLOUD_WEBSOCKET_STR_MAX,
		 ARTIK_CLOUD_WEBSOCKET_MESSAGE_BODY, device_id, access_token,
		 "register");

	node->data.handle = *handle;

	ret = websocket->websocket_set_connection_callback(*handle,
			websocket_connection_callback, (void *)&(node->data));
	if (ret != S_OK) {
		artik_list_delete_handle(&requested_node,
						(ARTIK_LIST_HANDLE)*handle);
		goto exit;
	}

exit:
	artik_release_api_module(websocket);

	return ret;
}


artik_error websocket_send_message(artik_websocket_handle handle, char *message)
{
	artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");
	artik_error ret = S_OK;
	cloud_node *node = (cloud_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE)handle);
	char message_buffer[ARTIK_CLOUD_WEBSOCKET_STR_MAX] = {0, };

	log_dbg("");

	if (!websocket)
		return E_NOT_SUPPORTED;

	if (!node)
		return E_BAD_ARGS;

	snprintf(message_buffer, ARTIK_CLOUD_WEBSOCKET_STR_MAX,
		ARTIK_CLOUD_WEBSOCKET_SEND_MESSAGE_BODY, node->data.device_id,
		"message", message);

	ret = websocket->websocket_write_stream(handle, message_buffer);

	artik_release_api_module(websocket);

	return ret;
}

artik_error websocket_set_connection_callback(artik_websocket_handle handle,
			artik_websocket_callback callback, void *user_data)
{
	cloud_node *node = (cloud_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE)handle);

	log_dbg("");

	if (!node)
		return E_BAD_ARGS;

	node->data.callback = callback;
	node->data.callback_data = user_data;

	return S_OK;
}

artik_error websocket_set_receive_callback(artik_websocket_handle handle,
			artik_websocket_callback callback, void *user_data)
{
	artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");
	artik_error ret = S_OK;

	log_dbg("");

	if (!websocket)
		return E_NOT_SUPPORTED;

	ret = websocket->websocket_set_receive_callback(handle, callback,
							user_data);

	artik_release_api_module(websocket);

	return ret;
}

artik_error websocket_close_stream(artik_websocket_handle handle)
{
	artik_websocket_module *websocket = (artik_websocket_module *)
					artik_request_api_module("websocket");
	artik_error ret = S_OK;
	cloud_node *node = (cloud_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE)handle);

	log_dbg("");

	if (!websocket)
		return E_NOT_SUPPORTED;

	if (!node)
		return E_BAD_ARGS;

	if (node->data.access_token)
		free(node->data.access_token);

	if (node->data.device_id)
		free(node->data.device_id);

	ret = artik_list_delete_handle(&requested_node,
					(ARTIK_LIST_HANDLE)handle);
	if (ret != S_OK)
		goto exit;

	ret = websocket->websocket_close_stream(handle);

exit:
	artik_release_api_module(websocket);

	return ret;
}
