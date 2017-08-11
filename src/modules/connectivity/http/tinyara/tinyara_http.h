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

#ifndef __TINYARA_HTTP_H__
#define __TINYARA_HTTP_H__

typedef void (*wget_callback_stream_t)(FAR char **buffer, int offset,
				int datend, FAR int *buflen, FAR void *arg);

int wget(FAR const char *url, FAR char *buffer, int buflen,
		wget_callback_stream_t callback, FAR void *arg, int with_tls,
		void *tls_conf);

#endif /* __TINYARA_HTTP_H__ */
