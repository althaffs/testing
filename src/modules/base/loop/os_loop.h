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

#ifndef _OS_LOOP_H_
#define _OS_LOOP_H_

#include "artik_error.h"

void	os_loop_run(void);
void	os_loop_quit(void);
artik_error os_add_timeout_callback(int *timeout_id, unsigned int msec,
					timeout_callback func, void *user_data);
artik_error os_remove_timeout_callback(int timeout_id);
artik_error os_add_periodic_callback(int *periodic_id, unsigned int msec,
		periodic_callback func, void *user_data);
artik_error os_remove_periodic_callback(int periodic_id);
artik_error os_add_fd_watch(int fd, enum watch_io io, watch_callback func,
				void *user_data, int *watch_id);
artik_error os_remove_fd_watch(int watch_id);
artik_error os_add_signal_watch(int signum, signal_callback func,
		void *user_data, int *watch_id);
artik_error os_remove_signal_watch(int signal_id);
artik_error os_add_idle_callback(int *idle_id, idle_callback func,
				void *user_data);
artik_error os_remove_idle_callback(int idle_id);

#endif /* _OS_LOOP_H_ */
