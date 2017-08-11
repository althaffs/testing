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

#ifndef	__OS_SERIAL_H__
#define	__OS_SERIAL_H__

#include <artik_loop.h>
#include "artik_error.h"

artik_error os_serial_request(artik_serial_config *config);
artik_error os_serial_release(artik_serial_config *config);
artik_error os_serial_read(artik_serial_config *config, unsigned char *buf,
				int *len);
artik_error os_serial_write(artik_serial_config *config,
				unsigned char * const buf, int *len);
artik_error os_serial_available_trx_bytes(artik_serial_config *config);
artik_error os_serial_change_callback(int fd, enum watch_io io,
					void *user_data);
artik_error os_serial_set_received_callback(artik_serial_config *config,
			artik_serial_callback callback, void *user_data);
artik_error os_serial_unset_received_callback(artik_serial_config *config);


#endif  /* __OS_SERIAL_H__ */
