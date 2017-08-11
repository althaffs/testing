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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
static const gchar service_introspection_xml[] =
"<node name='/'>"
"\t<interface name='org.freedesktop.DBus.ObjectManager'>"
"\t\t<method name='GetManagedObjects'>"
"\t\t\t<arg type='a{oa{sa{sv}}}' name='object_paths_interfaces_and_properties' direction='out'/>"
"\t\t</method>"
"\t</interface>"
"\t<interface name='org.bluez.GattService1'>"
"\t\t<property type='s' name='UUID' access='read'>"
"\t\t</property>"
"\t\t<property type='b' name='primary' access='read'>"
"\t\t</property>"
"\t\t<property type='o' name='Device' access='read'>"
"\t\t</property>"
"\t\t<property type='ao' name='Characteristics' access='read'>"
"\t\t</property>"
"\t\t<property type='s' name='Includes' access='read'>"
"\t\t</property>"
"\t</interface>"
"</node>";

static const gchar char_introspection_xml[] =
"<node name='/'>"
"\t<interface name='org.bluez.GattCharacteristic1'>"
"\t\t<method name='ReadValue'>"
"\t\t\t<arg type='a{sv}' name='options' direction='in'/>"
"\t\t<arg type='ay' name='Value' direction='out'/>"
"\t\t</method>"
"\t\t<method name='WriteValue'>"
"\t\t\t<arg type='ay' name='value' direction='in'/>"
"\t\t\t<arg type='a{sv}' name='options' direction='in'/>"
"\t\t</method>"
"\t\t<method name='StartNotify'>"
"\t\t</method>"
"\t\t<method name='StopNotify'>"
"\t\t</method>"
"\t\t<method name='IndicateConfirm'>"
"\t\t\t<arg type='s' name='address' direction='in'/>"
"\t\t\t<arg type='b' name='complete' direction='in'/>"
"\t\t</method>"
"\t</interface>"
"\t<interface name='org.freedesktop.DBus.Properties'>"
"\t\t<property type='s' name='UUID' access='read'>"
"\t\t</property>"
"\t\t<property type='o' name='Service' access='read'>"
"\t\t</property>"
"\t\t<property type='ay' name='Value' access='readwrite'>"
"\t\t</property>"
"\t\t<property type='b' name='Notifying' access='read'>"
"\t\t</property>"
"\t\t<property type='as' name='Flags' access='read'>"
"\t\t</property>"
"\t\t<property type='s' name='Unicast' access='read'>"
"\t\t</property>"
"\t\t<property type='ao' name='Descriptors' access='read'>"
"\t\t</property>"
"\t</interface>"
"</node>";

static const gchar descriptor_introspection_xml[] =
"<node name='/'>"
"\t<interface name='org.bluez.GattDescriptor1'>"
"\t\t<method name='ReadValue'>"
"\t\t\t<arg type='a{sv}' name='options' direction='in'/>"
"\t\t\t<arg type='ay' name='Value' direction='out'/>"
"\t\t</method>"
"\t\t<method name='WriteValue'>"
"\t\t\t<arg type='ay' name='value' direction='in'/>"
"\t\t\t<arg type='a{sv}' name='options' direction='in'/>"
"\t\t</method>"
"\t</interface>"
"\t<interface name='org.freedesktop.DBus.Properties'>"
"\t\t<property type='s' name='UUID' access='read'>"
"\t\t</property>"
"\t\t<property type='o' name='Characteristic' access='read'>"
"\t\t</property>"
"\t\t<property type='ay' name='Value' access='read'>"
"\t\t</property>"
"\t\t<property type='as' name='Flags' access='read'>"
"\t\t</property>"
"\t\t<property type='s' name='Permissions' access='read'>"
"\t\t</property>"
"\t</interface>"
"</node>";
#pragma GCC diagnostic pop

static void _serv_method_call(GDBusConnection * connection, const gchar *sender,
		const gchar *object_path, const gchar *interface_name,
		const gchar *method_name, GVariant * parameters,
		GDBusMethodInvocation * invocation, gpointer user_data)
{
	GVariantBuilder *b, *b_svc, *b_svc1, *b_svc11;
	GVariantBuilder *b_char, *b_char1, *b_char11, *b_char12, *b_char13;
	GVariantBuilder *b_desc, *b_desc1, *b_desc11, *b_desc12;
	GSList *l1, *l2;
	gboolean notify = FALSE;
	guint serv_id, i;
	gchar *unicast = "00:00:00:00:00:00";
	bt_gatt_service *serv_info;
	bt_gatt_char *char_info;
	bt_gatt_desc *desc_info;

	if (g_strcmp0(method_name, "GetManagedObjects") == 0) {
		log_dbg("getting values for service, chars and descriptors");

		if (!g_str_has_prefix(object_path, GATT_SERVICE_PREFIX))
			g_dbus_method_invocation_return_value(invocation, NULL);

		serv_id = strtol(object_path + strlen(GATT_SERVICE_PREFIX), NULL, 10);
		serv_info = g_slist_nth_data(hci.gatt_services, serv_id);

		b = g_variant_builder_new(G_VARIANT_TYPE("a{oa{sa{sv}}}"));
		b_svc = g_variant_builder_new(G_VARIANT_TYPE("a{sa{sv}}"));
		b_svc1 = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
		b_svc11 = g_variant_builder_new(G_VARIANT_TYPE("ao"));

		g_variant_builder_add(b_svc1, "{sv}", "UUID",
				g_variant_new_string(serv_info->service_uuid));
		g_variant_builder_add(b_svc1, "{sv}", "Primary",
				g_variant_new_boolean(serv_info->is_svc_primary));

		for (l1 = serv_info->char_data; l1 != NULL; l1 = l1->next) {
			char_info = l1->data;
			g_variant_builder_add(b_svc11, "o", char_info->char_path);
		}
		g_variant_builder_add(b_svc1, "{sv}", "Characteristics",
				g_variant_new("ao", b_svc11));
		g_variant_builder_add(b_svc, "{sa{sv}}",
				DBUS_IF_GATTSERVICE1, b_svc1);
		g_variant_builder_add(b, "{oa{sa{sv}}}",
				serv_info->serv_path, b_svc);

		for (l1 = serv_info->char_data; l1 != NULL; l1 = l1->next) {
			char_info = l1->data;

			b_char = g_variant_builder_new(G_VARIANT_TYPE("a{sa{sv}}"));
			b_char1 = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
			b_char11 = g_variant_builder_new(G_VARIANT_TYPE("ay"));
			b_char12 = g_variant_builder_new(G_VARIANT_TYPE("as"));
			b_char13 = g_variant_builder_new(G_VARIANT_TYPE("ao"));

			g_variant_builder_add(b_char1, "{sv}", "UUID",
					g_variant_new_string(char_info->char_uuid));
			g_variant_builder_add(b_char1, "{sv}", "Service",
					g_variant_new("o", serv_info->serv_path));

			if (char_info->char_value != NULL) {
				for (i = 0; i < char_info->value_length; i++)
					g_variant_builder_add(b_char11, "y",
							char_info->char_value[i]);

				g_variant_builder_add(b_char1, "{sv}", "Value",
						g_variant_new("ay", b_char11));
			}

			for (i = 0; i < g_slist_length(char_info->char_props); i++)
				g_variant_builder_add(b_char12, "s",
						g_slist_nth_data(char_info->char_props, i));

			g_variant_builder_add(b_char1, "{sv}", "Flags",
					g_variant_new("as", b_char12));
			g_variant_builder_add(b_char1, "{sv}", "Notifying",
					g_variant_new("b", notify));
			g_variant_builder_add(b_char1, "{sv}", "Unicast",
					g_variant_new("s", unicast));

			for (l2 = char_info->desc_data; l2 != NULL; l2 = l2->next) {
				desc_info = l2->data;
				g_variant_builder_add(b_char13, "o", desc_info->desc_path);
			}
			g_variant_builder_add(b_char1, "{sv}", "Descriptors",
					g_variant_new("ao", b_char13));

			g_variant_builder_add(b_char, "{sa{sv}}",
					DBUS_IF_GATTCHARACTERISTIC1, b_char1);
			g_variant_builder_add(b, "{oa{sa{sv}}}",
					char_info->char_path, b_char);

			for (l2 = char_info->desc_data; l2 != NULL; l2 = l2->next) {
				desc_info = l2->data;

				b_desc = g_variant_builder_new(G_VARIANT_TYPE("a{sa{sv}}"));
				b_desc1 = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
				b_desc11 = g_variant_builder_new(G_VARIANT_TYPE("ay"));
				b_desc12 = g_variant_builder_new(G_VARIANT_TYPE("as"));

				g_variant_builder_add(b_desc1, "{sv}", "UUID",
					g_variant_new_string(desc_info->desc_uuid));

				g_variant_builder_add(b_desc1, "{sv}", "Characteristic",
					g_variant_new("o", char_info->char_path));

				if (desc_info->desc_value != NULL) {
					for (i = 0; i < desc_info->value_length; i++)
						g_variant_builder_add(b_desc11, "y",
								desc_info->desc_value[i]);

					g_variant_builder_add(b_desc1, "{sv}", "Value",
							g_variant_new("ay", b_desc11));
				}

				for (i = 0; i < g_slist_length(desc_info->desc_props); i++)
					g_variant_builder_add(b_desc12, "s",
							g_slist_nth_data(desc_info->desc_props, i));

				g_variant_builder_add(b_desc1, "{sv}", "Flags",
						g_variant_new("as", b_desc12));

				g_variant_builder_add(b_desc, "{sa{sv}}",
						DBUS_IF_GATTDESCRIPTOR1, b_desc1);
				g_variant_builder_add(b, "{oa{sa{sv}}}", desc_info->desc_path,
						b_desc);

				g_variant_builder_unref(b_desc);
				g_variant_builder_unref(b_desc1);
				g_variant_builder_unref(b_desc11);
			}

			g_variant_builder_unref(b_char);
			g_variant_builder_unref(b_char1);
			g_variant_builder_unref(b_char11);
			g_variant_builder_unref(b_char12);
			g_variant_builder_unref(b_char13);
		}

		g_dbus_method_invocation_return_value(invocation,
						g_variant_new("(a{oa{sa{sv}}})", b));

		g_variant_builder_unref(b);
		g_variant_builder_unref(b_svc);
		g_variant_builder_unref(b_svc1);
		g_variant_builder_unref(b_svc11);
	}
}

static gint _compare_char_path(gconstpointer a, gconstpointer b)
{
	if (strcmp(((bt_gatt_char *)a)->char_path, b) == 0)
		return 0;
	else
		return 1;
}

static gint _compare_desc_path(gconstpointer a, gconstpointer b)
{
	if (strcmp(((bt_gatt_desc *)a)->desc_path, b) == 0)
		return 0;
	else
		return 1;
}

static bt_gatt_char *_find_chr_list_by_id(unsigned int svc_id, unsigned int char_id)
{
	bt_gatt_service *svc = NULL;
	bt_gatt_char *chr = NULL;

	if (g_slist_length(hci.gatt_services) < svc_id) {
		log_dbg("%s: GATT service with id %d not found", __func__, svc_id);
		return NULL;
	}

	svc = g_slist_nth_data(hci.gatt_services, svc_id);
	if (g_slist_length(svc->char_data) < char_id) {
		log_dbg("%s: GATT characteristic with id %d not found", __func__, char_id);
		return NULL;
	}

	chr = g_slist_nth_data(svc->char_data, char_id);

	return chr;
}

static bt_gatt_desc *_find_desc_list_by_id(unsigned int svc_id, unsigned int char_id, unsigned int desc_id)
{
	bt_gatt_char *chr = _find_chr_list_by_id(svc_id, char_id);
	bt_gatt_desc *desc = NULL;

	if (!chr)
		return NULL;

	if (g_slist_length(chr->desc_data) < desc_id) {
		log_dbg("%s: GATT descriptor with id %d not found", __func__, desc_id);
		return NULL;
	}

	desc = g_slist_nth_data(chr->desc_data, desc_id);
	return desc;
}

static GSList *_find_chr_list_by_path(const gchar *path)
{
	GSList *l, *l1;
	bt_gatt_service *svc;

	for (l1 = hci.gatt_services; l1; l1 = g_slist_next(l1)) {
		svc = l1->data;
		l = g_slist_find_custom(svc->char_data, path, _compare_char_path);
		if (l)
			return l;
	}

	log_dbg("%s not found chracteristic in %s", __func__, path);

	return NULL;
}

static GSList *_find_desc_list(const gchar *path)
{
	GSList *l, *l1, *l2;
	bt_gatt_service *svc;
	bt_gatt_char *chr;

	for (l1 = hci.gatt_services; l1; l1 = g_slist_next(l1)) {
		svc = l1->data;

		for (l2 = svc->char_data; l2; l2 = g_slist_next(l2)) {
			chr = l2->data;

			l = g_slist_find_custom(chr->desc_data, path, _compare_desc_path);
			if (l)
				return l;
		}
	}

	log_dbg("%s not found descriptor in %s", __func__, path);

	return NULL;
}

static void _send_request(bt_gatt_req_handle *handle, guint len, const guchar *value)
{
	GVariantBuilder *b;

	b = g_variant_builder_new(G_VARIANT_TYPE("ay"));

	for (guint i = 0; i < len; i++)
		g_variant_builder_add(b, "y", value[i]);

	g_dbus_method_invocation_return_value(handle->invocation,
			g_variant_new("(ay)", b));

	g_variant_builder_unref(b);
}

static void _extract_value_parameter(GVariant *parameters, unsigned int *len, guchar **value)
{
	GVariant *v1 = NULL, *v2 = NULL;

	g_variant_get(parameters, "(@aya{sv})", &v1, NULL);
	*len = g_variant_n_children(v1);
	guchar *extracted_value = (guchar *)malloc(sizeof(guchar) * *len);

	for (guint i = 0; i < *len; i++) {
		v2 = g_variant_get_child_value(v1, i);
		extracted_value[i] = g_variant_get_byte(v2);
	}

	*value = extracted_value;
	g_variant_unref(v1);
	g_variant_unref(v2);
}

static void _char_method_call(GDBusConnection *connection, const gchar *sender,
		const gchar *object_path, const gchar *interface_name,
		const gchar *method_name, GVariant *parameters,
		GDBusMethodInvocation *invocation, gpointer user_data)
{
	bt_gatt_char *chr;
	GSList *l;

	log_dbg("%s", method_name);

	l = _find_chr_list_by_path(object_path);
	if (l == NULL) {
		log_dbg("there is no characteristic");
		g_dbus_method_invocation_return_value(invocation, NULL);
		return;
	}
	chr = g_slist_nth_data(l, 0);

	if (g_strcmp0(method_name, "ReadValue") == 0) {
		bt_gatt_req_handle *handle = malloc(sizeof(bt_gatt_req_handle));

		memset(handle, 0, sizeof(bt_gatt_req_handle));
		handle->chr = chr;
		handle->invocation = invocation;
		handle->type = BT_GATT_REQ_TYPE_READ;

		if (chr->read_callback)
			chr->read_callback(handle, chr->read_user_data);
		else {
			_send_request(handle, chr->value_length, chr->char_value);
			free(handle);
		}
	} else if (g_strcmp0(method_name, "WriteValue") == 0) {
		bt_gatt_req_handle *handle = malloc(sizeof(bt_gatt_req_handle));

		memset(handle, 0, sizeof(bt_gatt_req_handle));
		handle->chr = chr;
		handle->type = BT_GATT_REQ_TYPE_WRITE;
		handle->invocation = invocation;

		_extract_value_parameter(parameters, &handle->len, &handle->value);

		if (chr->write_callback)
			chr->write_callback(handle, handle->value,
				handle->len, chr->write_user_data);
		else
			bt_gatt_req_set_result(handle, BT_GATT_REQ_STATE_TYPE_OK, NULL);

	} else if (g_strcmp0(method_name, "StartNotify") == 0) {
		if (chr->notify_callback)
			chr->notify_callback(true, chr->notify_user_data);

	} else if (g_strcmp0(method_name, "StopNotify") == 0) {
		if (chr->notify_callback)
			chr->notify_callback(false, chr->notify_user_data);

	} else if (g_strcmp0(method_name, "IndicateConfirm") == 0) {
		/* TODO */
	}
}

static void _desc_method_call(GDBusConnection *connection,
		const gchar *sender, const gchar *object_path,
		const gchar *interface_name, const gchar *method_name,
		GVariant *parameters, GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	bt_gatt_desc *desc;
	GSList *l;

	log_dbg("%s", method_name);

	l = _find_desc_list(object_path);
	if (l == NULL) {
		log_dbg("there is no descriptor");
		g_dbus_method_invocation_return_value(invocation, NULL);
		return;
	}
	desc = g_slist_nth_data(l, 0);

	bt_gatt_req_handle *handle = malloc(sizeof(bt_gatt_req_handle));

	memset(handle, 0, sizeof(bt_gatt_req_handle));
	handle->chr = NULL;
	handle->desc = desc;
	handle->invocation = invocation;

	if (g_strcmp0(method_name, "ReadValue") == 0) {
		handle->type = BT_GATT_REQ_TYPE_READ;

		if (desc->read_callback)
			desc->read_callback(handle, desc->read_user_data);
		else {
			_send_request(handle, desc->value_length, desc->desc_value);
			free(handle);
		}

	} else if (g_strcmp0(method_name, "WriteValue") == 0) {
		handle->type = BT_GATT_REQ_TYPE_WRITE;

		_extract_value_parameter(parameters, &handle->len, &handle->value);

		if (desc->write_callback)
			desc->write_callback(handle, handle->value, handle->len, desc->write_user_data);
		else
			bt_gatt_req_set_result(handle, BT_GATT_REQ_STATE_TYPE_OK, NULL);
	}
}

static void register_service_cb(GObject *object, GAsyncResult *res,
		gpointer user_data)
{
	GError *e = NULL;

	g_dbus_connection_call_finish(hci.conn, res, &e);
	if (e != NULL) {
		log_dbg("%s", e->message);
		g_clear_error(&e);
	}
	log_dbg("gatt service registered");
}

static const GDBusInterfaceVTable serv_interface_vtable = {
	.method_call = _serv_method_call,
	.get_property = NULL,
	.set_property = NULL,
};

static const GDBusInterfaceVTable char_interface_vtable = {
	.method_call = _char_method_call,
	.get_property = NULL,
	.set_property = NULL,
};

static const GDBusInterfaceVTable desc_interface_vtable = {
	.method_call = _desc_method_call,
	.get_property = NULL,
	.set_property = NULL,
};

static void _set_char_properties(int properties, GSList **list)
{
	if (properties == 0x00) {
		*list = g_slist_append(*list, g_strdup("read"));
		return;
	}

	if (properties & BT_GATT_CHAR_PROPERTY_BROADCAST)
		*list = g_slist_append(*list, g_strdup("broadcast"));

	if (properties & BT_GATT_CHAR_PROPERTY_READ)
		*list = g_slist_append(*list, g_strdup("read"));

	if (properties & BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE)
		*list = g_slist_append(*list, g_strdup("write-without-response"));

	if (properties & BT_GATT_CHAR_PROPERTY_WRITE)
		*list = g_slist_append(*list, g_strdup("write"));

	if (properties & BT_GATT_CHAR_PROPERTY_NOTIFY)
		*list = g_slist_append(*list, g_strdup("notify"));

	if (properties & BT_GATT_CHAR_PROPERTY_INDICATE)
		*list = g_slist_append(*list, g_strdup("indicate"));

	if (properties & BT_GATT_CHAR_PROPERTY_SIGNED_WRITE)
		*list = g_slist_append(*list, g_strdup("authenticated-signed-writes"));
}

static void _set_desc_properties(int properties, GSList **list)
{
	if (properties == 0x00) {
		*list = g_slist_append(*list, g_strdup("read"));
		return;
	}

	if (properties & BT_GATT_DESC_PROPERTY_READ)
		*list = g_slist_append(*list, g_strdup("read"));

	if (properties & BT_GATT_DESC_PROPERTY_WRITE)
		*list = g_slist_append(*list, g_strdup("write"));

	if (properties & BT_GATT_DESC_PROPERTY_ENC_READ)
		*list = g_slist_append(*list, g_strdup("encrypt-read"));

	if (properties & BT_GATT_DESC_PROPERTY_ENC_WRITE)
		*list = g_slist_append(*list, g_strdup("encrypt-write"));

	if (properties & BT_GATT_DESC_PROPERTY_ENC_AUTH_READ)
		*list = g_slist_append(*list, g_strdup("encrypt-authenticated-read"));

	if (properties & BT_GATT_DESC_PROPERTY_ENC_AUTH_WRITE)
		*list = g_slist_append(*list, g_strdup("encrypt-authenticated-write"));

	if (properties & BT_GATT_DESC_PROPERTY_SEC_READ)
		*list = g_slist_append(*list, g_strdup("secure-read"));

	if (properties & BT_GATT_DESC_PROPERTY_SEC_WRITE)
		*list = g_slist_append(*list, g_strdup("secure-write"));
}

int bt_gatt_add_service(artik_bt_gatt_service svc, int *id)
{
	GError *error = NULL;
	guint object_id, serv_id;
	GDBusNodeInfo *node_info = NULL;
	gchar *path = NULL;
	bt_gatt_service *serv_info = NULL;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	log_dbg("bt_gatt_add_service");

	node_info = g_dbus_node_info_new_for_xml(service_introspection_xml, &error);
	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	serv_id = g_slist_length(hci.gatt_services);
	path = g_strdup_printf("%s%d", GATT_SERVICE_PREFIX, serv_id);

	object_id = g_dbus_connection_register_object(hci.conn, path,
			node_info->interfaces[0], &serv_interface_vtable,
			NULL, NULL, &error);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	serv_info = g_new0(bt_gatt_service, 1);
	serv_info->serv_path = g_strdup(path);
	serv_info->serv_id = object_id;
	serv_info->service_uuid = g_strdup(svc.uuid);
	serv_info->is_svc_registered = FALSE;
	serv_info->is_svc_primary = svc.primary;

	hci.gatt_services = g_slist_append(hci.gatt_services, serv_info);

	*id = serv_id;

exit:
	g_free(path);

	return ret;
}

int bt_gatt_add_characteristic(int svc_id, artik_bt_gatt_chr chr, int *id)
{
	GError *error = NULL;
	GDBusNodeInfo *node_info = NULL;
	GSList *prop_list = NULL;
	gchar *path = NULL;
	guint object_id, char_id;
	bt_gatt_service *service = NULL;
	bt_gatt_char *characteristic = NULL;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (chr.uuid == NULL)
		return E_BT_ERROR;

	/* TODO: service id */
	svc_id = 0;
	service = g_slist_nth_data(hci.gatt_services, svc_id);
	char_id = g_slist_length(service->char_data);

	path = g_strdup_printf("%s%d%s%d", GATT_SERVICE_PREFIX, svc_id,
			GATT_CHARACTERISTIC_PREFIX, char_id);

	node_info = g_dbus_node_info_new_for_xml(char_introspection_xml, &error);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	object_id = g_dbus_connection_register_object(
			hci.conn,
			path,
			node_info->interfaces[0],
			&char_interface_vtable,
			NULL, NULL, &error);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	log_dbg("%s %s added", __func__, path);

	_set_char_properties(chr.property, &prop_list);

	characteristic = g_new0(bt_gatt_char, 1);

	characteristic->char_path = g_strdup(path);
	characteristic->char_id = object_id;
	characteristic->char_uuid = g_strdup(chr.uuid);
	characteristic->char_props = prop_list;
	characteristic->service = service;
	if (chr.length > 0) {
		characteristic->char_value = (guchar *)malloc(sizeof(guchar) * chr.length);
		memcpy(characteristic->char_value, chr.value, chr.length);
		characteristic->value_length = chr.length;
	} else {
		characteristic->value_length = 0;
	}

	service->char_data = g_slist_append(service->char_data, characteristic);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	*id = char_id;

exit:
	g_free(path);

	return ret;
}

artik_error bt_gatt_set_chr_on_read_request(int svc_id, int char_id,
		artik_bt_gatt_req_read callback, void *user_data)
{
	bt_gatt_char *chr = _find_chr_list_by_id(svc_id, char_id);

	if (!chr)
		return E_BAD_ARGS;

	chr->read_callback = callback;
	chr->read_user_data = user_data;

	return S_OK;
}

artik_error bt_gatt_set_chr_on_write_request(int svc_id, int char_id,
		artik_bt_gatt_req_write callback, void *user_data)
{
	bt_gatt_char *chr = _find_chr_list_by_id(svc_id, char_id);

	if (!chr)
		return E_BAD_ARGS;

	chr->write_callback = callback;
	chr->write_user_data = user_data;

	return S_OK;
}

artik_error bt_gatt_set_chr_on_notify_request(int svc_id, int char_id,
		artik_bt_gatt_req_notify callback, void *user_data)
{
	bt_gatt_char *chr = _find_chr_list_by_id(svc_id, char_id);

	if (!chr)
		return E_BAD_ARGS;

	chr->notify_callback = callback;
	chr->notify_user_data = user_data;

	return S_OK;
}

int bt_gatt_add_descriptor(int service_id, int char_id, artik_bt_gatt_desc desc,
		int *id)
{
	GError *error = NULL;
	GDBusNodeInfo *node_info;
	GSList *prop_list = NULL;
	gchar *path = NULL;
	guint object_id, desc_id;
	bt_gatt_service *service = NULL;
	bt_gatt_char *characteristic = NULL;
	bt_gatt_desc *descriptor = NULL;
	artik_error ret = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	log_dbg("bt_gatt_add_descriptor");

	service = g_slist_nth_data(hci.gatt_services, service_id);
	characteristic = g_slist_nth_data(service->char_data, char_id);
	desc_id = g_slist_length(characteristic->desc_data);

	path = g_strdup_printf("%s%d%s%d%s%d",
			GATT_SERVICE_PREFIX, service_id,
			GATT_CHARACTERISTIC_PREFIX, char_id,
			GATT_DESCRIPTOR_PREFIX, desc_id);

	node_info = g_dbus_node_info_new_for_xml(descriptor_introspection_xml,
			&error);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	object_id = g_dbus_connection_register_object(
			hci.conn,
			path,
			node_info->interfaces[0],
			&desc_interface_vtable,
			NULL, NULL, &error);

	ret = bt_check_error(error);
	if (ret != S_OK)
		goto exit;

	_set_desc_properties(desc.property, &prop_list);

	descriptor = g_new0(bt_gatt_desc, 1);

	descriptor->desc_path = g_strdup(path);
	descriptor->desc_id = object_id;
	descriptor->desc_uuid = g_strdup(desc.uuid);
	descriptor->desc_props = prop_list;
	descriptor->chr = characteristic;
	if (desc.length > 0) {
		descriptor->desc_value = (guchar *)malloc(sizeof(guchar) * desc.length);
		memcpy(descriptor->desc_value, desc.value, desc.length);
		descriptor->value_length = desc.length;
	} else {
		descriptor->value_length = 0;
	}

	characteristic->desc_data = g_slist_append(characteristic->desc_data,
			descriptor);

	*id = desc_id;

exit:
	g_free(path);
	return ret;
}

artik_error bt_gatt_set_desc_on_read_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_read callback, void *user_data)
{
	bt_gatt_desc *desc = _find_desc_list_by_id(svc_id, char_id, desc_id);

	if (!desc)
		return E_BAD_ARGS;

	desc->read_callback = callback;
	desc->read_user_data = user_data;

	return S_OK;
}

artik_error bt_gatt_set_desc_on_write_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_write callback, void *user_data)
{
	bt_gatt_desc *desc = _find_desc_list_by_id(svc_id, char_id, desc_id);

	if (!desc)
		return E_BAD_ARGS;

	desc->write_callback = callback;
	desc->write_user_data = user_data;

	return S_OK;
}

int bt_gatt_register_service(int id)
{
	GError *e = NULL;
	gchar *path = NULL;

	path = g_strdup_printf("%s%d", GATT_SERVICE_PREFIX, id);
	g_dbus_connection_call(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_GATTMANAGER1,
			"RegisterApplication",
			g_variant_new("(oa{sv})", path, NULL),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL,
			(GAsyncReadyCallback)register_service_cb, &e);

	g_free(path);

	return bt_check_error(e);
}

void _free_service(int id)
{
	GSList *l1, *l2;
	bt_gatt_service *serv_info = NULL;
	bt_gatt_char *char_info = NULL;
	bt_gatt_desc *desc_info = NULL;

	log_dbg("%s id:%d", __func__, id);
	serv_info = g_slist_nth_data(hci.gatt_services, id);
	if (serv_info == NULL)
		return;

	l1 = serv_info->char_data;
	while (l1 != NULL) {
		char_info = l1->data;

		l2 = char_info->desc_data;
		while (l2 != NULL) {
			desc_info = l2->data;
			l2 = g_slist_remove(l2, desc_info);

			g_free(desc_info->desc_path);
			g_free(desc_info->desc_uuid);
			if (desc_info->value_length > 0)
				g_free(desc_info->desc_value);
			g_free(desc_info);
		}

		l1 = g_slist_remove(l1, char_info);

		g_free(char_info->char_path);
		g_free(char_info->char_uuid);
		if (char_info->value_length > 0)
			g_free(char_info->char_value);
		g_free(char_info);
	}

	hci.gatt_services = g_slist_remove(hci.gatt_services, serv_info);
	g_free(serv_info->serv_path);
	g_free(serv_info->service_uuid);
	g_free(serv_info);
}

artik_error bt_gatt_req_set_value(artik_bt_gatt_req request, int len, const unsigned char *value)
{
	bt_gatt_req_handle *handle = request;
	bt_gatt_char *chr = handle->chr;
	bt_gatt_desc *desc = handle->desc;
	unsigned char **val = NULL;
	unsigned int *ptr_len = NULL;

	if (handle->type != BT_GATT_REQ_TYPE_READ)
		return E_BAD_ARGS;

	if (chr) {
		val = &chr->char_value;
		ptr_len = &chr->value_length;
	} else if (desc) {
		val = &desc->desc_value;
		ptr_len = &desc->value_length;
	}

	if (*val)
		free(*val);

	*val = malloc(sizeof(unsigned char)*len);
	*ptr_len = len;
	memcpy(*val, value, len);

	_send_request(handle, len, value);
	free(handle);
	return S_OK;
}

static artik_error _return_dbus_state(
	bt_gatt_req_handle *handle,
	artik_bt_gatt_req_state_type state, const char *err_msg)
{
	const char *error_type = NULL;

	switch (state) {
	case BT_GATT_REQ_STATE_TYPE_OK:
		if (handle->type != BT_GATT_REQ_TYPE_WRITE)
			return E_BAD_ARGS;

		g_dbus_method_invocation_return_value(handle->invocation, NULL);
		return S_OK;
	case BT_GATT_REQ_STATE_TYPE_FAILED:
		error_type = "org.bluez.Error.Failed";
		break;
	case BT_GATT_REQ_STATE_TYPE_IN_PROGRESS:
		error_type = "org.bluez.Error.InProgress";
		break;
	case BT_GATT_REQ_STATE_TYPE_NOT_PERMITTED:
		if (handle->type == BT_GATT_REQ_TYPE_NOTIFY)
			return E_BAD_ARGS;
		error_type = "org.bluez.Error.NotPermitted";
		break;
	case BT_GATT_REQ_STATE_TYPE_INVALID_VALUE_LENGTH:
		if (handle->type != BT_GATT_REQ_TYPE_WRITE)
			return E_BAD_ARGS;

		error_type = "org.bluez.Error.InvalidValueLength";
		break;
	case BT_GATT_REQ_STATE_TYPE_NOT_AUTHORIZED:
		if (handle->type == BT_GATT_REQ_TYPE_NOTIFY)
			return E_BAD_ARGS;
		error_type = "org.bluez.Error.NotAuthorized";
		break;
	case BT_GATT_REQ_STATE_TYPE_NOT_SUPPORTED:
		error_type = "org.bluez.Error.NotSupported";
		break;
	}

	g_dbus_method_invocation_return_dbus_error(handle->invocation, error_type, err_msg);
	return S_OK;
}

artik_error bt_gatt_req_set_result(artik_bt_gatt_req request, artik_bt_gatt_req_state_type state, const char *err_msg)
{
	bt_gatt_req_handle *handle = request;

	if (handle->type == BT_GATT_REQ_TYPE_WRITE && state == BT_GATT_REQ_STATE_TYPE_OK) {
		if (handle->chr) {
			if (handle->chr->char_value)
				free(handle->chr->char_value);
			handle->chr->value_length = handle->len;
			handle->chr->char_value = handle->value;
		} else if (handle->desc) {
			if (handle->desc->desc_value)
				free(handle->desc->desc_value);
			handle->desc->value_length = handle->len;
			handle->desc->desc_value = handle->value;
		}
	}

	artik_error err = _return_dbus_state(handle, state, err_msg);

	if (err != S_OK)
		return err;

	if (state != BT_GATT_REQ_STATE_TYPE_OK && handle->value)
		free(handle->value);

	free(handle);
	return S_OK;
}

int bt_gatt_unregister_service(int id)
{
	GError *e = NULL;
	gchar *path = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	log_dbg("%s id:%d", __func__, id);

	path = g_strdup_printf("%s%d", GATT_SERVICE_PREFIX, id);
	g_dbus_connection_call(
			hci.conn,
			DBUS_BLUEZ_BUS,
			DBUS_BLUEZ_OBJECT_PATH_HCI0,
			DBUS_IF_GATTMANAGER1,
			"UnregisterApplication",
			g_variant_new("(o)", path, NULL),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, NULL, &e);

	g_free(path);

	if (e != NULL) {
		log_dbg("%s", e->message);
		g_clear_error(&e);
		return E_BT_ERROR;
	}

	_free_service(id);
	return S_OK;
}

int bt_gatt_notify(int service_id, int char_id, unsigned char *byte, int len)
{
	GVariantBuilder *b1, *b11;
	int i = 0;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (len <= 0)
		return E_BT_ERROR;

	bt_gatt_char *chr = _find_chr_list_by_id(service_id, char_id);

	if (!chr)
		return E_BT_ERROR;

	b1 = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	b11 = g_variant_builder_new(G_VARIANT_TYPE("ay"));

	for (i = 0; i < len; i++)
		g_variant_builder_add(b11, "y", byte[i]);

	g_variant_builder_add(b1, "{sv}", "Value", g_variant_new("ay", b11));

	if (chr->char_value)
		free(chr->char_value);

	chr->value_length = len;
	chr->char_value = malloc(sizeof(unsigned char)*len);
	memcpy(chr->char_value, byte, len);
	g_dbus_connection_emit_signal(hci.conn, DBUS_BLUEZ_BUS, chr->char_path,
			DBUS_IF_PROPERTIES, "PropertiesChanged", g_variant_new("(sa{sv}as)",
			DBUS_IF_GATTCHARACTERISTIC1, b1, NULL), NULL);

	g_variant_builder_unref(b1);
	g_variant_builder_unref(b11);

	return S_OK;
}
