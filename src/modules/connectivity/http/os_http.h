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

#ifndef	OS_HTTP_H_
#define	OS_HTTP_H_

#include "artik_error.h"

artik_error os_http_get_stream(const char *url, artik_http_headers * headers,
			int *status, artik_http_stream_callback callback,
			void *user_data, artik_ssl_config * ssl);
artik_error os_http_get(const char *url, artik_http_headers *headers,
			char **response, int *status, artik_ssl_config *ssl);
artik_error os_http_post(const char *url, artik_http_headers *headers,
			const char *body, char **response, int *status,
			artik_ssl_config *ssl);
artik_error os_http_put(const char *url, artik_http_headers *headers,
			const char *body, char **response, int *status,
			artik_ssl_config *ssl);
artik_error os_http_delete(const char *url, artik_http_headers *headers,
			char **response, int *status, artik_ssl_config *ssl);

#endif	/* OS_HTTP_H_ */
