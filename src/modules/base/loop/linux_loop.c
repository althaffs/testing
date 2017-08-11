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
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <glib-unix.h>

#include <artik_log.h>
#include <artik_loop.h>

#include "os_loop.h"

struct _timeout {
	timeout_callback func;
	void *user_data;
	guint id;
};

struct _periodic {
	periodic_callback func;
	void *user_data;
	guint id;
};

struct _idle {
	idle_callback func;
	void *user_data;
	guint id;
};

struct _watch {
	watch_callback func;
	void *user_data;
	guint id;
};

struct _signal {
	signal_callback func;
	void *user_data;
	guint id;
};

static GMainLoop *mainloop;

static gboolean _timeout_callback(gpointer user_data)
{
	struct _timeout *timeout = user_data;

	timeout->func(timeout->user_data);

	return FALSE;
}

static void _timeout_destroy_callback(gpointer user_data)
{
	struct _timeout *timeout = user_data;

	memset(timeout, 0, sizeof(struct _timeout));
	g_free(timeout);
}

artik_error os_add_timeout_callback(int *timeout_id, unsigned int msec,
				    timeout_callback func, void *user_data)
{
	struct _timeout *timeout = NULL;
	GSource *source = NULL;

	if (!func || !timeout_id)
		return E_BAD_ARGS;

	timeout = g_try_new0(struct _timeout, 1);
	if (!timeout)
		return E_NO_MEM;

	timeout->func = func;
	timeout->user_data = user_data;

	source = g_timeout_source_new(msec);
	g_source_set_priority(source, G_PRIORITY_HIGH);
	g_source_set_callback(source, _timeout_callback, timeout,
			      _timeout_destroy_callback);
	timeout->id = g_source_attach(source, NULL);
	g_source_unref(source);

	*timeout_id = timeout->id;

	return S_OK;
}

artik_error os_remove_timeout_callback(int timeout_id)
{
	gboolean ret;

	if (timeout_id <= 0)
		return E_BAD_ARGS;

	ret = g_source_remove((guint) timeout_id);
	if (ret == FALSE)
		return E_BAD_ARGS;

	return S_OK;
}


static gboolean _periodic_callback(gpointer user_data)
{
	struct _periodic *periodic = user_data;
	int ret;

	ret = periodic->func(periodic->user_data);
	if (ret == 1)
		return TRUE;

	return FALSE;
}

static void _periodic_destroy_callback(gpointer user_data)
{
	struct _periodic *periodic = user_data;

	memset(periodic, 0, sizeof(struct _periodic));
	g_free(periodic);
}

artik_error os_add_periodic_callback(int *periodic_id, unsigned int msec,
		periodic_callback func, void *user_data)
{
	struct _periodic *periodic;
	GSource *source;

	if (!func || !periodic_id)
		return E_BAD_ARGS;

	periodic = g_try_new0(struct _periodic, 1);
	if (!periodic)
		return E_NO_MEM;

	periodic->func = func;
	periodic->user_data = user_data;

	source = g_timeout_source_new(msec);
	g_source_set_priority(source, G_PRIORITY_HIGH);
	g_source_set_callback(source, _periodic_callback, periodic,
			_periodic_destroy_callback);
	periodic->id = g_source_attach(source, NULL);
	g_source_unref(source);

	*periodic_id = periodic->id;

	return S_OK;
}

artik_error os_remove_periodic_callback(int periodic_id)
{
	gboolean ret;

	if (periodic_id <= 0)
		return E_BAD_ARGS;

	ret = g_source_remove((guint) periodic_id);
	if (ret == FALSE)
		return E_BAD_ARGS;

	return S_OK;
}

void os_loop_run(void)
{
	if (!mainloop)
		mainloop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(mainloop);
}

void os_loop_quit(void)
{
	if (!mainloop)
		return;

	g_main_loop_quit(mainloop);
}

static gboolean _gio_callback(GIOChannel *channel, GIOCondition cond,
			      gpointer user_data)
{
	struct _watch *watch = user_data;
	int fd;
	int ret;
	enum watch_io io = 0;

	if (cond & G_IO_IN)
		io |= WATCH_IO_IN;
	if (cond & G_IO_OUT)
		io |= WATCH_IO_OUT;
	if (cond & G_IO_PRI)
		io |= WATCH_IO_PRI;
	if (cond & G_IO_ERR)
		io |= WATCH_IO_ERR;
	if (cond & G_IO_HUP)
		io |= WATCH_IO_HUP;
	if (cond & G_IO_NVAL)
		io |= WATCH_IO_NVAL;

	fd = g_io_channel_unix_get_fd(channel);

	ret = watch->func(fd, io, watch->user_data);
	if (ret == 1)
		return TRUE;

	return FALSE;
}

static gboolean _gsignal_callback(gpointer user_data)
{
	struct _signal *signal = user_data;
	int ret;

	ret = signal->func(signal->user_data);
	if (ret == 1)
		return TRUE;

	return FALSE;
}

static void _gsignal_destory_callback(gpointer user_data)
{
	struct _signal *signal = user_data;

	memset(signal, 0, sizeof(struct _signal));
	g_free(signal);
}

static void _gio_destroy_callback(gpointer user_data)
{
	struct _watch *watch = user_data;

	memset(watch, 0, sizeof(struct _watch));
	g_free(watch);
}

artik_error os_add_fd_watch(int fd, enum watch_io io, watch_callback func,
						void *user_data, int *watch_id)
{
	struct _watch *watch;
	GIOChannel *channel;
	GIOCondition cond = 0;

	if (fd < 0) {
		log_err("invalid fd(%d)", fd);
		return E_BAD_ARGS;
	}

	if (!func) {
		log_err("func is NULL");
		return E_BAD_ARGS;
	}

	if (io == 0) {
		log_err("invalid io(%d) type", io);
		return E_BAD_ARGS;
	}

	watch = g_try_new0(struct _watch, 1);
	if (!watch)
		return E_NO_MEM;

	if (io & WATCH_IO_IN)
		cond |= G_IO_IN;
	if (io & WATCH_IO_OUT)
		cond |= G_IO_OUT;
	if (io & WATCH_IO_PRI)
		cond |= G_IO_PRI;
	if (io & WATCH_IO_ERR)
		cond |= G_IO_ERR;
	if (io & WATCH_IO_HUP)
		cond |= G_IO_HUP;
	if (io & WATCH_IO_NVAL)
		cond |= G_IO_NVAL;

	watch->func = func;
	watch->user_data = user_data;

	channel = g_io_channel_unix_new(fd);
	watch->id = g_io_add_watch_full(channel, G_PRIORITY_HIGH, cond,
			_gio_callback, watch, _gio_destroy_callback);
	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_unref(channel);

	if (watch_id)
		*watch_id = (int)watch->id;

	return S_OK;
}

artik_error os_remove_fd_watch(int watch_id)
{
	gboolean ret;

	if (watch_id <= 0) {
		log_err("invalid watch_id(%d)", watch_id);
		return -EINVAL;
	}

	ret = g_source_remove((guint) watch_id);
	if (ret == FALSE) {
		log_err("invalid watch_id(%d)", watch_id);
		return -EINVAL;
	}

	return S_OK;
}

artik_error os_add_signal_watch(int signum, signal_callback func,
		void *user_data, int *signal_id)
{
	struct _signal *signal;

	signal = g_try_new0(struct _signal, 1);

	if (!signal)
		return E_NO_MEM;

	signal->func = func;
	signal->user_data = user_data;

	signal->id = g_unix_signal_add_full(G_PRIORITY_DEFAULT, signum,
			_gsignal_callback, signal, _gsignal_destory_callback);

	if (signal_id)
		*signal_id = (int)signal->id;

	return S_OK;
}

artik_error os_remove_signal_watch(int signal_id)
{
	gboolean ret;

	if (signal_id <= 0) {
		log_err("invalid signal_id(%d)", signal_id);
		return -EINVAL;
	}

	ret = g_source_remove((guint) signal_id);
	if (ret == FALSE) {
		log_err("invalid signal_id(%d)", signal_id);
		return -EINVAL;
	}

	return S_OK;
}

static gboolean _idle_callback(gpointer user_data)
{
	struct _idle *idle = user_data;
	int ret;

	ret = idle->func(idle->user_data);
	if (ret == 1)
		return TRUE;

	return FALSE;
}

static void _idle_destroy_callback(gpointer user_data)
{
	struct _idle *idle = user_data;

	memset(idle, 0, sizeof(struct _idle));
	g_free(idle);
}

artik_error os_add_idle_callback(int *idle_id, idle_callback func,
				void *user_data)
{
	struct _idle *idle;

	if (!func)
		return E_BAD_ARGS;

	idle = g_try_new0(struct _idle, 1);
	if (!idle)
		return E_NO_MEM;

	idle->func = func;
	idle->user_data = user_data;
	idle->id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, _idle_callback,
				idle, _idle_destroy_callback);

	*idle_id = (int)idle->id;

	return S_OK;
}

artik_error os_remove_idle_callback(int idle_id)
{
	if (idle_id <= 0)
		return E_BAD_ARGS;

	if (!g_source_remove((guint)idle_id))
		return E_BAD_ARGS;

	return S_OK;
}
