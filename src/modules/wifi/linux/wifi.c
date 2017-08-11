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
/**
 *
 * @file	wifi.c
 * @version	0.95
 * @brief	ARTIK WiFi API library
 * @date	Apr 18, 2016
 */

#include "config.h"
#include "wifi.h"

#include "wpas/includes.h"
#include "wpas/common.h"
#include "wpas/eloop.h"
#include "wpas/wpa_ctrl.h"
#include "wpas/list.h"
#include "wpas/wpa_cli.h"

#include <artik_log.h>
#include <artik_wifi.h>

#ifndef CONFIG_ELOOP_GMAINLOOP
static pthread_t thread;
static pthread_mutex_t mutex;
#endif
static struct wpa_ctrl *ctrl_conn;
static const char *ctrl_ifname;

static int _wifi_get_bss_count(char *bsslist, size_t len)
{
	int bss_count = 0;
	int idx = 0;

	while ((size_t)idx++ < len) {
		if (bsslist[idx] == '\n')
			bss_count++;
	}

	return bss_count - 1;
}

void wifi_set_scan_result_callback(wifi_scan_result_callback callback,
					void *user_data)
{
	set_scan_result_callback(callback, user_data);
}

void wifi_unset_scan_result_callback(void)
{
	set_scan_result_callback(NULL, NULL);
}

void wifi_set_connect_callback(wifi_connect_callback callback,
					void *user_data)
{
	set_connect_callback(callback, user_data);
}

void wifi_unset_connect_callback(void)
{
	set_connect_callback(NULL, NULL);
}

static int _wifi_send_cmd(struct wpa_ctrl *ctrl, char *cmd, char *buf,
				size_t *buflen)
{
	if (!ctrl || !buf)
		return WIFI_ERROR_NO_CONTROL_HANDLE;

	os_memset(buf, 0, *buflen);

	if (wpa_ctrl_request(ctrl, cmd, os_strlen(cmd), buf, (unsigned int *)buflen, NULL) < 0)
		return WIFI_ERROR_WPA_CMD_REQ_FAIL;

	return WIFI_SUCCESS;
}

int wifi_scan_request(void)
{
	int ret = WIFI_ERROR;
	size_t len = 4096;
	char buf[4096];

	os_memset(buf, '\0', len);

	ret = _wifi_send_cmd(ctrl_conn, "SCAN", buf, &len);
	if (ret != WIFI_SUCCESS)
		return ret;

	if (str_starts(buf, "OK")) {
		set_active_scan(1);
		ret = WIFI_SUCCESS;
	} else {
		if (str_starts(buf, "FAIL-BUSY"))
			ret = WIFI_ERROR_SCAN_FAIL_BUSY;
		else
			ret = WIFI_ERROR;
	}

	return ret;
}

static void _atomac(char *str_mac, macaddr *mac)
{
	int i = 0;
	char *pos = str_mac;

	mac[i] = strtol(strtok(pos, ":"), NULL, 16);

	for (i = 1; i < MAC_ADDR_FIELD; i++)
		mac[i] = strtol(strtok(NULL, ":"), NULL, 16);
}

static int _wifi_set_security_mode(char *chflag, wifi_scan_bss *bss)
{
	int ret = WIFI_ERROR;
	char *start = NULL, *end = NULL;
	char *strflag = NULL;
	int len = 0;


	start = chflag;
	while (*chflag != '\t')
		chflag++;
	end = chflag;

	len = (end - start) + 1;
	strflag = os_malloc(len);
	os_memset(strflag, 0, len);

	os_strlcpy(strflag, start, len - 1);

	/* authentication flags */
	if (os_strstr(strflag, "WPA2"))	{
		if (os_strstr(strflag, "PSK"))
			bss->auth = WIFI_SECURITY_MODE_AUTH_WPA2_PSK;
		else if (os_strstr(strflag, "EAP"))
			bss->auth = WIFI_SECURITY_MODE_AUTH_WPA2_EAP;
	} else if (os_strstr(strflag, "WPA")) {
		if (os_strstr(strflag, "PSK"))
			bss->auth = WIFI_SECURITY_MODE_AUTH_WPA_PSK;
		else if (os_strstr(strflag, "EAP"))
			bss->auth = WIFI_SECURITY_MODE_AUTH_WPA_EAP;
	} else
		bss->auth = WIFI_SECURITY_MODE_AUTH_OPEN;

	/* encryption flags */
	if (os_strstr(strflag, "CCMP"))
		bss->encrypt = WIFI_SECURITY_MODE_ENCRYPT_CCMP;
	else if (os_strstr(strflag, "TKIP"))
		bss->encrypt = WIFI_SECURITY_MODE_ENCRYPT_TKIP;
	else if (os_strstr(strflag, "WEP"))
		bss->encrypt = WIFI_SECURITY_MODE_ENCRYPT_WEP;

	/* WPS flags */
	if (os_strstr(strflag, "WPS"))
		bss->wps = WIFI_SECURITY_MODE_WPS_ON;

	os_free(strflag);
	return ret;
}

int wifi_get_scan_result(wifi_scan_bssinfo **bssinfo)
{
	int ret = WIFI_ERROR;
	char *pos, *securityflag;
	size_t len = 8192;
	char buf[8192];
	wifi_scan_bss *bss = NULL;
	int eol;
	int i;

	if (*bssinfo)
		return WIFI_ERROR;

	os_memset(buf, 0, len);

	ret = _wifi_send_cmd(ctrl_conn, "SCAN_RESULTS", buf, &len);
	if (ret != WIFI_SUCCESS)
		return ret;


	*bssinfo = os_malloc(sizeof(wifi_scan_bssinfo));
	os_memset(*bssinfo, 0, sizeof(wifi_scan_bssinfo));

	pos = buf;
	(*bssinfo)->bss_count = _wifi_get_bss_count(pos, len);

	pos = os_strstr(pos, "\n");

	if (!pos || !(pos + 1))
		return WIFI_ERROR_NO_AVAILABLE_NETWORK_LIST;

	pos++;

	bss = os_malloc((*bssinfo)->bss_count * sizeof(wifi_scan_bss));
	os_memset(bss, 0, (*bssinfo)->bss_count * sizeof(wifi_scan_bss));

	for (i = 0; i < (*bssinfo)->bss_count; i++) {
		_atomac(strtok(pos, "\t"), bss[i].bssid);
		pos += 18;

		bss[i].freq = atoi(pos);
		while (*pos != '\t')
			pos++;
		pos++;

		bss[i].rssi = atoi(pos);
		while (*pos != '\t')
			pos++;

		pos++;
		securityflag = pos;
		_wifi_set_security_mode(securityflag, &(bss[i]));

		while (*pos != '\t')
			pos++;
		pos++;
		eol = os_strstr(pos, "\n") - pos;
		if (eol < 0)
			break;
		strncpy(bss[i].ssid, pos, eol);
		bss[i].ssid[eol] = '\0';
		pos += eol + 1;
	}
	(*bssinfo)->bss_list = bss;

	return WIFI_SUCCESS;
}

void wifi_free_bssinfo(wifi_scan_bssinfo *bssinfo)
{
	if (!bssinfo)
		return;

	if (!(bssinfo->bss_list))
		goto freeinfo;

	os_free(bssinfo->bss_list);
	bssinfo->bss_count = 0;
	bssinfo->bss_list = NULL;

freeinfo:
	os_free(bssinfo);
}

int wifi_connect(const char *ssid, const char *psk, int save_profile)
{
	int ret = WIFI_ERROR;
	char cmd[100];
	char *buf = NULL;
	size_t len = 4096;
	char *pos, *netid_idx;
	int netid = 0;
	int netid_len = 0;
	char strnid[5] = "\0";

	if (!ssid || (os_strlen(ssid) == 0) || (os_strchr(ssid, ' ')))
		return WIFI_ERROR_CONNECT_INVALID_SSID;

	if (psk && ((os_strlen(psk) < 8) || (os_strchr(psk, ' '))))
		return WIFI_ERROR_CONNECT_INVALID_PSK;

	buf = os_malloc(len);
	os_memset(buf, 0, len);
	os_memset(cmd, 0, 100);

	if (!buf)
		return WIFI_ERROR;


	ret = _wifi_send_cmd(ctrl_conn, "LIST_NETWORKS", buf, &len);

	if (ret != WIFI_SUCCESS) {
		os_free(buf);
		return ret;
	}

	pos = NULL;
	/* if SSID exists, reconfigure it. */
	pos = os_strstr(buf, ssid);
	if (pos) {
		while (*pos != '\n')
			--pos;
		netid_idx = pos + 1;

		while (*pos != '\t') {
			netid_len++;
			pos++;
		}

		strncpy(strnid, netid_idx, netid_len - 1);
		netid = atoi(strnid);
	} else {
		ret = _wifi_send_cmd(ctrl_conn, "ADD_NETWORK", buf, &len);
		if (ret != WIFI_SUCCESS) {
			os_free(buf);
			return ret;
		}

		netid = atoi(buf);
	}

	sprintf(cmd, "SET_NETWORK %d ssid \"%s\"", netid, ssid);
	ret = _wifi_send_cmd(ctrl_conn, cmd, buf, &len);

	if (ret != WIFI_SUCCESS) {
		os_free(buf);
		return ret;
	}

	if (!str_starts(buf, "OK")) {
		log_err("%s: %s", cmd, buf);
		os_free(buf);
		return WIFI_ERROR;
	}

	if (psk && os_strlen(psk)) {
		sprintf(cmd, "SET_NETWORK %d psk \"%s\"", netid, psk);
		ret = _wifi_send_cmd(ctrl_conn, cmd, buf, &len);
		if (ret != WIFI_SUCCESS) {
			os_free(buf);
			return ret;
		}

		if (!str_starts(buf, "OK")) {
			log_err("%s: %s", cmd, buf);
			os_free(buf);
			return WIFI_ERROR;
		}
	} else {
		os_memset(cmd, 0, 100);
		sprintf(cmd, "SET_NETWORK %d key_mgmt %s", netid, "NONE");
		ret = _wifi_send_cmd(ctrl_conn, cmd, buf, &len);
		if (ret != WIFI_SUCCESS) {
			os_free(buf);
			return ret;
		}
		if (!str_starts(buf, "OK")) {
			log_err("%s: %s", cmd, buf);
			os_free(buf);
			return WIFI_ERROR;
		}
	}

	os_memset(cmd, 0, 100);
	sprintf(cmd, "SELECT_NETWORK %d", netid);
	ret = _wifi_send_cmd(ctrl_conn, cmd, buf, &len);
	if (ret != WIFI_SUCCESS) {
		os_free(buf);
		return ret;
	}
	if (!str_starts(buf, "OK")) {
		log_err("%s: %s", cmd, buf);
		os_free(buf);
		return WIFI_ERROR;
	}

	if (save_profile) {
		os_memset(cmd, 0, 100);
		sprintf(cmd, "SAVE_CONFIG");
		ret = _wifi_send_cmd(ctrl_conn, cmd, buf, &len);
		if (ret != WIFI_SUCCESS) {
			os_free(buf);
			return ret;
		}
		if (!str_starts(buf, "OK")) {
			log_err("%s: %s", cmd, buf);
			os_free(buf);
			return WIFI_ERROR;
		}
	}

	os_free(buf);

	return ret;
}

int wifi_disconnect(void)
{
	int ret = WIFI_ERROR;
	char buf[4096];
	size_t len = sizeof(buf);

	ret = _wifi_send_cmd(ctrl_conn, "DISCONNECT", buf, &len);
	if (ret != WIFI_SUCCESS)
		return ret;
	if (!str_starts(buf, "OK"))
		return WIFI_ERROR;

	return WIFI_SUCCESS;
}

int wifi_initialize(void)
{
	if (ctrl_ifname)
		return WIFI_ERROR_ALREADY_INITIALIZED;

/*	ctrl_ifname = strdup("wlp2s0");*/
	ctrl_ifname = strdup("wlan0");
	set_ctrl_ifname(ctrl_ifname);

	if (eloop_init())
		return WIFI_ERROR_ELOOP_INIT_FAIL;

#ifndef CONFIG_ELOOP_GMAINLOOP
	/* Register terminate signal */
	if (eloop_register_signal_terminate(wpa_cli_terminate, NULL))
		return WIFI_ERROR_ELOOP_REGISTER_SIGNAL_FAIL;
#endif

	if (wpa_cli_open_connection(ctrl_ifname, 1) < 0) {
		log_err("Failed to connect to ctrl_ifname: %s,  error: %s",
			ctrl_ifname ? ctrl_ifname : "(nil)", strerror(errno));
		return WIFI_ERROR_CONNECT_SOCKET;
	}
	ctrl_conn = get_ctrl();
	if (!ctrl_conn)
		return WIFI_ERROR_CONNECT_SOCKET;

	set_active_scan(0);

#ifndef CONFIG_ELOOP_GMAINLOOP
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&thread, NULL, (void *)&eloop_run, NULL);
#endif

	log_dbg("%s succeeded", __func__);

	return WIFI_SUCCESS;
}

void wifi_deinitialize(void)
{
	if (ctrl_ifname) {
		os_free((void *)ctrl_ifname);
		ctrl_ifname = NULL;
	}

	wpa_cli_close_connection();

#ifndef CONFIG_ELOOP_GMAINLOOP
	eloop_terminate();
#endif
	eloop_destroy();

#ifndef CONFIG_ELOOP_GMAINLOOP
	pthread_mutex_destroy(&mutex);
	pthread_cancel(thread);
#endif

	log_dbg("%s succeeded", __func__);
}
