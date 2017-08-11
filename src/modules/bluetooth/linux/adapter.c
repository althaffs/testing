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
#include "adapter.h"
#include "device.h"

artik_error bt_set_scan_filter(artik_bt_scan_filter * filter)
{
	GError *e = NULL;
	GVariantBuilder *b, *b1;
	GVariant *f;
	gchar *type;
	guint i;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (filter != NULL) {
		log_dbg("bt_set_scan_filter");

		b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
		b1 = g_variant_builder_new(G_VARIANT_TYPE("as"));

		if (filter->type == 1)
			type = "bredr";
		else if (filter->type == 2)
			type = "le";
		else
			type = "auto";

		log_dbg("type: %s", type);
		g_variant_builder_add(b, "{sv}", "Transport",
				g_variant_new_string(type));

		if (filter->uuid_length < 1) {
			filter->uuid_length = 0;
			filter->uuid_list = NULL;
		}

		for (i = 0; i < filter->uuid_length; i++) {
			g_variant_builder_add(b1, "s", filter->uuid_list[i].uuid);
			log_dbg("uuid: %s", filter->uuid_list[i].uuid);
		}
		g_variant_builder_add(b, "{sv}", "UUIDs", g_variant_new("as", b1));

		g_variant_builder_add(b, "{sv}", "RSSI",
				g_variant_new_int16(filter->rssi));
		log_dbg("rssi: %d", filter->rssi);

		f = g_variant_new("(a{sv})", b);

		g_variant_builder_unref(b);
		g_variant_builder_unref(b1);
	} else {
		log_dbg("%s: remove filter", __func__);
		f = g_variant_new("(a{sv})", NULL);
	}

	g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_ADAPTER1,
			"SetDiscoveryFilter",
			f, NULL,
			G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	return bt_check_error(e);
}

artik_error bt_set_alias(const char *alias)
{
	GVariant *v;
	GError *e = NULL;

	if (alias == NULL)
		return S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	v = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_PROPERTIES,
			"Set",
			g_variant_new("(ssv)", DBUS_IF_ADAPTER1, "Alias",
					g_variant_new_string(alias)),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	g_variant_unref(v);

	return bt_check_error(e);
}

artik_error bt_set_powered(bool powered)
{
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_PROPERTIES,
			"Set",
			g_variant_new("(ssv)", DBUS_IF_ADAPTER1, "Powered",
					g_variant_new_boolean(powered)),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	if (e) {
		log_err("Set powered failed: %s", e->message);
		g_error_free(e);
		return E_BT_ERROR;
	}
	return S_OK;
}

artik_error bt_set_discoverable(bool discoverable)
{
	GVariant *v;
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	v = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_PROPERTIES,
			"Set",
			g_variant_new("(ssv)", DBUS_IF_ADAPTER1, "Discoverable",
					g_variant_new_boolean(discoverable)),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	g_variant_unref(v);

	return bt_check_error(e);
}

artik_error bt_set_pairable(bool pairable)
{
	GVariant *v;
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	v = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_PROPERTIES,
			"Set",
			g_variant_new("(ssv)", DBUS_IF_ADAPTER1, "Pairable",
					g_variant_new_boolean(pairable)),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	g_variant_unref(v);

	return bt_check_error(e);
}

artik_error bt_set_pairableTimeout(unsigned int timeout)
{
	GVariant *v;
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	v = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_PROPERTIES,
			"Set",
			g_variant_new("(ssv)", DBUS_IF_ADAPTER1, "PairableTimeout",
					g_variant_new_uint32(timeout)),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	g_variant_unref(v);

	return bt_check_error(e);
}

artik_error bt_set_discoverableTimeout(unsigned int timeout)
{
	GVariant *v;
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	v = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_PROPERTIES,
			"Set",
			g_variant_new("(ssv)", DBUS_IF_ADAPTER1, "DiscoverableTimeout",
					g_variant_new_uint32(timeout)),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	g_variant_unref(v);

	return bt_check_error(e);
}

artik_error bt_start_scan(void)
{
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	log_dbg("");

	bt_remove_unpaired_devices();

	log_dbg("bt_start_scan");

	if (bt_is_scanning())
		return E_BT_ERROR;

	g_dbus_connection_call_sync(hci.conn, DBUS_BLUEZ_BUS,
		DBUS_BLUEZ_OBJECT_PATH_HCI0, DBUS_IF_ADAPTER1,
		"StartDiscovery", NULL, G_VARIANT_TYPE("()"),
		G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	return bt_check_error(e);
}

artik_error bt_stop_scan(void)
{
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	log_dbg("");

	if (!bt_is_scanning())
		return E_BT_ERROR;

	g_dbus_connection_call_sync(hci.conn, DBUS_BLUEZ_BUS,
		DBUS_BLUEZ_OBJECT_PATH_HCI0,
		DBUS_IF_ADAPTER1, "StopDiscovery", NULL,
		G_VARIANT_TYPE("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &e);

	return bt_check_error(e);
}

bool bt_is_scanning(void)
{
	GError *e = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	GVariant *rst, *v;
	gboolean b;

	rst = g_dbus_connection_call_sync(hci.conn, DBUS_BLUEZ_BUS,
		DBUS_BLUEZ_OBJECT_PATH_HCI0, DBUS_IF_PROPERTIES, "Get",
		g_variant_new("(ss)", DBUS_IF_ADAPTER1, "Discovering"),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	if (bt_check_error(e) != S_OK)
		return false;

	g_variant_get(rst, "(v)", &v);
	g_variant_get(v, "b", &b);
	g_variant_unref(rst);
	g_variant_unref(v);

	return b;
}

artik_error bt_get_device_property(const char *addr, const char *property, char **value)
{
	GVariant *v, *tuple;
	GError *e = NULL;
	artik_error ret = S_OK;
	gchar *path = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(addr, &path);

	tuple = g_dbus_connection_call_sync(hci.conn, DBUS_BLUEZ_BUS,
		path, DBUS_IF_PROPERTIES, "Get",
		g_variant_new("(ss)", DBUS_IF_DEVICE1, property),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	ret = bt_check_error(e);
	if (ret != S_OK)
		return ret;

	g_variant_get(tuple, "(v)", &v);
	g_variant_get(v, "s", value);

	return S_OK;
}

artik_error bt_get_adapter_info(artik_bt_adapter *adapter)
{
	GVariant *r, *v;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (adapter == NULL)
		return E_BT_ERROR;

	r = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_PROPERTIES, "GetAll",
			g_variant_new("(s)", DBUS_IF_ADAPTER1),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, NULL);

	v = g_variant_get_child_value(r, 0);
	_get_adapter_properties(v, adapter);

	g_variant_unref(r);
	g_variant_unref(v);

	return S_OK;
}
