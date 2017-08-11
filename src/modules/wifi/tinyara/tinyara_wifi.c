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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "artik_list.h"
#include "artik_log.h"
#include "artik_types.h"
#include "artik_wifi.h"
#include "../os_wifi.h"
#include "../linux/wifi.h"
#include <wifi/slsi_wifi_api.h>
#include <wifi/slsi_wifi_utils.h>

enum wifi_callback_type {
	WIFI_CB_CONNECT = 0,
	WIFI_CB_DISCONNECT,
	WIFI_CB_SCAN,
	WIFI_CB_COUNT
};

struct wifi_callback {
	artik_wifi_callback cb;
	void *user_data;
};

static struct wifi_callback g_callbacks[WIFI_CB_COUNT];
static artik_wifi_mode_t g_mode = ARTIK_WIFI_MODE_NONE;
static artik_wifi_ap *g_saved_aps = NULL;
static int g_saved_num_aps = 0;

static void wifi_link_up_callback(slsi_reason_t *reason)
{
	artik_wifi_connection_info info;

	log_dbg("reason: %d", reason->reason_code);

	info.connected = false;

	switch (reason->reason_code) {
	case SLSI_STATUS_SUCCESS:
		info.error = S_OK;
		info.connected = true;
		break;
	case SLSI_REASON_NETWORK_AUTHENTICATION_FAILED:
		info.error = E_WIFI_ERROR_AUTHENTICATION;
		break;
	case SLSI_REASON_ASSOCIATION_REQ_FAILED:
		info.error = E_WIFI_ERROR_ASSOCIATION;
		break;
	default:
		info.error = E_WIFI_ERROR;
		break;
	}

	if (g_callbacks[WIFI_CB_CONNECT].cb)
		g_callbacks[WIFI_CB_CONNECT].cb(&info,
				g_callbacks[WIFI_CB_CONNECT].user_data);
}

static void wifi_link_down_callback(slsi_reason_t *reason)
{
	artik_wifi_connection_info info;

	log_dbg("reason: %d", reason->reason_code);

	switch (reason->reason_code) {
	case SLSI_STATUS_SUCCESS:
		info.error = S_OK;
		break;
	case SLSI_REASON_NETWORK_AUTHENTICATION_FAILED:
		info.error = E_WIFI_ERROR_AUTHENTICATION;
		break;
	case SLSI_REASON_ASSOCIATION_REQ_FAILED:
		info.error = E_WIFI_ERROR_ASSOCIATION;
		break;
	default:
		info.error = E_WIFI_ERROR;
		break;
	}

	info.connected = false;

	if (g_callbacks[WIFI_CB_CONNECT].cb)
		g_callbacks[WIFI_CB_CONNECT].cb(&info,
				g_callbacks[WIFI_CB_CONNECT].user_data);
}

static void wifi_save_scan_results(void)
{
	int8_t ret = 0;
	slsi_scan_info_t *scan_results = NULL;
	slsi_scan_info_t *head = NULL;
	int bss_count = 1;
	int i = 0;

	ret = WiFiGetScanResults(&scan_results);
	if (ret != SLSI_STATUS_SUCCESS) {
		log_err("Wi-Fi scan results request error=%d", ret);
		return;
	}

	if (!scan_results)
		return;

	head = scan_results;

	/* Count the number of scanned APs */
	while (scan_results->next) {
		scan_results = scan_results->next;
		bss_count++;
	}

	scan_results = head;

	if (g_saved_aps) {
		/*
		 * Previous scan result was saved but not retrieved,
		 * override it
		 */
		free(g_saved_aps);
		g_saved_num_aps = 0;
	}

	g_saved_num_aps = bss_count;
	g_saved_aps = malloc(bss_count * sizeof(artik_wifi_ap));
	if (!g_saved_aps) {
		log_err("Failed to allocated memory for scan results");
		return;
	}

	for (i = 0; i < bss_count; i++) {
		snprintf(g_saved_aps[i].name, MAX_AP_NAME_LEN, "%s",
				scan_results->ssid);
		snprintf(g_saved_aps[i].bssid, MAX_AP_BSSID_LEN, "%s",
				scan_results->bssid);
		g_saved_aps[i].frequency = 2407 + 5 * scan_results->channel;
		g_saved_aps[i].signal_level = scan_results->rssi;

		if (scan_results->sec_modes->secmode == SLSI_SEC_MODE_OPEN)
			g_saved_aps[i].encryption_flags = WIFI_ENCRYPTION_OPEN;
		else if (scan_results->sec_modes->secmode ==
						SLSI_SEC_MODE_WPA2_CCMP)
			g_saved_aps[i].encryption_flags = WIFI_ENCRYPTION_WPA2;
		else if (scan_results->sec_modes->secmode ==
						SLSI_SEC_MODE_WPA_CCMP)
			g_saved_aps[i].encryption_flags = WIFI_ENCRYPTION_WPA;
		else if (scan_results->sec_modes->secmode == SLSI_SEC_MODE_WEP)
			g_saved_aps[i].encryption_flags = WIFI_ENCRYPTION_WEP;
		else
			g_saved_aps[i].encryption_flags = 0;

		scan_results = scan_results->next;
	}

	scan_results = head;
	if (scan_results)
		WiFiFreeScanResults(&scan_results);
}

static int8_t wifi_scan_callback(slsi_reason_t *reason)
{
	artik_error result = (reason->reason_code == SLSI_STATUS_SUCCESS) ?
			S_OK : E_WIFI_ERROR;

	log_dbg("reason: %d", reason->reason_code);

	if (reason->reason_code == 0) {
		/* Save scan results for later retrieval by the application */
		wifi_save_scan_results();

		if (g_callbacks[WIFI_CB_SCAN].cb)
			g_callbacks[WIFI_CB_SCAN].cb(&result,
					g_callbacks[WIFI_CB_SCAN].user_data);
	}

	return 0;
}

artik_error os_wifi_disconnect(void)
{
	int8_t ret = 0;

	log_dbg("");

	if (g_mode != ARTIK_WIFI_MODE_STATION)
		return E_NOT_INITIALIZED;

	ret = WiFiNetworkLeave();
	if (ret != SLSI_STATUS_SUCCESS) {
		log_err("Wi-Fi leave request error=%d", ret);
		return E_WIFI_ERROR;
	}

	return S_OK;
}

artik_error os_wifi_scan_request(void)
{
	int8_t ret = 0;

	log_dbg("");

	if (g_mode == ARTIK_WIFI_MODE_NONE)
		return E_NOT_INITIALIZED;

	WiFiRegisterScanCallback(wifi_scan_callback);
	WiFiRegisterLinkCallback(wifi_link_up_callback,
				wifi_link_down_callback);

	ret = WiFiScanNetwork();
	if (ret != SLSI_STATUS_SUCCESS) {
		WiFiRegisterScanCallback(NULL);
		log_err("Wi-Fi scan request error=%d", ret);
		return E_WIFI_ERROR;
	}

	return S_OK;
}

artik_error os_wifi_init(artik_wifi_mode_t mode)
{
	int8_t ret = 0;

	log_dbg("");

	if (g_mode != ARTIK_WIFI_MODE_NONE)
		return E_BUSY;

	if ((mode != ARTIK_WIFI_MODE_STATION) && (mode != ARTIK_WIFI_MODE_AP))
		return E_BAD_ARGS;

	memset(&g_callbacks, 0, sizeof(g_callbacks));

	//WiFiRegisterScanCallback(wifi_scan_callback);
	WiFiRegisterLinkCallback(wifi_link_up_callback,
				wifi_link_down_callback);

	/*
	 * Always init in station mode to allow reading MAC address.
	 * Mode will be changed later if starting AP.
	 */
	ret = WiFiStart(SLSI_WIFI_STATION_IF, NULL);
	if (ret != SLSI_STATUS_SUCCESS) {
		WiFiRegisterScanCallback(NULL);
		WiFiRegisterLinkCallback(NULL, NULL);
		log_err("Wi-Fi start request error=%d", ret);
		return E_WIFI_ERROR;
	}

	g_mode = mode;

	return S_OK;
}

artik_error os_wifi_deinit(void)
{
	log_dbg("");

	WiFiRegisterScanCallback(NULL);
	WiFiRegisterLinkCallback(NULL, NULL);
	WiFiStop();
	g_mode = ARTIK_WIFI_MODE_NONE;

	return S_OK;
}

artik_error os_wifi_get_scan_result(artik_wifi_ap **aps, int *num_aps)
{
	log_dbg("");

	if (g_mode == ARTIK_WIFI_MODE_NONE)
		return E_NOT_INITIALIZED;

	if (!aps || !num_aps)
		return E_BAD_ARGS;

	/* Give out pointer to the user, who is now in charge of freeing it */
	*num_aps = g_saved_num_aps;
	*aps = g_saved_aps;
	g_saved_aps = NULL;
	g_saved_num_aps = 0;

	return S_OK;
}

artik_error os_wifi_start_ap(const char *ssid, const char *password,
	unsigned int channel, unsigned int encryption_flags)
{
	int8_t ret = 0;
	slsi_ap_config_t config;
	slsi_security_config_t sec_config;

	log_dbg("");

	if (g_mode != ARTIK_WIFI_MODE_AP)
		return E_NOT_INITIALIZED;

	if ((channel > 14) || (channel < 1) || !ssid ||
		(strlen(ssid) > SLSI_SSID_LEN))
		return E_BAD_ARGS;

	/* Only WPA2 and open modes supported for now */
	if (encryption_flags && !(encryption_flags & WIFI_ENCRYPTION_WPA2))
		return E_NOT_SUPPORTED;

	/* Check if password is provided and valid for relevant modes */
	if (encryption_flags & WIFI_ENCRYPTION_WPA2) {
		if (!password ||
			(strlen(password) < MIN_AP_WPA2PASS_LEN) ||
			(strlen(password) > MAX_AP_WPA2PASS_LEN))
			return E_BAD_ARGS;
	}

	memset(&config, 0, sizeof(config));
	config.beacon_period = 100;
	config.DTIM = 2;
	config.phy_mode = 1;
	config.channel = channel;
	config.ssid_len = strlen(ssid);
	memcpy(config.ssid, ssid, config.ssid_len);

	if (encryption_flags && !(encryption_flags & WIFI_ENCRYPTION_WPA2)) {
		sec_config.secmode = SLSI_SEC_MODE_WPA2_CCMP;
		memcpy(sec_config.passphrase, password, strlen(password));
		config.security = &sec_config;
	}

	ret = WiFiStart(SLSI_WIFI_SOFT_AP_IF, &config);
	if ((ret != SLSI_STATUS_SUCCESS) && (ret !=
						SLSI_STATUS_ALREADY_STARTED)) {
		log_err("Failed to start Access Point (%d)", ret);
		return E_WIFI_ERROR;
	}

	return S_OK;
}

artik_error os_wifi_connect(const char *ssid, const char *password,
		bool persistent)
{
	int8_t ret = 0;
	slsi_security_config_t config;

	log_dbg("");

	if (g_mode != ARTIK_WIFI_MODE_STATION)
		return E_NOT_INITIALIZED;

	memset(&config, 0, sizeof(config));

	if (password != NULL) {
		config.secmode = SLSI_SEC_MODE_WPA2_CCMP;
		memcpy(config.passphrase, password, strlen(password));
	}

	ret = WiFiNetworkJoin((uint8_t *)ssid, strlen(ssid), NULL, &config);
	if (ret != SLSI_STATUS_SUCCESS) {
		switch (ret) {
		case SLSI_STATUS_PARAM_FAILED:
			return E_WIFI_ERROR_BAD_PARAMS;
		default:
			log_err("Failed to join Access Point (%d)", ret);
			return E_WIFI_ERROR;
		}
	}

	return S_OK;
}

artik_error os_wifi_set_connect_callback(artik_wifi_callback user_callback,
		void *user_data)
{
	log_dbg("");

	if (g_mode == ARTIK_WIFI_MODE_NONE)
		return E_NOT_INITIALIZED;

	if (g_callbacks[WIFI_CB_CONNECT].cb && user_callback)
		return E_BUSY;

	g_callbacks[WIFI_CB_CONNECT].cb = user_callback;
	g_callbacks[WIFI_CB_CONNECT].user_data = user_data;

	return S_OK;
}

artik_error os_wifi_set_scan_result_callback(artik_wifi_callback user_callback,
		void *user_data)
{
	log_dbg("");

	if (g_mode == ARTIK_WIFI_MODE_NONE)
		return E_NOT_INITIALIZED;

	if (g_callbacks[WIFI_CB_SCAN].cb && user_callback)
		return E_BUSY;

	g_callbacks[WIFI_CB_SCAN].cb = user_callback;
	g_callbacks[WIFI_CB_SCAN].user_data = user_data;

	return S_OK;
}
