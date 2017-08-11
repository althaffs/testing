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
#include <inttypes.h>

#include "core.h"
#include "ftp.h"

static FILE *obexd_start_stream;
static bool obexed_is_used = FALSE;
static bt_ftp_transport_state ftp_state = BT_FTP_STATE_END;
static bt_ftp_req_type ftp_req = BT_FTP_REQ_END;

static artik_error _obexd_start_check
	(const char *stream, const char *target_stream)
{
	if (strstr(stream, target_stream))
		return S_OK;
	else
		return E_BT_ERROR;
}

static artik_error _call_exe(const char *cmd)
{
	FILE *obexd_check_stream = NULL;
	artik_error err = S_OK;
	char obexd_buf[OBEXD_BUF_SIZE];
	int obexd_buf_len = 0;

	obexd_start_stream = NULL;
	memset(obexd_buf, '\0', sizeof(obexd_buf));
	obexd_check_stream = popen(OBEXD_CHECK_CMD, "r");

	if (obexd_check_stream == NULL) {
		log_dbg("get obexd status failed!\n");
		return E_BT_ERROR;
	}

	obexd_buf_len = fread(obexd_buf, sizeof(char),
		sizeof(obexd_buf), obexd_check_stream);
	if (obexd_buf_len <= 0)
		log_dbg("get obexd check stream failed!\n");

	if (_obexd_start_check(obexd_buf, OBEXD_CMD) == S_OK) {
		obexed_is_used = TRUE;
		log_dbg("obexd is already started!\n");
	} else
		obexed_is_used = FALSE;

	if (!obexed_is_used) {
		obexd_start_stream = popen(cmd, "r");
		sleep(OBEXD_WATI_TIME);
		if (obexd_start_stream == NULL)
			err = E_BT_ERROR;
		else
			err = S_OK;
	} else
		err = S_OK;

	return err;
}

static artik_error _destroy_exe(const char *cmd)
{
	FILE *obexd_destroy_stream = NULL;

	if (obexed_is_used)
		return S_OK;

	obexd_destroy_stream = popen(cmd, "r");
	if (pclose(obexd_destroy_stream) == -1) {
		log_dbg("close destroy_exe process faield!\n");
		return E_BT_ERROR;
	}
	if (pclose(obexd_start_stream) == -1) {
		log_dbg("close call_exe process faield!\n");
		return E_BT_ERROR;
	}

	return S_OK;
}

static artik_error _check_status(void)
{
	if (strlen(session_path) == 0) {
		log_err("FTP session is not initialized");
		return E_NOT_INITIALIZED;
	}

	if (ftp_state == BT_FTP_STATE_CANCELING) {
		log_err("Last Cancel operation is not finished, please try later");
		return E_BUSY;
	}
	return S_OK;
}

static void _ftp_session_callback(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
{
	GDBusConnection *bus = G_DBUS_CONNECTION(source_object);
	gboolean connected = FALSE;
	GError *error = NULL;

	g_dbus_connection_call_finish(bus, res, &error);
	if (error == NULL) {
		connected = TRUE;
		hci.state = BT_DEVICE_STATE_CONNECTED;
	} else {
		hci.state = BT_DEVICE_STATE_IDLE;
		log_err("Connect remote device failed :%s\n", error->message);
		g_clear_error(&error);
	}
	_user_callback(BT_EVENT_CONNECT, &connected);
}

artik_error bt_ftp_create_session(char *dest_addr)
{
	GVariantBuilder *args = NULL;

	if (S_OK != (_call_exe(OBEXD_START_CMD))) {
		log_dbg("Start obexd faild!\n");
		return E_BT_ERROR;
	}

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	args = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(args, "{sv}", "Target", g_variant_new_string("ftp"));

	hci.state = BT_DEVICE_STATE_CONNECTING;
	g_dbus_connection_call(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS,
		DBUS_BLUEZ_OBEX_PATH,
		DBUS_IF_OBEX_CLIENT,
		"CreateSession",
		g_variant_new("(sa{sv})", dest_addr, args),	NULL,
		G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL,
		_ftp_session_callback, NULL);

	g_variant_builder_unref(args);

	return S_OK;
}

artik_error bt_ftp_remove_session(void)
{
	GError *error = NULL;

	if (strlen(session_path) == 0)
		return E_NOT_INITIALIZED;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	g_dbus_connection_call_sync(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS,
		DBUS_BLUEZ_OBEX_PATH,
		DBUS_IF_OBEX_CLIENT, "RemoveSession",
		g_variant_new("(o)", session_path),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT,
		NULL, &error);

	if (error) {
		log_dbg("Remove FTP session failed :%s\n", error->message);
		g_clear_error(&error);

		return E_BT_ERROR;
	}
	if (S_OK != (_destroy_exe(OBEXD_DESTROY_CMD))) {
		log_dbg("Destroy obexd failed!\n");

		return E_BT_ERROR;
	}

	return S_OK;
}

artik_error bt_ftp_change_folder(char *folder)
{
	GError *error = NULL;
	artik_error ret = S_OK;

	if (!folder)
		return E_BAD_ARGS;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path != NULL)
		return E_BUSY;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	g_dbus_connection_call_sync(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS, session_path,
		DBUS_IF_OBEX_FILE_TRANSFER, "ChangeFolder",
		g_variant_new("(s)", folder), NULL,
		G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("Change folder failed :%s\n", error->message);
		g_clear_error(&error);

		return E_BT_ERROR;
	}

	return S_OK;
}

artik_error bt_ftp_create_folder(char *folder)
{
	GError *error = NULL;
	artik_error ret = S_OK;

	if (!folder)
		return E_BAD_ARGS;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path != NULL)
		return E_BUSY;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	g_dbus_connection_call_sync(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS, session_path,
		DBUS_IF_OBEX_FILE_TRANSFER, "CreateFolder",
		g_variant_new("(s)", folder), NULL,
		G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("Create folder failed :%s\n", error->message);
		g_clear_error(&error);

		return E_BT_ERROR;
	}

	return S_OK;
}

artik_error bt_ftp_delete_file(char *file)
{
	GError *error = NULL;
	artik_error ret = S_OK;

	if (!file)
		return E_BAD_ARGS;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path != NULL)
		return E_BUSY;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	g_dbus_connection_call_sync(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS, session_path,
		DBUS_IF_OBEX_FILE_TRANSFER, "Delete",
		g_variant_new("(s)", file), NULL,
		G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("Delete file failed :%s\n", error->message);
		g_clear_error(&error);

		return E_BT_ERROR;
	}

	return S_OK;
}

static artik_bt_ftp_file *_parse_list(GVariant *result)
{
	gchar *key = NULL;
	GVariant *ar, *value;
	GVariantIter *iter, *iter2;
	gchar *type, *name, *permission, *modified, *accessed, *created;
	unsigned long long size;

	artik_bt_ftp_file *current_item = NULL;
	artik_bt_ftp_file *head_item = NULL;

	g_variant_get(result, "(aa{sv})", &iter);
	while (g_variant_iter_loop(iter, "@a{sv}", &ar)) {
		g_variant_get(ar, "a{sv}", &iter2);
		artik_bt_ftp_file *file_item = (artik_bt_ftp_file *) malloc(
				sizeof(artik_bt_ftp_file));
		if (file_item) {
			memset(file_item, 0, sizeof(artik_bt_ftp_file));
			while (g_variant_iter_loop(iter2, "{&sv}", &key, &value)) {
				if (g_strcmp0(key, "Type") == 0) {
					g_variant_get(value, "s", &type);
					file_item->file_type = (char *) malloc(strlen(type) + 1);
					if (file_item->file_type) {
						strncpy(file_item->file_type, type, strlen(type));
						file_item->file_type[strlen(type)] = '\0';
					}
				} else if (g_strcmp0(key, "Name") == 0) {
					g_variant_get(value, "s", &name);
					file_item->file_name = (char *) malloc(strlen(name) + 1);
					if (file_item->file_name) {
						strncpy(file_item->file_name, name, strlen(name));
						file_item->file_name[strlen(name)] = '\0';
					}
				} else if (g_strcmp0(key, "Size") == 0) {
					g_variant_get(value, "t", &size);
					file_item->size = size;
				} else if (g_strcmp0(key, "User-perm") == 0) {
					g_variant_get(value, "s", &permission);
					file_item->file_permission = (char *) malloc(
							strlen(permission) + 1);
					if (file_item->file_permission) {
						file_item->file_permission[strlen(permission)] = '\0';
						strncpy(file_item->file_permission, permission,
								strlen(permission));
					}
				} else if (g_strcmp0(key, "Modified") == 0) {
					g_variant_get(value, "s", &modified);
					file_item->modified = (char *) malloc(strlen(modified) + 1);
					if (file_item->modified) {
						file_item->modified[strlen(modified)] = '\0';
						strncpy(file_item->modified, modified,
								strlen(file_item->modified));
					}
				} else if (g_strcmp0(key, "Accessed") == 0) {
					g_variant_get(value, "s", &accessed);
				} else if (g_strcmp0(key, "Created") == 0) {
					g_variant_get(value, "s", &created);
				}
				/* do not call g_variant_unref(value) here */
			}
			file_item->next_file = NULL;
			if (current_item != NULL)
				current_item->next_file = file_item;
			if (head_item == NULL)
				head_item = file_item;

			current_item = file_item;
		} else {
			log_err("No memory\n");
			g_variant_iter_free(iter2);
			return NULL;
		}
		g_variant_iter_free(iter2);
	}
	g_variant_iter_free(iter);
	return head_item;
}

artik_error bt_ftp_list_folder(artik_bt_ftp_file **file_list)
{
	GVariant *result = NULL;
	GError *error = NULL;
	artik_error ret = S_OK;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path != NULL)
		return E_BUSY;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	result = g_dbus_connection_call_sync(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS, session_path,
		DBUS_IF_OBEX_FILE_TRANSFER, "ListFolder",
		NULL, NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT,
		NULL, &error);

	if (error) {
		log_dbg("List folder failed :%s\n", error->message);
		g_clear_error(&error);

		return E_BT_ERROR;
	}

	*file_list = _parse_list(result);
	g_variant_unref(result);

	return S_OK;
}

static void _parse_transfer_property(GVariant *result)
{
	GVariant *tmp = NULL;

	tmp = g_variant_get_child_value(result, 0);
	transfer_property.object_path = g_variant_dup_string(tmp, NULL);
	g_variant_unref(tmp);

	tmp = g_variant_get_child_value(result, 1);
	_fill_transfer_property(tmp);
	g_variant_unref(tmp);
}

artik_error bt_ftp_get_file(char *target_file, char *source_file)
{
	GVariant *result = NULL;
	GError *error = NULL;
	artik_error ret = S_OK;

	if ((!source_file) || (!target_file))
		return E_BAD_ARGS;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path != NULL)
		return E_BUSY;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	result = g_dbus_connection_call_sync(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS, session_path,
		DBUS_IF_OBEX_FILE_TRANSFER, "GetFile",
		g_variant_new("(ss)", target_file, source_file), NULL,
		G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("Get file failed :%s\n", error->message);
		g_clear_error(&error);

		return E_BT_ERROR;
	}

	ftp_req = BT_FTP_REQ_GET;

	_parse_transfer_property(result);
	g_variant_unref(result);

	return S_OK;
}

artik_error bt_ftp_put_file(char *source_file, char *target_file)
{
	GError *error = NULL;
	GVariant *result;
	artik_error ret = S_OK;

	if ((!source_file) || (!target_file))
		return E_BAD_ARGS;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path != NULL)
		return E_BUSY;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	result = g_dbus_connection_call_sync(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS, session_path,
		DBUS_IF_OBEX_FILE_TRANSFER, "PutFile",
		g_variant_new("(ss)", source_file, target_file), NULL,
		G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("Put file failed :%s\n", error->message);
		g_clear_error(&error);

		return E_BT_ERROR;
	}

	ftp_req = BT_FTP_REQ_PUT;

	_parse_transfer_property(result);
	g_variant_unref(result);
	return S_OK;
}

static void _ftp_cancel_callback(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
{
	GDBusConnection *bus = G_DBUS_CONNECTION(source_object);
	GError *error = NULL;

	g_dbus_connection_call_finish(bus, res, &error);
	if (error) {
		log_err("FTP cancel transport failed :%s\n", error->message);
		g_clear_error(&error);
		ftp_state = BT_FTP_STATE_ERROR;
	} else {
		ftp_state = BT_FTP_STATE_CANCELED;
		ftp_req = BT_FTP_REQ_END;
	}
}

static void _ftp_resume_callback(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
{
	GDBusConnection *bus = G_DBUS_CONNECTION(source_object);
	GError *error = NULL;

	g_dbus_connection_call_finish(bus, res, &error);
	if (error) {
		log_err("FTP resume transport failed :%s\n", error->message);
		g_clear_error(&error);
		ftp_state = BT_FTP_STATE_ERROR;
	} else {
		if (ftp_req == BT_FTP_REQ_PUT
			&& ftp_state == BT_FTP_STATE_CANCELING) {
			g_dbus_connection_call(hci.session_conn,
				DBUS_BLUEZ_OBEX_BUS,
				transfer_property.object_path,
				DBUS_IF_OBEX_TRANSFER,
				"Cancel", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
				G_MAXINT, NULL,
				_ftp_cancel_callback, NULL);
			return;
		}
		ftp_state = BT_FTP_STATE_RESUMED;
	}
}

static void _ftp_suspend_callback(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
{
	GDBusConnection *bus = G_DBUS_CONNECTION(source_object);
	GError *error = NULL;

	g_dbus_connection_call_finish(bus, res, &error);
	if (error) {
		log_err("FTP suspend transport failed :%s\n", error->message);
		g_clear_error(&error);
		ftp_state = BT_FTP_STATE_ERROR;
	} else
		ftp_state = BT_FTP_STATE_SUSPENDED;
}

artik_error bt_ftp_resume_transfer(void)
{
	artik_error ret = S_OK;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	if (transfer_property.object_path == NULL)
		return E_BUSY;

	g_dbus_connection_call(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS,
		transfer_property.object_path,
		DBUS_IF_OBEX_TRANSFER,
		"Resume", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
		G_MAXINT, NULL,
		_ftp_resume_callback, NULL);

	ftp_state = BT_FTP_STATE_RESUMING;

	return S_OK;
}

artik_error bt_ftp_suspend_transfer(void)
{
	artik_error ret = S_OK;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path == NULL)
		return E_BUSY;

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	g_dbus_connection_call(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS,
		transfer_property.object_path,
		DBUS_IF_OBEX_TRANSFER,
		"Suspend", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
		G_MAXINT, NULL,
		_ftp_suspend_callback, NULL);

	ftp_state = BT_FTP_STATE_SUSPENDING;

	return S_OK;
}

artik_error bt_ftp_cancel_transfer(void)
{
	artik_error ret = S_OK;

	ret = _check_status();
	if (ret != S_OK)
		return ret;

	if (transfer_property.object_path == NULL) {
		log_err("transfer path is NULL");
		return E_BUSY;
	}

	bt_init(G_BUS_TYPE_SESSION, &(hci.session_conn));

	if (ftp_req == BT_FTP_REQ_PUT
		&& ftp_state == BT_FTP_STATE_SUSPENDED) {
		g_dbus_connection_call(hci.session_conn,
			DBUS_BLUEZ_OBEX_BUS,
			transfer_property.object_path,
			DBUS_IF_OBEX_TRANSFER,
			"Resume", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT, NULL,
			_ftp_resume_callback, NULL);
		ftp_state = BT_FTP_STATE_CANCELING;
		return S_OK;
	}

	g_dbus_connection_call(hci.session_conn,
		DBUS_BLUEZ_OBEX_BUS,
		transfer_property.object_path,
		DBUS_IF_OBEX_TRANSFER,
		"Cancel", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
		G_MAXINT, NULL,
		_ftp_cancel_callback, NULL);

	ftp_state = BT_FTP_STATE_CANCELING;

	return S_OK;
}

