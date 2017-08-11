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


#include "artik_loop.h"
#include "os_loop.h"

static void		loop_run(void);
static void		loop_quit(void);
static artik_error	add_timeout_callback(int *timeout_id, unsigned int msec,
					timeout_callback func, void *user_data);
static artik_error	remove_timeout_callback(int timeout_id);
static artik_error	add_periodic_callback(int *periodic_id,
						unsigned int msec,
						periodic_callback func,
						void *user_data);
static artik_error	remove_periodic_callback(int periodic_id);
static artik_error	add_fd_watch(int fd, enum watch_io io,
					watch_callback func, void *user_data,
					int *watch_id);
static artik_error	remove_fd_watch(int watch_id);
static artik_error	add_signal_watch(int signum, signal_callback func,
					void *user_data, int *signal_id);
static artik_error	remove_signal_watch(int signal_id);
static artik_error	add_idle_callback(int *idle_id, idle_callback func,
							void *user_data);
static artik_error	remove_idle_callback(int idle_id);

EXPORT_API const artik_loop_module loop_module = {
	loop_run,
	loop_quit,
	add_timeout_callback,
	remove_timeout_callback,
	add_periodic_callback,
	remove_periodic_callback,
	add_fd_watch,
	remove_fd_watch,
	add_signal_watch,
	remove_signal_watch,
	add_idle_callback,
	remove_idle_callback
};

void loop_run(void)
{
	os_loop_run();
}

void loop_quit(void)
{
	os_loop_quit();
}

artik_error add_timeout_callback(int *timeout_id, unsigned int msec,
				 timeout_callback func, void *user_data)
{
	return os_add_timeout_callback(timeout_id, msec, func, user_data);
}

artik_error remove_timeout_callback(int timeout_id)
{
	return os_remove_timeout_callback(timeout_id);
}
artik_error add_periodic_callback(int *periodic_id, unsigned int msec,
		periodic_callback func, void *user_data)
{
	return os_add_periodic_callback(periodic_id, msec, func, user_data);
}
artik_error remove_periodic_callback(int periodic_id)
{
	return os_remove_periodic_callback(periodic_id);
}

artik_error add_fd_watch(int fd, enum watch_io io, watch_callback func,
						void *user_data, int *watch_id)
{
	return os_add_fd_watch(fd, io, func, user_data, watch_id);
}

artik_error remove_fd_watch(int watch_id)
{
	return os_remove_fd_watch(watch_id);
}

artik_error add_signal_watch(int signum, signal_callback func,
						void *user_data, int *signal_id)
{
	return os_add_signal_watch(signum, func, user_data, signal_id);
}

artik_error remove_signal_watch(int signal_id)
{
	return os_remove_signal_watch(signal_id);
}

artik_error add_idle_callback(int *idle_id, idle_callback func, void *user_data)
{
	return os_add_idle_callback(idle_id, func, user_data);
}

artik_error remove_idle_callback(int idle_id)
{
	return os_remove_idle_callback(idle_id);
}
