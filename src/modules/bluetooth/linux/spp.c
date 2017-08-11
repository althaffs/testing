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
#include <stdbool.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <gio/gio.h>
#pragma GCC diagnostic pop
#include <glib.h>
#include <gio-unix-2.0/gio/gunixfdlist.h>

#include "core.h"
#include "spp.h"

/* Introspection data for the service we are exporting */
static const gchar _introspection_xml[] = "<node>"
		"<interface name='org.bluez.Profile1'>"
		"<method name='Release'>"
		"</method>"
		"<method name='NewConnection'>"
		"<arg type='o' name='device' direction='in'/>"
		"<arg type='h' name='fd' direction='in'/>"
		"<arg type='a{sv}' name='fd_properties' direction='in'/>"
		"</method>"
		"<method name='RequestDisconnection'>"
		"<arg type='o' name='device' direction='in'/>"
		"</method>"
		"</interface>"
		"</node>";

static GDBusNodeInfo *_introspection_data;
static guint registration_id;

artik_bt_spp_server *_spp_server;

static void _handle_release(void)
{
	/* do the release work, such as: (1) quit the mainloop ... */
	if (_spp_server->release_func)
		_spp_server->release_func(_spp_server->user_data);
}

static void _handle_new_connection(GVariant *parameters,
		GDBusMethodInvocation *invocation) {
	GVariant *g_property = NULL;
	GVariant *g_property_dic = NULL;
	GVariant *value = NULL;
	gchar *device_path = NULL;
	gint property_len = 0;
	gchar *key = NULL;
	gint i = 0;
	gint32 fd_handler;
	gint fd;
	GDBusMessage *message = NULL;
	GUnixFDList *fd_list = NULL;
	GError *error = NULL;
	gint version = 0, features = 0;

	g_variant_get(parameters, "(&oh@a{sv})", &device_path, &fd_handler,
			&g_property);
	property_len = g_variant_n_children(g_property);
	if (property_len > 0) {
		for (i = 0; i < property_len; i++) {
			g_property_dic = g_variant_get_child_value(g_property, i);
			g_variant_get(g_property_dic, "{&sv}", &key, &value);
			if (g_strcmp0(key, "Version") == 0)
				g_variant_get(value, "q", &version);
			if (g_strcmp0(key, "Features") == 0)
				g_variant_get(value, "q", &features);
		}
	}
	message = g_dbus_method_invocation_get_message(invocation);
	fd_list = g_dbus_message_get_unix_fd_list(message);
	fd = g_unix_fd_list_get(fd_list, fd_handler, &error);
	if (g_property)
		g_variant_unref(g_property);
	if (g_property_dic)
		g_variant_unref(g_property_dic);
	if (value)
		g_variant_unref(value);

	if (_spp_server->connect_func) {
		gchar *address;

		_get_device_address(device_path, &address);
		_spp_server->connect_func(address, fd, version, features,
			_spp_server->user_data);
		g_free(address);
	}

	g_free(device_path);
}

static void _handle_request_disconnection(GVariant *parameters,
		GDBusMethodInvocation *invocation) {
	gchar *device_path = NULL;

	g_variant_get(parameters, "(o)", &device_path);
	/* return the path, and release the socket */
	if (_spp_server->disconnect_func) {
		gchar *address = NULL;

		_get_device_address(device_path, &address);
		_spp_server->disconnect_func(device_path,
			_spp_server->user_data);
		g_free(address);
	}

	g_free(device_path);
}

static void _handle_method_call(GDBusConnection *connection,
		const gchar *sender, const gchar *object_path,
		const gchar *interface_name, const gchar *method_name,
		GVariant *parameters, GDBusMethodInvocation *invocation,
		gpointer user_data) {
	if (g_strcmp0(method_name, "Release") == 0)
		_handle_release();
	if (g_strcmp0(method_name, "NewConnection") == 0)
		_handle_new_connection(parameters, invocation);
	if (g_strcmp0(method_name, "RequestDisconnection") == 0)
		_handle_request_disconnection(parameters, invocation);
}

static const GDBusInterfaceVTable _interface_vtable = {
		.method_call = _handle_method_call,
		.get_property = NULL,
		.set_property = NULL
};

static int _register_spp_object(void)
{
	GError *error = NULL;
	/* register objects */
	_introspection_data = g_dbus_node_info_new_for_xml(_introspection_xml,
			NULL);

	GDBusInterfaceInfo *interface = g_dbus_node_info_lookup_interface(
			_introspection_data, DBUS_IF_PROFILE);

	registration_id = g_dbus_connection_register_object(hci.conn,
			DBUS_BLUEZ_SPP_PROFILE, interface,
			&_interface_vtable, NULL, NULL,
			&error);
	if (error) {
		log_dbg("g_dbus_connection_register_object failed :%s\n",
			error->message);
		g_clear_error(&error);
		return E_BT_ERROR;
	}
	log_dbg("registration id : %d\n", registration_id);

	g_hash_table_insert(hci.registration_ids, g_strdup("ObjectRegistered"),
			GUINT_TO_POINTER(registration_id));

	if (!_spp_server) {
		_spp_server = (artik_bt_spp_server *) malloc(
				sizeof(artik_bt_spp_server));
		if (_spp_server)
			memset(_spp_server, 0, sizeof(artik_bt_spp_server));
		else
			return E_NO_MEM;
	}
	return S_OK;
}

artik_error bt_spp_register_profile(artik_bt_spp_profile_option *opt)
{
	GError *error = NULL;
	GVariantBuilder *option = NULL;
	int ret = -1;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (opt == NULL)
		return E_INVALID_VALUE;
	if (opt->channel <= 0)
		return E_INVALID_VALUE;
	if ((strncmp(opt->role, "server", strlen("server")))
		&& (strncmp(opt->role, "client", strlen("client"))))
		return E_INVALID_VALUE;

	option = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	if (opt->name != NULL)
		g_variant_builder_add(option, "{sv}", "Name",
				g_variant_new_string(opt->name));
	if (opt->service != NULL)
		g_variant_builder_add(option, "{sv}", "Service",
				g_variant_new_string(opt->service));
	if (opt->role != NULL)
		g_variant_builder_add(option, "{sv}", "Role",
				g_variant_new_string(opt->role));
	g_variant_builder_add(option, "{sv}", "Channel",
			g_variant_new_uint16(opt->channel));
	g_variant_builder_add(option, "{sv}", "PSM",
			g_variant_new_uint16(opt->PSM));
	g_variant_builder_add(option, "{sv}", "RequireAuthentication",
			g_variant_new_boolean(opt->require_authentication));
	g_variant_builder_add(option, "{sv}", "RequireAuthorization",
			g_variant_new_boolean(opt->require_authorization));
	g_variant_builder_add(option, "{sv}", "AutoConnect",
			g_variant_new_boolean(opt->auto_connect));
	g_variant_builder_add(option, "{sv}", "Version",
			g_variant_new_uint16(opt->version));
	g_variant_builder_add(option, "{sv}", "Features",
			g_variant_new_uint16(opt->features));
	g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		DBUS_BLUEZ_OBJECT_PATH,
		DBUS_IF_PROFILE_MANAGER1,
		"RegisterProfile",
		g_variant_new("(osa{sv})",
		DBUS_BLUEZ_SPP_PROFILE,
		BT_SPP_SHORT_UUID, option),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);
	g_variant_builder_unref(option);

	if (error) {
		log_dbg("Register profile option failed :%s\n", error->message);
		g_clear_error(&error);
		return E_BT_ERROR;
	}

	ret = _register_spp_object();

	return ret;
}

artik_error bt_spp_unregister_profile(void)
{
	GError *error = NULL;
	gboolean status;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		DBUS_BLUEZ_OBJECT_PATH,
		DBUS_IF_PROFILE_MANAGER1,
		"UnregisterProfile",
		g_variant_new("(o)", DBUS_BLUEZ_SPP_PROFILE),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("Unregister Profile failed :%s\n", error->message);
		g_clear_error(&error);
		return E_BT_ERROR;
	}

	status = g_dbus_connection_unregister_object(
		hci.conn, registration_id);
	if (status != TRUE)
		return E_BT_ERROR;

	if (_spp_server) {
		free(_spp_server);
		_spp_server = NULL;
	}

	return S_OK;
}

artik_error bt_spp_set_callback(release_callback release_func,
		new_connection_callback connect_func,
		request_disconnect_callback disconnect_func,
		void *user_data) {
	if (_spp_server != NULL) {
		_spp_server->release_func = release_func;
		_spp_server->connect_func = connect_func;
		_spp_server->disconnect_func = disconnect_func;
		_spp_server->user_data = user_data;
		return S_OK;
	}
	return E_INVALID_VALUE;
}
