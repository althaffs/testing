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

#include <artik_http.h>
#include "os_http.h"

static artik_error artik_http_get_stream(const char *url,
			artik_http_headers * headers,
			int *status, artik_http_stream_callback callback,
			void *user_data,
			artik_ssl_config * ssl);
static artik_error artik_http_get(const char *url, artik_http_headers *headers,
			char **response, int *status, artik_ssl_config *ssl);
static artik_error artik_http_post(const char *url, artik_http_headers *headers,
				const char *body, char **response, int *status,
				artik_ssl_config *ssl);
static artik_error artik_http_put(const char *url, artik_http_headers *headers,
				const char *body, char **response, int *status,
				artik_ssl_config *ssl);
static artik_error artik_http_delete(const char *url,
				artik_http_headers *headers, char **response,
				int *status, artik_ssl_config *ssl);

const artik_http_module http_module = {
	artik_http_get_stream,
	artik_http_get,
	artik_http_post,
	artik_http_put,
	artik_http_delete,
};

artik_error artik_http_get_stream(const char *url, artik_http_headers *headers,
			int *status, artik_http_stream_callback callback,
			void *user_data, artik_ssl_config *ssl)
{
	return os_http_get_stream(url, headers, status, callback, user_data,
									ssl);
}

artik_error artik_http_get(const char *url, artik_http_headers *headers,
			char **response, int *status, artik_ssl_config *ssl)
{
	return os_http_get(url, headers, response, status, ssl);
}

artik_error artik_http_post(const char *url, artik_http_headers *headers,
			const char *body, char **response, int *status,
							artik_ssl_config *ssl)
{
	return os_http_post(url, headers, body, response, status, ssl);
}

artik_error artik_http_put(const char *url, artik_http_headers *headers,
			const char *body, char **response, int *status,
							artik_ssl_config *ssl)
{
	return os_http_put(url, headers, body, response, status, ssl);
}

artik_error artik_http_delete(const char *url, artik_http_headers *headers,
			char **response, int *status, artik_ssl_config *ssl)
{
	return os_http_delete(url, headers, response, status, ssl);
}
