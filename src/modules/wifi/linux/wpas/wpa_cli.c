/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Based on wpa_cli.c code of wpa_supplicant open source,
 * many tweaks have applied - remove interact mode, adding
 * callbacks, etc.
 */
#include "config.h"
#include "wifi.h"
#include "wpa_cli.h"
#include "includes.h"
#include "common.h"
#include "eloop.h"
#include "wpa_ctrl.h"

#include <artik_log.h>
#include <artik_wifi.h>

static const char *ctrl_iface_dir = "/var/run/wpa_supplicant";
static const char *client_socket_dir = NULL;
static int wpa_cli_attached = 0;

static const char *ctrl_ifname;
static struct wpa_ctrl *ctrl_conn = NULL;
static struct wpa_ctrl *mon_conn = NULL;
static int active_scan;

/**
 * @typedef	wifi_callbacks
 * @brief	a set of wifi callback
 */
typedef struct _wifi_callbacks {
	wifi_scan_result_callback scan_result_callback;
	void *scan_result_user_data;
	wifi_connect_callback connect_callback;
	void *connect_user_data;
} wifi_callbacks;
static wifi_callbacks wifi_cb = {0};


struct wpa_ctrl *get_ctrl(void)
{
	return ctrl_conn;
}

void set_active_scan(const int n)
{
	active_scan = n;
}

void set_ctrl_ifname(const char *ifname)
{
	ctrl_ifname = ifname;
}

void set_scan_result_callback(
		wifi_scan_result_callback callback,
		void *user_data)
{
	wifi_cb.scan_result_callback = callback;
	wifi_cb.scan_result_user_data = user_data;
}

void set_connect_callback(
		wifi_connect_callback callback,
		void *user_data)
{
	wifi_cb.connect_callback = callback;
	wifi_cb.connect_user_data = user_data;
}


static void wpa_cli_mon_receive(int sock, void *eloop_ctx, void *sock_ctx);

void wpa_cli_terminate(int sig, void *ctx)
{
#ifndef CONFIG_ELOOP_GMAINLOOP
	eloop_terminate();
#endif
}

void wpa_cli_close_connection(void)
{
	if (!ctrl_conn)
		return;

	if (wpa_cli_attached) {
		wpa_ctrl_detach(mon_conn);
		wpa_cli_attached = 0;
	}
	wpa_ctrl_close(ctrl_conn);
	ctrl_conn = NULL;
	if (mon_conn) {
		eloop_unregister_read_sock(wpa_ctrl_get_fd(mon_conn));
		wpa_ctrl_close(mon_conn);
		mon_conn = NULL;
	}
}

int str_starts(const char *src, const char *match)
{
	return os_strncmp(src, match, os_strlen(match)) == 0;
}

static int check_event(const char *event)
{
	const char *start;
log_dbg("event: %s", event);
	start = os_strchr(event, '>');
	if (start == NULL)
		return 1;

	start++;
	/*
	 * Skip BSS added/removed events since they can be relatively frequent
	 * and are likely of not much use for an interactive user.
	 */
	if (str_starts(start, WPA_EVENT_BSS_ADDED) ||
	    str_starts(start, WPA_EVENT_BSS_REMOVED))
		return 0;

	if (active_scan && str_starts(start, WPA_EVENT_SCAN_RESULTS)) {
		active_scan = 0;
		if (wifi_cb.scan_result_callback) {
			artik_error err = S_OK;

			wifi_cb.scan_result_callback(&err,
				wifi_cb.scan_result_user_data);
		}
	} else if (str_starts(start, WPA_EVENT_CONNECTED)) {
		if (wifi_cb.connect_callback) {
			artik_wifi_connection_info info;

			info.connected = true;
			info.error = S_OK;
			wifi_cb.connect_callback((void *)&info,
				wifi_cb.connect_user_data);
		}
	} else if (str_starts(start, WPA_EVENT_DISCONNECTED)) {
		if (wifi_cb.connect_callback) {
			artik_wifi_connection_info info;

			info.connected = false;
			info.error = S_OK;
			wifi_cb.connect_callback((void *)&info,
				wifi_cb.connect_user_data);
		}
	}
	return 1;
}

static int check_terminating(const char *msg)
{
	const char *pos = msg;

	if (*pos == '<') {
		/* skip priority */
		pos = os_strchr(pos, '>');
		if (pos)
			pos++;
		else
			pos = msg;
	}

	if (str_starts(pos, WPA_EVENT_TERMINATING) && ctrl_conn) {
		log_dbg("Connection to wpa_supplicant lost - trying to\n"
			"reconnect");
		wpa_cli_attached = 0;
		wpa_cli_close_connection();
		return 1;
	}

	return 0;
}

int wpa_cli_open_connection(const char *ifname, int attach)
{
	char *cfile = NULL;
	int flen, res;

	if (!ifname)
		return -1;

	if (client_socket_dir && client_socket_dir[0] &&
	    access(client_socket_dir, F_OK) < 0) {
		perror(client_socket_dir);
		os_free(cfile);
		return -1;
	}

	if (!cfile) {
		flen = os_strlen(ctrl_iface_dir) + os_strlen(ifname) + 2;
		cfile = os_malloc(flen);
		if (cfile == NULL)
			return -1;
		res = os_snprintf(cfile, flen, "%s/%s", ctrl_iface_dir,
				  ifname);
		if (os_snprintf_error(flen, res)) {
			os_free(cfile);
			return -1;
		}
	}

	ctrl_conn = wpa_ctrl_open2(cfile, client_socket_dir);
	if (!ctrl_conn) {
		os_free(cfile);
		return -1;
	}

	if (attach)
		mon_conn = wpa_ctrl_open2(cfile, client_socket_dir);
	else
		mon_conn = NULL;
	os_free(cfile);

	if (mon_conn) {
		if (wpa_ctrl_attach(mon_conn) == 0) {
			wpa_cli_attached = 1;
			eloop_register_read_sock(
				wpa_ctrl_get_fd(mon_conn),
				wpa_cli_mon_receive, NULL, NULL);
		} else {
			log_err("Failed to attach to wpa_supplicant.");
			wpa_cli_close_connection();
			return -1;
		}
	}

	return 0;
}

static void wpa_cli_reconnect(void)
{
	wpa_cli_close_connection();
	wpa_cli_open_connection(ctrl_ifname, 1);
}

static void wpa_cli_recv_pending(struct wpa_ctrl *ctrl, int action_monitor)
{
	if (!ctrl) {
		wpa_cli_reconnect();
		return;
	}
	while (wpa_ctrl_pending(ctrl) > 0) {
		char buf[4096];
		size_t len = sizeof(buf) - 1;

		if (wpa_ctrl_recv(ctrl, buf, &len) == 0) {
			buf[len] = '\0';

			check_event(buf);

			if (check_terminating(buf) > 0)
				return;
		} else {
			log_dbg("Could not read pending message.");
			break;
		}
	}

	if (wpa_ctrl_pending(ctrl) < 0) {
		log_dbg("Connection to wpa_supplicant lost - trying to reconnect");
		wpa_cli_reconnect();
	}
}

static void wpa_cli_mon_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	wpa_cli_recv_pending(mon_conn, 0);
}
