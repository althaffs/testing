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

#ifndef __ARTIK_BT_CORE_H
#define __ARTIK_BT_CORE_H

#include <stdio.h>
#include <unistd.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <gio/gio.h>
#pragma GCC diagnostic pop

#include <artik_bluetooth.h>
#include <artik_log.h>

#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DBUS_BLUEZ_BUS				"org.bluez"
#define DBUS_BLUEZ_OBEX_BUS			"org.bluez.obex"

#define DBUS_IF_ADAPTER1			"org.bluez.Adapter1"
#define DBUS_IF_DEVICE1				"org.bluez.Device1"
#define DBUS_IF_AGENT_MANGER1		"org.bluez.AgentManager1"
#define DBUS_IF_AGENT1			"org.bluez.Agent1"
#define DBUS_IF_HEARTRATE_MANAGER1	"org.bluez.HeartRateManager1"
#define DBUS_IF_HEARTRATE_WATCHER1	"org.bluez.HeartRateWatcher1"
#define DBUS_IF_PROXIMITYMONITOR1	"org.bluez.ProximityMonitor1"
#define DBUS_IF_PROXIMITYREPORTER1	"org.bluez.ProximityReporter1"
#define DBUS_IF_GATTMANAGER1		"org.bluez.GattManager1"
#define DBUS_IF_GATTSERVICE1		"org.bluez.GattService1"
#define DBUS_IF_GATTCHARACTERISTIC1	"org.bluez.GattCharacteristic1"
#define DBUS_IF_GATTDESCRIPTOR1		"org.bluez.GattDescriptor1"
#define DBUS_IF_LEADV_MANAGER1		"org.bluez.LEAdvertisingManager1"
#define DBUS_IF_PROPERTIES			"org.freedesktop.DBus.Properties"
#define DBUS_IF_OBJECT_MANAGER		"org.freedesktop.DBus.ObjectManager"
#define DBUS_IF_MEDIA1				"org.bluez.Media1"
#define DBUS_IF_MEDIA_TRANSPORT1	"org.bluez.MediaTransport1"
#define DBUS_IF_MEDIA_ENDPOINT1		"org.bluez.MediaEndpoint1"
#define DBUS_IF_MEDIA_FOLDER1		"org.bluez.MediaFolder1"
#define DBUS_IF_MEDIA_CONTROL1		"org.bluez.MediaControl1"
#define DBUS_IF_MEDIA_PLAYER1		"org.bluez.MediaPlayer1"
#define DBUS_IF_MEDIAITEM1			"org.bluez.MediaItem1"
#define DBUS_IF_NETWORK_SERVER1		"org.bluez.NetworkServer1"
#define DBUS_IF_NETWORK1			"org.bluez.Network1"
#define DBUS_IF_PROFILE				"org.bluez.Profile1"
#define DBUS_IF_PROFILE_MANAGER1	"org.bluez.ProfileManager1"
#define DBUS_IF_OBEX_CLIENT			"org.bluez.obex.Client1"
#define DBUS_IF_OBEX_TRANSFER		"org.bluez.obex.Transfer1"
#define DBUS_IF_OBEX_FILE_TRANSFER	"org.bluez.obex.FileTransfer1"
#define DBUS_IF_OBEX_SESSION		"org.bluez.obex.Session1"



#define DBUS_BLUEZ_OBJECT_PATH_HCI0	"/org/bluez/hci0"
#define DBUS_BLUEZ_OBJECT_PATH		"/org/bluez"
#define DBUS_AGENT_PATH				"/artik/agent"
#define DBUS_HEARTRATE_AGENT		"/artik/agent/heartrate"
#define ADVERTISEMENT_PREFIX	"/artik/advertisement"
#define GATT_SERVICE_PREFIX			"/artik/service"
#define GATT_CHARACTERISTIC_PREFIX	"/characteristic"
#define GATT_DESCRIPTOR_PREFIX		"/descriptor"
#define DBUS_BLUEZ_SPP_PROFILE		"/artik/spp/profile"

#define BT_SPP_SHORT_UUID			"1101"
#define DBUS_BLUEZ_OBEX_PATH		"/org/bluez/obex"
#define DBUS_BLUEZ_OBEX_SESSION_PATH "/org/bluez/obex/client/"

#define BT_UUID_LENGHT			32
#define BT_DEVICE_OBJECT_PATH	38
#define SESSION_PATH_LEN		43

#define error_list(x)	{x, bt_error_message[x]},


typedef enum {
	BT_DEVICE_STATE_IDLE,
	BT_DEVICE_STATE_PAIRING,
	BT_DEVICE_STATE_PAIRED,
	BT_DEVICE_STATE_CONNECTING,
	BT_DEVICE_STATE_CONNECTED
} bt_device_state;

typedef enum {
	BT_GATT_REQ_TYPE_READ,
	BT_GATT_REQ_TYPE_WRITE,
	BT_GATT_REQ_TYPE_NOTIFY,
	BT_GATT_REQ_TYPE_END
} bt_gatt_req_type;

typedef enum {
	BT_FTP_STATE_SUSPENDING,
	BT_FTP_STATE_SUSPENDED,
	BT_FTP_STATE_RESUMING,
	BT_FTP_STATE_RESUMED,
	BT_FTP_STATE_CANCELING,
	BT_FTP_STATE_CANCELED,
	BT_FTP_STATE_ERROR,
	BT_FTP_STATE_END
} bt_ftp_transport_state;

typedef enum {
	BT_FTP_REQ_GET,
	BT_FTP_REQ_PUT,
	BT_FTP_REQ_END
} bt_ftp_req_type;

typedef struct {
	gchar *serv_path;
	guint serv_id;
	gchar *service_uuid;
	guint manager_id;
	guint prop_id;
	GSList *char_data;
	gboolean is_svc_registered;
	gboolean is_svc_primary;
} bt_gatt_service;

typedef struct {
	gchar *char_uuid;
	gchar *srv_uuid;
	gchar *path;
} bt_gatt_client;

typedef struct {
	gchar *char_path;
	guint char_id;
	gchar *char_uuid;
	guchar *char_value;
	GSList *char_props;
	bt_gatt_service *service;
	guint value_length;
	guint flags_length;
	GSList *desc_data;

	artik_bt_gatt_req_read read_callback;
	artik_bt_gatt_req_write write_callback;
	artik_bt_gatt_req_notify notify_callback;

	void *read_user_data;
	void *write_user_data;
	void *notify_user_data;
} bt_gatt_char;

typedef struct {
	gchar *desc_path;
	guint desc_id;
	gchar *desc_uuid;
	guchar *desc_value;
	guint value_length;
	GSList *desc_props;
	bt_gatt_char *chr;

	artik_bt_gatt_req_read read_callback;
	artik_bt_gatt_req_write write_callback;

	void *read_user_data;
	void *write_user_data;
} bt_gatt_desc;

typedef struct {
	bt_gatt_req_type type;
	bt_gatt_char *chr;
	bt_gatt_desc *desc;
	GDBusMethodInvocation *invocation;
	guint len;
	guchar *value;
} bt_gatt_req_handle;

typedef struct {
	gchar *adv_path;
	guint adv_id;
	artik_bt_advertisement *user_data;
} bt_advertisement;

typedef struct {
	artik_bt_callback fn;
	void *data;
} bt_event_callback;

typedef struct {
	GDBusConnection *conn;
	GDBusConnection *session_conn;
	GHashTable *subscribe_ids;
	GHashTable *registration_ids;
	bt_event_callback callback[BT_EVENT_END];
	GSList *gatt_services;
	GSList *gatt_clients;
	GSList *advertisements;
	GSource *source;
	bt_device_state state;
	prop_change_callback prop_callback;
} bt_handler;

extern bt_handler hci;

extern char session_path[SESSION_PATH_LEN];
extern artik_bt_ftp_property transfer_property;

artik_error bt_init(GBusType dbus_type, GDBusConnection **connection);

artik_error bt_deinit(void);

void _user_callback(artik_bt_event event, void *data);

void _get_adapter_properties(GVariant *prop_array, artik_bt_adapter *adapter);

void _get_device_properties(GVariant *prop_array, artik_bt_device *device);

artik_error _get_managed_objects(GVariant **variant);

void _get_object_path(const char *addr, char **path);

artik_error _get_devices(bt_device_state state,
		artik_bt_device **device_list, int *count);

void _get_gatt_path(const char *addr, const char *interface,
		const char *uuid, const char *property, const char *value, char **gatt_path);

void _get_gatt_uuid_list(const char *gatt_path, const char *interface,
		artik_bt_uuid **uuid_list, int *len);

void _on_interface_added(const gchar *sender_name,
	const gchar *object_path, const gchar *interface_name,
	GVariant *parameters, gpointer user_data);

void _on_interface_removed(const gchar *sender_name,
	const gchar *object_path, const gchar *interface_name,
	GVariant *parameters, gpointer user_data);

void _on_properties_changed(const gchar *sender_name,
	const gchar *object_path, const gchar *interface_name,
	GVariant *parameters, gpointer user_data);

void _dbus_signal_callback(GDBusConnection *conn,
	const gchar *sender_name, const gchar *object_path,
	const gchar *interface_name, const gchar *signal_name,
	GVariant *parameters, gpointer user_data);

void _fill_transfer_property(GVariant *result);

gboolean _is_connected(const char *device_path);

gboolean _is_paired(const char *device_path);

void _get_device_address(const gchar *path, gchar **address);

artik_error bt_check_error(GError *err);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_CORE_H */
