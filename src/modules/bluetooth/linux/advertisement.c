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
#include <stdlib.h>
#include "core.h"
#include "helper.h"
#include "advertisement.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
static const gchar adv_introspection_xml[] =
"<node>"
"\t<interface name='org.bluez.LEAdvertisement1'>"
"\t\t<method name='Release'/>"
"\t\t<property type='s' name='Type' access='read'/>"
"\t\t<property type='as' name='ServiceUUIDs' access='read'/>"
"\t\t<property type='a{qv}' name='ManufacturerData' access='read'/>"
"\t\t<property type='as' name='SolicitUUIDs' access='read'/>"
"\t\t<property type='a{sv}' name='ServiceData' access='read'/>"
"\t\t<property type='b' name='IncludeTxPower' access='read'/>"
"\t</interface>"
"</node>";
#pragma GCC diagnostic pop

static GVariant *_handle_get_property(GDBusConnection * connection,
		const gchar * sender, const gchar * object_path,
		const gchar * interface_name, const gchar * property_name,
		GError * *error, gpointer user_data)
{
	GVariant *ret = NULL;
	GVariantBuilder *b, *b1;
	artik_bt_advertisement *adv_obj = user_data;
	gint i = 0;

	log_dbg("_handle_get_property");
	log_dbg("%s, %s.%s", object_path, interface_name, property_name);

	if (g_strcmp0(property_name, "Type") == 0) {
		ret = g_variant_new_string(adv_obj->type ? adv_obj->type : "");
	} else if (g_strcmp0(property_name, "ServiceUUIDs") == 0) {
		if (adv_obj->svc_uuid_len <= 0)
			return NULL;

		b = g_variant_builder_new(G_VARIANT_TYPE("as"));

		for (i = 0; i < adv_obj->svc_uuid_len; i++)
			g_variant_builder_add(b, "s", adv_obj->svc_uuid[i]);

		ret = g_variant_new("as", b);

		g_variant_builder_unref(b);
	} else if (g_strcmp0(property_name, "ManufacturerData") == 0) {
		if (adv_obj->mfr_data_len <= 0)
			return NULL;

		b = g_variant_builder_new(G_VARIANT_TYPE("a{qv}"));
		b1 = g_variant_builder_new(G_VARIANT_TYPE("ay"));

		for (i = 0; i < adv_obj->mfr_data_len; i++)
			g_variant_builder_add(b1, "y", adv_obj->mfr_data[i]);

		g_variant_builder_add(b, "{qv}", adv_obj->mfr_id,
				g_variant_new("ay", b1));

		ret = g_variant_new("a{qv}", b);

		g_variant_builder_unref(b);
		g_variant_builder_unref(b1);
	} else if (g_strcmp0(property_name, "ServiceData") == 0) {
		if (adv_obj->svc_data_len <= 0)
			return NULL;

		b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
		b1 = g_variant_builder_new(G_VARIANT_TYPE("ay"));

		for (i = 0; i < adv_obj->svc_data_len; i++)
			g_variant_builder_add(b1, "y", adv_obj->svc_data[i]);

		g_variant_builder_add(b, "{sv}", adv_obj->svc_id,
				g_variant_new("ay", b1));

		ret = g_variant_new("a{sv}", b);

		g_variant_builder_unref(b);
		g_variant_builder_unref(b1);
	} else if (g_strcmp0(property_name, "SolicitUUIDs") == 0) {
		/* TODO */
	} else if (g_strcmp0(property_name, "IncludeTxPower") == 0) {
		ret = g_variant_new_boolean(adv_obj->tx_power);
	}

	if (ret)
		print_variant(ret);

	return ret;
}

static void _handle_method_call(GDBusConnection *connection,
		const gchar *sender, const gchar *object_path,
		const gchar *interface_name, const gchar *method_name,
		GVariant *parameters, GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	log_dbg("_handle_method_call");

	g_dbus_method_invocation_return_value(invocation, NULL);
}

static void _asyn_ready_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
{
	GDBusConnection *bus = G_DBUS_CONNECTION(source_object);
	GError *e = NULL;

	g_dbus_connection_call_finish(bus, res, &e);

	if (e) {
		log_err("%s failed :%s\n", user_data, e->message);
		g_clear_error(&e);
	}
}

static const GDBusInterfaceVTable interface_vtable = {
		.method_call = _handle_method_call,
		.get_property = _handle_get_property,
		.set_property = NULL
};

artik_error bt_register_advertisement(artik_bt_advertisement *user_adv, int *id)
{
	GError *e = NULL;
	GDBusNodeInfo *node_info;
	gchar *obj_path;
	guint object_id, adv_id;
	bt_advertisement *adv_info;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	log_dbg("%s", __func__);

	node_info = g_dbus_node_info_new_for_xml(adv_introspection_xml, NULL);

	adv_id = g_slist_length(hci.advertisements);
	obj_path = g_strdup_printf("%s%d", ADVERTISEMENT_PREFIX, adv_id);

	object_id = g_dbus_connection_register_object(
			hci.conn,
			obj_path,
			node_info->interfaces[0],
			&interface_vtable,
			user_adv, NULL, &e);

	ret = bt_check_error(e);
	if (ret != S_OK)
		goto exit;

	adv_info = g_new0(bt_advertisement, 1);
	adv_info->adv_id = object_id;
	adv_info->adv_path = (char *) malloc(strlen(obj_path) + 1);
	strncpy(adv_info->adv_path, obj_path, strlen(obj_path) + 1);
	adv_info->user_data = user_adv;
	hci.advertisements = g_slist_append(hci.advertisements, adv_info);

	g_dbus_connection_call(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_LEADV_MANAGER1,
			"RegisterAdvertisement",
			g_variant_new("(oa{sv})", obj_path, NULL),
			NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
			(GAsyncReadyCallback)_asyn_ready_cb, "Register Advertisement");

	*id = adv_id;

exit:
	g_free(obj_path);

	return ret;
}

artik_error bt_unregister_advertisement(int id)
{
	bt_advertisement *adv_info;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	log_dbg("%s", __func__);

	adv_info = g_slist_nth_data(hci.advertisements, id);

	if (adv_info == NULL)
		return E_BT_ERROR;

	g_dbus_connection_call(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_LEADV_MANAGER1,
			"UnregisterAdvertisement",
			g_variant_new("(o)", adv_info->adv_path),
			NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
			(GAsyncReadyCallback)_asyn_ready_cb, "Unregister Advertisement");

	g_free(adv_info->adv_path);
	g_free(adv_info);

	return S_OK;
}
