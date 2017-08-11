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
 * @file	wifi.h
 * @version	0.95
 * @brief	ARTIK WiFi API header
 * @date	Feb 20, 2017
 */

#ifndef __ARTIK_WIFI_H
#define __ARTIK_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup	wifi WiFi
 * @brief		ARTIK WiFi API
 *
 * This module includes WiFi API.
 * @{
 */

#define MAC_ADDR_FIELD		6
#define SSID_LENGTH			32
#define PASSPHRASE_MIN_LEN	8
#define PASSPHRASE_MAX_LEN	64

/**
 * @brief	enum for error return values
 */
enum wifi_result {
	WIFI_SUCCESS				= 0,
	WIFI_ERROR				= -1,
	WIFI_ERROR_ALREADY_INITIALIZED		= -1000,
	WIFI_ERROR_ELOOP_INIT_FAIL		= -1001,
	WIFI_ERROR_ELOOP_REGISTER_SIGNAL_FAIL	= -1002,
	WIFI_ERROR_CONNECT_SOCKET		= -1003,
	WIFI_ERROR_NO_CONTROL_HANDLE		= -1004,
	WIFI_ERROR_WPA_CMD_REQ_FAIL		= -1005,
	WIFI_ERROR_NO_AVAILABLE_NETWORK_LIST	= -1006,
	WIFI_ERROR_SCAN_FAIL_BUSY		= -1007,
	WIFI_ERROR_CONNECT_INVALID_SSID		= -1008,
	WIFI_ERROR_CONNECT_INVALID_PSK		= -1009,

	WIFI_ERROR_SOFTAP			= -2,
	WIFI_ERROR_SOFTAP_ALREADY_INTIALIZED	= -2000,
	WIFI_ERROR_SOFTAP_INVALID_CONFIG	= -2001,
	WIFI_ERROR_SOFTAP_ACCESS_FAIL_CONF	= -2002,
	WIFI_ERROR_SOFTAP_PERMISSION_DENIED	= -2003,
	WIFI_ERROR_SOFTAP_RELOAD_FIRMWARE	= -2004,
	WIFI_ERROR_SOFTAP_SET_IP_ADDR		= -2005,
	WIFI_ERROR_SOFTAP_DNSMASQ		= -2006,
	WIFI_ERROR_SOFTAP_IPFORWARD		= -2007,
	WIFI_ERROR_SOFTAP_HOSTAPD		= -2008,
	WIFI_ERROR_SOFTAP_INTERFACE		= -2009,
	WIFI_ERROR_SOFTAP_NETMAN		= -2010,

};

/**
 * @brief	enum for return parameter of wifi_get_scan_result()
 * @see		wifi_get_scan_result()
 * @see		wifi_scan_bss
 */
enum wifi_security_mode {
	WIFI_SECURITY_MODE			= 0x0,
	WIFI_SECURITY_MODE_AUTH_OPEN		= WIFI_SECURITY_MODE + 0x1,
	WIFI_SECURITY_MODE_AUTH_WPA_PSK		= WIFI_SECURITY_MODE + 0x2,
	WIFI_SECURITY_MODE_AUTH_WPA_EAP		= WIFI_SECURITY_MODE + 0x3,
	WIFI_SECURITY_MODE_AUTH_WPA2_PSK	= WIFI_SECURITY_MODE + 0x4,
	WIFI_SECURITY_MODE_AUTH_WPA2_EAP	= WIFI_SECURITY_MODE + 0x5,

	WIFI_SECURITY_MODE_ENCRYPT_NONE		= WIFI_SECURITY_MODE + 0x10,
	WIFI_SECURITY_MODE_ENCRYPT_WEP		= WIFI_SECURITY_MODE + 0x20,
	WIFI_SECURITY_MODE_ENCRYPT_TKIP		= WIFI_SECURITY_MODE + 0x30,
	WIFI_SECURITY_MODE_ENCRYPT_CCMP		= WIFI_SECURITY_MODE + 0x40,

	WIFI_SECURITY_MODE_WPS_ON		= WIFI_SECURITY_MODE + 0x100
};

/**
 * @brief	enum for wireless mode of soft ap
 * @see		wifi_softap_enable()
 * @see		softap_config
 */
enum wifi_wireless_mode {
	WIFI_WIRELESS_MODE_11B,
	WIFI_WIRELESS_MODE_11G,
	WIFI_WIRELESS_MODE_11A
};

/**
 * @brief	enum for high throughput mode of soft ap
 * @see		wifi_softap_enable()
 * @see		softap_config
 */
enum wifi_ht_mode {
	WIFI_HT_MODE_LEGACY,
	WIFI_HT_MODE_HT,
	WIFI_HT_MODE_VHT
};

typedef unsigned char macaddr;

/**
 * @brief	basic service set information structure
 */
typedef struct wifi_scan_bss {
	macaddr bssid[MAC_ADDR_FIELD];
	/**< BSSID of access point */
	char ssid[SSID_LENGTH];
	/**< SSID of access point */
	int freq;
	/**< operating frequency of access point */
	int rssi;
	/**< rssi value of access point */
	unsigned int auth;
	/**< authentication type of access point */
	unsigned int encrypt;
	/**< encryption type of access point */
	unsigned int wps;
	/**< wps capability of access point */
} wifi_scan_bss;

/**
 * @brief	list of bss items and the number of scanned bss
 */
typedef struct wifi_scan_bssinfo {
	int bss_count;
	/**< the number of scanned bss */
	wifi_scan_bss *bss_list;
	/**< start address of bss items */
} wifi_scan_bssinfo;

/**
 * @brief	soft ap config structure
 */
typedef struct softap_config {
	char ssid[SSID_LENGTH];
	/**< SSID of access point */
	char passphrase[PASSPHRASE_MAX_LEN];
	/**< PASSPHRASE of access point */
	short channel;
	/**< operating channel of access point */
	unsigned int auth;
	/**< authentication type of access point */
	unsigned int encrypt;
	/**< encryption type of access point */
	unsigned int wireless_mode;
	/**< 802.11 wireless mode of access point */
	unsigned int ht_mode;
	/**< HT(high throughput) mode of access point */
} softap_config;

typedef void (*wifi_scan_result_callback)(void *result, void *user_data);
typedef void (*wifi_connect_callback)(void *result, void *user_data);
typedef void (*softap_enable_callback)(int *result, void *user_data);
typedef void (*softap_disable_callback)(int *result, void *user_data);


/**
 * @brief	Initialize wifi API
 * @return	int
 * @see		enum wifi_result
 */
int wifi_initialize(void);

/**
 * @brief	Deinitialize wifi API
 */
void wifi_deinitialize(void);

/**
 * @brief	Issue a scan command
 * @return	int
 * @see		enum wifi_result
 */
int wifi_scan_request(void);

/**
 * @brief	Read a scan result buffer
 * @param[out]	bssinfo	the number of bss and bss list. This pointer
 *			must be null when passed to the function.
 *			This function allocates memory for this pointer
 *			when the function succeeds. The memory should be
 *			released by calling wlan_free_bssinfo() after
 *			the pointer is no longer needed.
 * @return	int
 * @see		struct wifi_scan_bssinfo
 * @see		enum wifi_result
 */
int wifi_get_scan_result(wifi_scan_bssinfo **bssinfo);

/**
 * @brief	free a bssinfo structure
 * @param[in]	bssinfo	the pointer to the memory to be freed.
 * @remark	If bssinfo points to memory that has already been freed
 *		or has not been allocated, an access violation or
 *		heap corruption may occur.
 * @see		struct wifi_scan_bssinfo
 */
void wifi_free_bssinfo(wifi_scan_bssinfo *bssinfo);

/**
 * @brief	Connect to access point
 * @param[in]	ssid SSID of access point to connect
 * @param[in]	psk password of access point. NULL for Open authentication.
 * @param[in]	save_profile a flag to save profile for auto connection after
 *              boot.
 * @return	int
 * @see		enum wifi_result
 */
int wifi_connect(const char *ssid, const char *psk, int save_profile);

/**
 * @brief	Disconnect from access point
 * @return	int
 * @see		enum wifi_result
 */
int wifi_disconnect(void);

/**
 * @brief	Enable Soft AP
 * @param[in]	config	config structure used for Soft AP setup.
 * @return	int
 * @see		enum wifi_result
 * @see		struct softap_config
 */
int wifi_softap_enable(const softap_config *config);

/**
 * @brief	Enable Soft AP by .conf file
 * @param[in]	conf absolute path of .conf file.
 * @return	int
 * @see		enum wifi_result
 */
int wifi_softap_enable_by_conf(const char *conf);

/**
 * @brief	Disable Soft AP
 * @return	int
 * @see		enum wifi_result
 */
int wifi_softap_disable(void);

/**
 * @brief	register scan result callback function.
 * @param[in]	callback
 *		function to register for wifi_scan_result_callback.
 * @param[in]	user_data
 *		A pointer to the client data that will be passed to the
 *              callback. This parameter can be set to NULL.
 */
void wifi_set_scan_result_callback(
		wifi_scan_result_callback callback,
		void *user_data);

/**
 * @brief	unset scan result callback function
 */
void wifi_unset_scan_result_callback(void);

/**
 * @brief	register connection result callback function
 * @param[in]	callback
 *		function to register for wifi_connect_callback.
 * @param[in]	user_data
 *		A pointer to the client data that will be passed to the
 *              callback. This parameter can be set to NULL.
 */
void wifi_set_connect_callback(
		wifi_connect_callback callback,
		void *user_data);

/**
 * @brief	unset connection result callback function
 */
void wifi_unset_connect_callback(void);

/**
 * @brief	register softap enable result callback function
 * @param[in]	callback
 *		function to register for softap_enable_callback.
 * @param[out]	result		the result of enabling Soft AP operation
 * @param[in]	user_data
 *		A pointer to the client data that will be passed to the
 *              callback. This parameter can be set to NULL.
 */
void softap_set_enable_callback(
		softap_enable_callback callback,
		int *result,
		void *user_data);

/**
 * @brief	unset softap enable result callback function
 */
void softap_unset_enable_callback(void);

/**
 * @brief	register softap disable result callback function
 * @param[in]	callback
 *		function to register for softap_disable_callback.
 * @param[out]	result		the result of disabling Soft AP operation
 * @param[in]	user_data
 *		A pointer to the client data that will be passed to the
 *              callback. This parameter can be set to NULL.
 */
void softap_set_disable_callback(
		softap_disable_callback callback,
		int *result,
		void *user_data);

/**
 * @brief	unset softap disable result callback function
 */
void softap_unset_disable_callback(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_WIFI_H */
