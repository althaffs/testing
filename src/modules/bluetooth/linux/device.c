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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <gio/gio.h>
#pragma GCC diagnostic pop

#include "core.h"

static void _set_trusted(const char *device_path, gboolean value)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		device_path,
		DBUS_IF_PROPERTIES,
		"Set",
		g_variant_new("(ssv)", DBUS_IF_DEVICE1, "Trusted",
			g_variant_new_boolean(value)),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (bt_check_error(error) != S_OK)
		return;

	g_variant_unref(result);
}

static void _set_blocked(const char *device_path, gboolean value)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		device_path,
		DBUS_IF_PROPERTIES,
		"Set",
		g_variant_new("(ssv)", DBUS_IF_DEVICE1,
				"Blocked", g_variant_new_boolean(value)),
		NULL, G_DBUS_CALL_FLAGS_NONE,
		G_MAXINT, NULL, &error);

	if (bt_check_error(error) != S_OK)
		return;

	g_variant_unref(result);
}

artik_error bt_remove_unpaired_devices(void)
{
	artik_bt_device *device_list = NULL;
	int count = 0;
	int i = 0;
	artik_error ret = S_OK;

	log_dbg("");

	ret = _get_devices(BT_DEVICE_STATE_IDLE, &device_list, &count);
	if (ret != S_OK)
		return ret;

	for (i = 0; i < count; i++) {
		if (!bt_is_paired(device_list[i].remote_address))
			bt_remove_device(device_list[i].remote_address);
	}

	bt_free_devices(device_list, count);

	return S_OK;
}

artik_error bt_remove_devices(void)
{
	artik_bt_device *device_list = NULL;
	int count = 0;
	int i = 0;
	artik_error ret = S_OK;

	log_dbg("");

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	ret = _get_devices(BT_DEVICE_STATE_IDLE, &device_list, &count);
	if (ret != S_OK)
		return ret;

	for (i = 0; i < count; i++)
		bt_remove_device(device_list[i].remote_address);

	bt_free_devices(device_list, count);

	return S_OK;
}

artik_error bt_remove_device(const char *remote_address)
{
	GVariant *result = NULL;
	GError *error = NULL;
	gchar *path = NULL;
	artik_error ret = S_OK;

	log_dbg("");

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	if (_is_connected(path))
		bt_disconnect(remote_address);

	result = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		DBUS_BLUEZ_OBJECT_PATH_HCI0,
		DBUS_IF_ADAPTER1,
		"RemoveDevice",
		g_variant_new("(o)", path),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	g_free(path);

exit:
	if (result)
		g_variant_unref(result);

	return S_OK;
}

static void bt_bond_callback(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
{
	GDBusConnection *bus = G_DBUS_CONNECTION(source_object);
	gboolean paired = FALSE;
	GError *error = NULL;

	g_dbus_connection_call_finish(bus, res, &error);
	if (error == NULL) {
		paired = TRUE;
		hci.state = BT_DEVICE_STATE_PAIRED;
	} else {
		hci.state = BT_DEVICE_STATE_IDLE;
		log_err("Pair remote device failed :%s\n", error->message);
		g_clear_error(&error);
	}
	/*If pair failed, user callback should also be invoked.*/
	_user_callback(BT_EVENT_BOND, &paired);
}

artik_error bt_start_bond(const char *remote_address)
{
	gchar *path;
	gboolean bonded = FALSE;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (hci.state == BT_DEVICE_STATE_PAIRING)
		return E_IN_PROGRESS;

	if (hci.state == BT_DEVICE_STATE_PAIRED) {
		bonded = TRUE;
		_user_callback(BT_EVENT_BOND, &bonded);
		return S_OK;
	}

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	if (_is_paired(path)) {
		bonded = TRUE;
		_user_callback(BT_EVENT_BOND, &bonded);
		g_free(path);
		return S_OK;
	}

	g_dbus_connection_call(
		hci.conn,
		DBUS_BLUEZ_BUS,
		path,
		DBUS_IF_DEVICE1,
		"Pair",
		NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
		bt_bond_callback, NULL);

	hci.state = BT_DEVICE_STATE_PAIRING;

	g_free(path);

	return S_OK;
}

static void bt_connect_callback(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
{
	GDBusConnection *bus = G_DBUS_CONNECTION(source_object);
	gboolean connected = FALSE;
	GError *error = NULL;

	g_dbus_connection_call_finish(bus, res, &error);
	if (!error) {
		connected = TRUE;
		hci.state = BT_DEVICE_STATE_CONNECTED;
	} else {
		hci.state = BT_DEVICE_STATE_IDLE;
		log_err("Connect remote device failed :%s\n", error->message);
		g_clear_error(&error);
	}
	/*If connect failed, user callback should also be invoked.*/
	_user_callback(BT_EVENT_CONNECT, &connected);
}

artik_error bt_connect(const char *remote_address)
{
	gchar *path;
	gboolean connected = FALSE;

	log_dbg("");

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (hci.state == BT_DEVICE_STATE_CONNECTING)
		return E_IN_PROGRESS;

	if (hci.state == BT_DEVICE_STATE_CONNECTED) {
		connected = TRUE;
		_user_callback(BT_EVENT_CONNECT, &connected);
		return S_OK;
	}

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	if (hci.state == BT_DEVICE_STATE_IDLE
			&& _is_connected(path)) {
		connected = TRUE;
		_user_callback(BT_EVENT_CONNECT, &connected);
		g_free(path);
		return S_OK;
	}

	g_dbus_connection_call(hci.conn,
		DBUS_BLUEZ_BUS,
		path,
		DBUS_IF_DEVICE1,
		"Connect",
		NULL, NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL,
		bt_connect_callback, NULL);

	hci.state = BT_DEVICE_STATE_CONNECTING;
	g_free(path);

	return S_OK;
}

artik_error bt_connect_profile(const char *remote_address, const char *uuid)
{
	gchar *path;
	gboolean connected = FALSE;

	log_dbg("%s[%s]", __func__, uuid);

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (hci.state == BT_DEVICE_STATE_CONNECTING)
		return E_IN_PROGRESS;

	if (hci.state == BT_DEVICE_STATE_CONNECTED) {
		connected = TRUE;
		_user_callback(BT_EVENT_CONNECT, &connected);
		return S_OK;
	}

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	if (hci.state == BT_DEVICE_STATE_IDLE
			&& _is_connected(path)) {
		connected = TRUE;
		_user_callback(BT_EVENT_CONNECT, &connected);
		g_free(path);
		return S_OK;
	}

	g_dbus_connection_call(hci.conn,
		DBUS_BLUEZ_BUS,
		path,
		DBUS_IF_DEVICE1,
		"ConnectProfile",
		g_variant_new("(s)", uuid),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL,
		bt_connect_callback, NULL);

	hci.state = BT_DEVICE_STATE_CONNECTING;
	g_free(path);

	return S_OK;
}

artik_error bt_disconnect(const char *remote_address)
{
	GVariant *result;
	GError *error = NULL;
	gchar *path;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	result = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		path,
		DBUS_IF_DEVICE1,
		"Disconnect",
		NULL, NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	hci.state = BT_DEVICE_STATE_IDLE;
	g_variant_unref(result);

exit:
	g_free(path);

	return ret;
}

artik_error bt_stop_bond(const char *remote_address)
{
	GVariant *result;
	GError *error = NULL;
	gchar *path;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	result = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		path,
		DBUS_IF_DEVICE1,
		"CancelPairing",
		NULL, NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	hci.state = BT_DEVICE_STATE_IDLE;

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	g_variant_unref(result);

exit:
	g_free(path);

	return ret;
}

artik_error bt_set_trust(const char *remote_address)
{
	gchar *path;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	_set_trusted(path, TRUE);

	g_free(path);

	return S_OK;
}

artik_error bt_unset_trust(const char *remote_address)
{
	gchar *path;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	_set_trusted(path, FALSE);

	g_free(path);

	return S_OK;
}

artik_error bt_set_block(const char *remote_address)
{
	gchar *path;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	_set_blocked(path, TRUE);

	g_free(path);

	return S_OK;
}

artik_error bt_unset_block(const char *remote_address)
{
	gchar *path;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	_set_blocked(path, FALSE);

	g_free(path);

	return S_OK;
}

artik_error bt_get_devices(artik_bt_device **device_list, int *count)
{
	int cnt = 0;
	artik_bt_device *tmp_list;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	ret = _get_devices(BT_DEVICE_STATE_IDLE, &tmp_list, &cnt);
	if (ret != S_OK)
		return ret;

	*count = cnt;
	*device_list = tmp_list;

	return S_OK;
}

artik_error bt_get_paired_devices(artik_bt_device **device_list, int *count)
{
	int cnt = 0;
	artik_bt_device *tmp_list;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	ret = _get_devices(BT_DEVICE_STATE_PAIRED, &tmp_list, &cnt);
	if (ret != S_OK)
		return ret;

	*count = cnt;
	*device_list = tmp_list;

	return S_OK;
}

artik_error bt_get_connected_devices(artik_bt_device **device_list, int *count)
{
	int cnt = 0;
	artik_bt_device *tmp_list;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	ret = _get_devices(BT_DEVICE_STATE_CONNECTED, &tmp_list, &cnt);
	if (ret != S_OK)
		return ret;

	*count = cnt;
	*device_list = tmp_list;

	return S_OK;
}

artik_error bt_free_device(artik_bt_device *device)
{
	gint i = 0;

	if (device == NULL)
		return S_OK;

	if (device->uuid_length > 0) {
		for (i = 0; i < device->uuid_length; i++) {
			g_free(device->uuid_list[i].uuid);
			g_free(device->uuid_list[i].uuid_name);
		}

		g_free(device->uuid_list);
	}
	g_free(device->remote_address);
	g_free(device->remote_name);
	g_free(device);

	return S_OK;
}

artik_error bt_free_devices(artik_bt_device *device_list, int count)
{
	int i = 0, j = 0;

	for (i = 0; i < count; i++) {
		for (j = 0; j < device_list[i].uuid_length; j++) {
			g_free(device_list[i].uuid_list[j].uuid);
			g_free(device_list[i].uuid_list[j].uuid_name);
		}

		g_free(device_list[i].uuid_list);
		g_free(device_list[i].remote_address);
		g_free(device_list[i].remote_name);
	}
	g_free(device_list);

	return S_OK;
}

bool bt_is_paired(const char *remote_address)
{
	gchar *path;
	bool paired = false;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return paired;

	paired = _is_paired(path) ? true : false;

	g_free(path);

	return paired;
}

bool bt_is_connected(const char *remote_address)
{
	gchar *path;
	bool connected = false;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return connected;

	connected = _is_connected(path) ? true : false;

	g_free(path);

	return connected;
}

bool bt_is_trusted(const char *remote_address)
{
	gchar *path;
	GVariant *rst = NULL;
	GVariant *v = NULL;
	GError *error = NULL;
	bool trusted = false;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	rst = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		path,
		DBUS_IF_PROPERTIES,
		"Get",
		g_variant_new("(ss)", DBUS_IF_DEVICE1, "Trusted"),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	g_free(path);

	bt_check_error(error);

	g_variant_get(rst, "(v)", &v);
	g_variant_get(v, "b", &trusted);
	g_variant_unref(rst);
	g_variant_unref(v);

	return trusted;
}

bool bt_is_blocked(const char *remote_address)
{
	gchar *path;
	GVariant *rst = NULL;
	GVariant *v = NULL;
	GError *error = NULL;
	bool blocked = false;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(remote_address, &path);

	if (path == NULL)
		return E_BT_ERROR;

	rst = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		path,
		DBUS_IF_PROPERTIES,
		"Get",
		g_variant_new("(ss)", DBUS_IF_DEVICE1, "Blocked"),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	g_free(path);

	bt_check_error(error);

	g_variant_get(rst, "(v)", &v);
	g_variant_get(v, "b", &blocked);
	g_variant_unref(rst);
	g_variant_unref(v);

	return blocked;
}
