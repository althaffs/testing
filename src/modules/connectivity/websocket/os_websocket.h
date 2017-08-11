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

#ifndef	OS_WEBSOCKET_H_
#define	OS_WEBSOCKET_H_

#include "artik_error.h"

artik_error os_websocket_open_stream(artik_websocket_config *config);
artik_error os_websocket_write_stream(artik_websocket_config *config,
					char *message, int len);
artik_error os_websocket_set_connection_callback(artik_websocket_config *config,
			artik_websocket_callback callback, void *user_data);
artik_error os_websocket_set_receive_callback(artik_websocket_config *config,
			artik_websocket_callback callback, void *user_data);
artik_error os_websocket_close_stream(artik_websocket_config *config);

#endif	/* OS_WEBSOCKET_H_ */
