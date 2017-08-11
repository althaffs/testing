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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <gio/gio.h>
#pragma GCC diagnostic pop
#include <string.h>
#include <stdlib.h>
#include "core.h"
#include "gatt.h"
#include "helper.h"

static void _remove_notify(gpointer data, gpointer user_data)
{
	bt_gatt_client *client = (bt_gatt_client *)data;

	if (g_strcmp0(client->char_uuid, user_data))
		return;

	hci.gatt_clients = g_slist_remove(hci.gatt_clients, client);
	log_dbg("number of gatt client: %d", g_slist_length(hci.gatt_clients));

	g_free(client->char_uuid);
	g_free(client->srv_uuid);
	g_free(client->path);
	g_free(client);
}

artik_error _read_value(const char *itf, const char *path,
		unsigned char **byte, int *byte_len)
{
	GVariant *r = NULL, *v1 = NULL, *v2 = NULL;
	GError *e = NULL;
	guint i = 0, len = 0;

	log_dbg("bt_gatt_read_value [%s]", path);
	r = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			path,
			itf,
			"ReadValue",
			g_variant_new("(a{sv})", NULL),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	if (e != NULL) {
		log_dbg("%s", e->message);
		g_error_free(e);
		return E_BT_ERROR;
	}

	v1 = g_variant_get_child_value(r, 0);
	len = g_variant_n_children(v1);
	*byte_len = len;

	*byte = (unsigned char *)malloc(sizeof(unsigned char) * len);
	for (i = 0; i < len; i++) {
		v2 = g_variant_get_child_value(v1, i);
		(*byte)[i] = g_variant_get_byte(v2);
	}

	g_variant_unref(r);
	g_variant_unref(v1);
	g_variant_unref(v2);

	return S_OK;
}

artik_error _write_value(const char *itf, const char *path,
		const unsigned char byte[], int byte_len)
{
	GVariantBuilder *b;
	GError *e = NULL;
	gint i = 0;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));


	b = g_variant_builder_new(G_VARIANT_TYPE("ay"));
	for (i = 0; i < byte_len; i++)
		g_variant_builder_add(b, "y", byte[i]);

	log_dbg("%s [%s]", __func__, path);
	g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			path,
			itf,
			"WriteValue",
			g_variant_new("(aya{sv})", b, NULL),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	if (e != NULL) {
		log_dbg("%s", e->message);
		g_error_free(e);
		return E_BT_ERROR;
	}

	g_variant_builder_unref(b);

	return S_OK;
}

artik_error bt_gatt_char_read_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		unsigned char **byte, int *byte_len)
{
	gchar *srv_path, *char_path;

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &char_path);
	g_free(srv_path);
	if (char_path == NULL)
		return E_BT_ERROR;


	artik_error err =  _read_value(DBUS_IF_GATTCHARACTERISTIC1, char_path, byte, byte_len);

	g_free(char_path);
	return err;
}

artik_error bt_gatt_char_write_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		const unsigned char byte[], int byte_len)
{
	gchar *srv_path, *char_path;

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &char_path);
	g_free(srv_path);
	if (char_path == NULL)
		return E_BT_ERROR;


	artik_error err = _write_value(DBUS_IF_GATTCHARACTERISTIC1, char_path, byte, byte_len);

	g_free(char_path);
	return err;
}

artik_error bt_gatt_desc_read_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		const char *desc_uuid, unsigned char **byte, int *byte_len)
{
	gchar *srv_path, *char_path, *desc_path;

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &char_path);
	g_free(srv_path);
	if (char_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTDESCRIPTOR1, desc_uuid, "Characteristic", char_path, &desc_path);
	g_free(char_path);
	if (desc_path == NULL)
		return E_BT_ERROR;

	artik_error err = _read_value(DBUS_IF_GATTDESCRIPTOR1, desc_path, byte, byte_len);

	g_free(desc_path);
	return err;
}

artik_error bt_gatt_desc_write_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		const char *desc_uuid, const unsigned char byte[], int byte_len)
{
	gchar *srv_path, *char_path, *desc_path;

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &char_path);
	g_free(srv_path);
	if (char_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTDESCRIPTOR1, desc_uuid, "Characteristic", char_path, &desc_path);
	g_free(char_path);
	if (desc_path == NULL)
		return E_BT_ERROR;

	artik_error err = _write_value(DBUS_IF_GATTDESCRIPTOR1, desc_path, byte, byte_len);

	g_free(desc_path);
	return err;
}

artik_error bt_gatt_start_notify(const char *addr, const char *srv_uuid, const char *char_uuid)
{
	gchar *path = NULL;
	gchar *srv_path = NULL;
	bt_gatt_client *client;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &path);
	g_free(srv_path);
	if (path == NULL)
		return E_BT_ERROR;

	client = g_new0(bt_gatt_client, 1);
	client->srv_uuid = g_strdup(srv_uuid);
	client->char_uuid = g_strdup(char_uuid);
	client->path = g_strdup(path);
	hci.gatt_clients = g_slist_append(hci.gatt_clients, client);

	log_dbg("%s [%s]", __func__, path);
	g_dbus_connection_call(
			hci.conn,
			DBUS_BLUEZ_BUS,
			path,
			DBUS_IF_GATTCHARACTERISTIC1,
			"StartNotify",
			NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1,
			NULL, NULL, NULL);

	g_free(path);

	return S_OK;
}

artik_error bt_gatt_stop_notify(const char *addr, const char *srv_uuid, const char *char_uuid)
{
	gchar *path = NULL;
	gchar *srv_path = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &path);
	g_free(srv_path);
	if (path == NULL)
		return E_BT_ERROR;

	log_dbg("%s [%s]", __func__, path);
	g_dbus_connection_call(
			hci.conn,
			DBUS_BLUEZ_BUS,
			path,
			DBUS_IF_GATTCHARACTERISTIC1,
			"StopNotify",
			NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1,
			NULL, NULL, NULL);

	g_slist_foreach(hci.gatt_clients, _remove_notify, (char *)char_uuid);

	g_free(path);

	return S_OK;
}

artik_error bt_gatt_get_char_properties(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_gatt_char_properties *properties)
{
	GVariant *r, *v;
	GVariantIter *iter;
	GError *e = NULL;
	gchar *path = NULL, *str, *srv_path = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;

	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &path);
	g_free(srv_path);
	if (path == NULL)
		return E_BT_ERROR;

	log_dbg("bt_gatt_get_char_properties [%s]", path);
	r = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			path,
			DBUS_IF_PROPERTIES,
			"Get",
			g_variant_new("(ss)", DBUS_IF_GATTCHARACTERISTIC1, "Flags"),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	if (e != NULL) {
		log_dbg("%s", e->message);
		g_error_free(e);
		g_free(path);
		return E_BT_ERROR;
	}

	g_variant_get(r, "(v)", &v);
	g_variant_get(v, "as", &iter);
	while (g_variant_iter_loop(iter, "&s", &str)) {
		if (!g_strcmp0(str, "broadcast"))
			*properties |= BT_GATT_CHAR_PROPERTY_BROADCAST;
		else if (!g_strcmp0(str, "read"))
			*properties |= BT_GATT_CHAR_PROPERTY_READ;
		else if (!g_strcmp0(str, "write-without-response"))
			*properties |= BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE;
		else if (!g_strcmp0(str, "write"))
			*properties |= BT_GATT_CHAR_PROPERTY_WRITE;
		else if (!g_strcmp0(str, "notify"))
			*properties |= BT_GATT_CHAR_PROPERTY_NOTIFY;
		else if (!g_strcmp0(str, "indicate"))
			*properties |= BT_GATT_CHAR_PROPERTY_INDICATE;
		else if (!g_strcmp0(str, "authenticated-signed-writes"))
			*properties |= BT_GATT_CHAR_PROPERTY_SIGNED_WRITE;
		else
			log_dbg("extended properties(%s)", str);
	}

	g_variant_unref(r);
	g_variant_unref(v);
	g_variant_iter_free(iter);
	g_free(path);

	return S_OK;
}

artik_error bt_gatt_get_desc_properties(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, artik_bt_gatt_desc_properties *properties)
{
	GVariant *r, *v;
	GVariantIter *iter;
	GError *e = NULL;
	gchar *srv_path, *char_path, *desc_path, *str;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &char_path);
	g_free(srv_path);
	if (char_path == NULL)
		return E_BT_ERROR;
	_get_gatt_path(addr, DBUS_IF_GATTDESCRIPTOR1, desc_uuid, "Characteristic", char_path, &desc_path);
	g_free(char_path);
	if (desc_path == NULL)
		return E_BT_ERROR;

	log_dbg("%s [%s]", __func__, desc_path);
	r = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			desc_path,
			DBUS_IF_PROPERTIES,
			"Get",
			g_variant_new("(ss)", DBUS_IF_GATTDESCRIPTOR1, "Flags"),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	if (e != NULL) {
		log_dbg("%s", e->message);
		g_error_free(e);
		g_free(desc_path);
		return E_BT_ERROR;
	}

	g_variant_get(r, "(v)", &v);
	g_variant_get(v, "as", &iter);
	while (g_variant_iter_loop(iter, "&s", &str)) {
		if (g_strcmp0(str, "read"))
			*properties |= BT_GATT_DESC_PROPERTY_READ;
		else if (g_strcmp0(str, "write"))
			*properties |= BT_GATT_DESC_PROPERTY_WRITE;
		else if (g_strcmp0(str, "encrypt-read"))
			*properties |= BT_GATT_DESC_PROPERTY_ENC_READ;
		else if (g_strcmp0(str, "encrypt-write"))
			*properties |= BT_GATT_DESC_PROPERTY_ENC_WRITE;
		else if (g_strcmp0(str, "encrypt-authenticated-read"))
			*properties |= BT_GATT_DESC_PROPERTY_ENC_AUTH_READ;
		else if (g_strcmp0(str, "encrypt-authenticated-write"))
			*properties |= BT_GATT_DESC_PROPERTY_ENC_AUTH_WRITE;
		else if (g_strcmp0(str, "secure-read"))
			*properties |= BT_GATT_DESC_PROPERTY_SEC_READ;
		else if (g_strcmp0(str, "secure-write"))
			*properties |= BT_GATT_DESC_PROPERTY_SEC_WRITE;
		else
			log_dbg("extended properties(%s)", str);
	}

	g_variant_unref(r);
	g_variant_unref(v);
	g_variant_iter_free(iter);
	g_free(desc_path);

	return S_OK;
}
