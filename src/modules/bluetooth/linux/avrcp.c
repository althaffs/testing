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

#include "core.h"
#include "avrcp.h"

GSList *items;

static void _fill_property_metadata(GVariant *metadata, artik_bt_avrcp_item_property *property);
static artik_error _get_malloc_content(char **dest, GVariant *v, char *type);

static artik_error _get_property(char *_path, char *_interface, char *_property,
	GVariant **variant)
{
	GVariant *result = NULL;
	GError *error = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	result = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			_path,
			DBUS_IF_PROPERTIES,
			"Get",
			g_variant_new("(ss)", _interface, _property),
			G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT, NULL, &error);

	if (error) {
		log_err("Get property failed: %s\n", error->message);
		g_clear_error(&error);
		return E_BT_ERROR;
	}

	g_variant_get(result, "(v)", variant);
	g_variant_unref(result);
	return S_OK;
}

static artik_error _get_control_path(char **path)
{
	GVariant *obj1 = NULL, *ar1 = NULL, *ar2 = NULL;
	GVariantIter *iter1 = NULL, *iter2 = NULL;
	char *dev_path = NULL, *itf = NULL;
	bool is_find = false, is_connected = false;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));
	artik_error err = _get_managed_objects(&obj1);

	if (err != S_OK)
		return err;

	g_variant_get(obj1, "(a{oa{sa{sv}}})", &iter1);
	while (g_variant_iter_loop(iter1, "{&o@a{sa{sv}}}", &dev_path, &ar1)) {
		g_variant_get(ar1, "a{sa{sv}}", &iter2);
		while (g_variant_iter_loop(iter2, "{&s@a{sv}}", &itf, &ar2)) {
			if (strncmp(itf, DBUS_IF_MEDIA_CONTROL1,
					strlen(DBUS_IF_MEDIA_CONTROL1)) != 0)
				continue;
			else {
				GVariant *v = NULL;
				artik_error e = S_OK;

				e = _get_property(dev_path,
					DBUS_IF_MEDIA_CONTROL1, "Connected", &v);
				if (e == S_OK) {
					g_variant_get(v, "b", &is_connected);
					g_variant_unref(v);
					if (is_connected) {
						*path = strdup(dev_path);
						is_find = true;
						g_variant_unref(ar2);
						break;
					}
				}
			}
		}
		g_variant_iter_free(iter2);
		if (is_find) {
			g_variant_unref(ar1);
			break;
		}
	}

	g_variant_iter_free(iter1);
	if (is_find) {
		log_dbg("control_interface_path[%s]\n", *path);
		return S_OK;
	}
	log_dbg("no control interface find.\n");
	return E_BT_ERROR;
}

static artik_error _get_player_path(char **path)
{
	char *control_path = NULL, *player_path = NULL;
	GVariant *v = NULL;
	artik_error e = S_OK;

	e = _get_control_path(&control_path);
	if (e == S_OK && control_path) {
		e = _get_property(control_path,
			DBUS_IF_MEDIA_CONTROL1, "Player", &v);
		free(control_path);
		if (e == S_OK && v) {
			g_variant_get(v, "o", &player_path);
			g_variant_unref(v);
			*path = (char *) malloc(strlen(player_path) + 1);
			if (*path) {
				memcpy(*path, player_path, strlen(player_path));
				(*path)[strlen(player_path)] = '\0';
				log_dbg("player_interface_path[%s]", *path);
				return S_OK;
			}
		}
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_change_folder(const char *folder)
{
	GVariant *result;
	GError *g_error = NULL;
	artik_error e = S_OK;
	char *player_path = NULL;
	GSList *l;
	bool find_folder = false;

	for (l = items; l; l = l->next) {
		if (strcmp(folder, (char *) (l->data)) == 0) {
			find_folder = true;
			log_dbg("AVRCP Change folder : folder found\n");
			break;
		}
	}

	if (!find_folder) {
		log_dbg("AVRCP Change folder failed : no such folder\n");
		return E_BT_ERROR;
	}

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));
	e = _get_player_path(&player_path);

	if (e == S_OK && player_path) {
		result = g_dbus_connection_call_sync(hci.conn, DBUS_BLUEZ_BUS,
				player_path,
				DBUS_IF_MEDIA_FOLDER1, "ChangeFolder",
				g_variant_new("(o)", folder),
				NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &g_error);
		free(player_path);
		if (g_error) {
			log_dbg("AVRCP Change folder failed :%s\n", g_error->message);
			g_clear_error(&g_error);
			return E_BT_ERROR;
		}
		g_variant_unref(result);
		return S_OK;
	}
	return E_BT_ERROR;
}

artik_bt_avrcp_item *_parse_list(GVariant *result)
{
	GVariant *ar1, *ar2;
	GVariantIter *iter1, *iter2;
	gchar *path, *key;

	g_variant_get(result, "(a{oa{sv}})", &iter1);

	artik_bt_avrcp_item *current_item = NULL;
	artik_bt_avrcp_item *head_item = NULL;

	if (items) {
		g_slist_free(items);
		items = NULL;
	}

	while (g_variant_iter_loop(iter1, "{&o@a{sv}}", &path, &ar1)) {
		g_variant_get(ar1, "a{sv}", &iter2);
		artik_bt_avrcp_item *avrcp_item = (artik_bt_avrcp_item *) malloc(
				sizeof(artik_bt_avrcp_item));
		if (avrcp_item) {
			avrcp_item->item_obj_path = (char *) malloc(strlen(path) + 1);
			strcpy(avrcp_item->item_obj_path, path);
			avrcp_item->property = NULL;
			avrcp_item->next_item = NULL;
			items = g_slist_append(items, avrcp_item->item_obj_path);
		} else {
			return NULL;
		}

		if (current_item == NULL) {
			current_item = avrcp_item;
			head_item = avrcp_item;
		} else {
			current_item->next_item = avrcp_item;
			current_item = avrcp_item;
		}

		artik_bt_avrcp_item_property *avrcp_current_property
			= malloc(sizeof(artik_bt_avrcp_item_property));
		memset(avrcp_current_property, 0, sizeof(artik_bt_avrcp_item_property));

		if (!avrcp_current_property)
			return NULL;
		avrcp_item->property = avrcp_current_property;

		while (g_variant_iter_loop(iter2, "{&sv}", &key, &ar2)) {
			if (strcmp(key, "Player") == 0)
				_get_malloc_content(&(avrcp_current_property->player), ar2, "o");
			else if (strcmp(key, "Name") == 0)
				_get_malloc_content(&(avrcp_current_property->name), ar2, "s");
			else if (strcmp(key, "Type") == 0)
				_get_malloc_content(&(avrcp_current_property->type), ar2, "s");
			else if (strcmp(key, "FolderType") == 0)
				_get_malloc_content(&(avrcp_current_property->folder), ar2, "s");
			else if (strcmp(key, "Playable") == 0)
				g_variant_get(ar2, "b", &(avrcp_current_property->playable));
			else if (strcmp(key, "Metadata") == 0)
				_fill_property_metadata(ar2, avrcp_current_property);
		}
		g_variant_iter_free(iter2);
	}

	g_variant_iter_free(iter1);
	return head_item;
}

artik_error bt_avrcp_controller_list_item(int start_item, int end_item,
		artik_bt_avrcp_item **item_list)
{
	artik_error e = S_OK;
	char *player_path = NULL;
	GVariant *result = NULL;
	GVariantBuilder *builder = NULL;
	GError *g_error = NULL;

	if (start_item >= 0 && end_item >= 0 && end_item >= start_item) {
		builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
		g_variant_builder_add(builder, "{sv}", "Start",
				g_variant_new_uint32(start_item));
		g_variant_builder_add(builder, "{sv}", "End",
				g_variant_new_uint32(end_item));
	} else if (start_item != -1 && end_item != -1) {
		return E_BAD_ARGS;
	}

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));
	e = _get_player_path(&player_path);

	if (e != S_OK || player_path == NULL) {
		if (builder != NULL)
			g_variant_builder_unref(builder);
		return E_BT_ERROR;
	}

	result = g_dbus_connection_call_sync(hci.conn, DBUS_BLUEZ_BUS, player_path,
			DBUS_IF_MEDIA_FOLDER1, "ListItems", g_variant_new("(a{sv})", builder), NULL,
			G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &g_error);

	free(player_path);
	if (g_error) {
		log_dbg("AVRCP list item failed :%s\n", g_error->message);
		g_clear_error(&g_error);
		return E_BT_ERROR;
	}
	if (result == NULL)
		return E_BT_ERROR;

	*item_list = _parse_list(result);
	if (builder != NULL)
		g_variant_builder_unref(builder);
	g_variant_unref(result);
	return S_OK;
}

artik_error bt_avrcp_controller_set_repeat(const char *repeat_mode)
{
	GVariant *result;
	GError *g_error = NULL;
	artik_error e = S_OK;
	char *player_path = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));
	e = _get_player_path(&player_path);

	if (e == S_OK && player_path) {
		result = g_dbus_connection_call_sync(hci.conn, DBUS_BLUEZ_BUS,
				player_path,
				DBUS_IF_PROPERTIES, "Set",
				g_variant_new("(ssv)", DBUS_IF_MEDIA_PLAYER1, "Repeat",
						g_variant_new_string(repeat_mode)), NULL,
				G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &g_error);
		free(player_path);
		if (g_error) {
			log_dbg("AVRCP set repeat failed :%s\n", g_error->message);
			g_clear_error(&g_error);
			return E_BT_ERROR;
		}
		g_variant_unref(result);

		return S_OK;
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_get_repeat(char **repeat_mode)
{
	char *player_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_player_path(&player_path);
	if (e != S_OK || !player_path)
		return E_BT_ERROR;

	e = _get_property(player_path,
			DBUS_IF_MEDIA_PLAYER1, "Repeat", &v);
	free(player_path);

	if (e != S_OK && !v)
		return E_BT_ERROR;

	g_variant_get(v, "s", repeat_mode);
	g_variant_unref(v);
	return S_OK;
}

artik_error bt_avrcp_controller_is_connected(bool *is_connected)
{
	char *control_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_control_path(&control_path);
	if (e == S_OK && control_path) {
		e = _get_property(control_path,
			DBUS_IF_MEDIA_CONTROL1, "Connected", &v);
		free(control_path);
		if (e == S_OK && v) {
			g_variant_get(v, "b", is_connected);
			g_variant_unref(v);
			return S_OK;
		}
	}
	return E_BT_ERROR;
}

static artik_error _invoke_remote_control(const char *command)
{
	GError *error = NULL;
	char *player_path = NULL;
	artik_error e = S_OK;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));
	e = _get_player_path(&player_path);
	if (e == S_OK && player_path) {
		g_dbus_connection_call_sync(hci.conn,
				DBUS_BLUEZ_BUS,
				player_path,
				DBUS_IF_MEDIA_PLAYER1, command, NULL,
				NULL, G_DBUS_CALL_FLAGS_NONE,
				G_MAXINT, NULL, &error);
		free(player_path);
		if (error) {
			log_dbg("Remote control failed: %s\n", error->message);
			g_clear_error(&error);
			return E_BT_ERROR;
		}
		return S_OK;
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_resume_play(void)
{
	return _invoke_remote_control("Play");
}
artik_error bt_avrcp_controller_pause(void)
{
	return _invoke_remote_control("Pause");
}
artik_error bt_avrcp_controller_stop(void)
{
	return _invoke_remote_control("Stop");
}
artik_error bt_avrcp_controller_next(void)
{
	return _invoke_remote_control("Next");
}
artik_error bt_avrcp_controller_previous(void)
{
	return _invoke_remote_control("Previous");
}
artik_error bt_avrcp_controller_fast_forward(void)
{
	return _invoke_remote_control("FastForward");
}
artik_error bt_avrcp_controller_rewind(void)
{
	return _invoke_remote_control("Rewind");
}

static artik_error _get_malloc_content(char **dest, GVariant *v, char *type)
{
	char *content;
	char *content_dest;

	g_variant_get(v, type, &content);

	content_dest = (char *) malloc(strlen(content) + 1);
	strcpy(content_dest, content);
	content_dest[strlen(content)] = '\0';
	*dest = content_dest;

	return S_OK;
}

static artik_error _get_property_malloc_content(char **dest,
			char *object_path, char *property_name, char *type)
{
	GVariant *v;

	if (_get_property(object_path,
			DBUS_IF_MEDIAITEM1, property_name, &v) != S_OK) {
		log_err("get %s property error!\n", property_name);
		return E_BT_ERROR;
	}
	_get_malloc_content(dest, v, type);

	g_variant_unref(v);
	return S_OK;
}

static artik_error _get_property_bool_content(bool *dest,
			char *object_path, char *property_name)
{
	GVariant *v;
	bool content;

	if (_get_property(object_path,
			DBUS_IF_MEDIAITEM1, property_name, &v) != S_OK) {
		log_err("get %s property error!\n", property_name);
		return E_BT_ERROR;
	}
	g_variant_get(v, "b", &content);

	*dest = content;

	g_variant_unref(v);
	return S_OK;
}

static void _fill_property_metadata(GVariant *metadata, artik_bt_avrcp_item_property *property)
{
	GVariant *prop_dict = NULL;
	gchar *key;
	GVariant *value = NULL;

	int property_len = 0;


	property_len = g_variant_n_children(metadata);

	if (property_len <= 0)
		return;

	for (int i = 0; i < property_len; i++) {
		prop_dict = g_variant_get_child_value(metadata, i);
		g_variant_get(prop_dict, "{&sv}", &key, &value);

		if (g_strcmp0(key, "Title") == 0) {
			_get_malloc_content(&(property->title), value, "s");
			log_dbg("Title is: %s\n", property->title);
		} else if (g_strcmp0(key, "Artist") == 0) {
			_get_malloc_content(&(property->artist), value, "s");
			log_dbg("Artist is: %s\n", property->artist);
		} else if (g_strcmp0(key, "Album") == 0) {
			_get_malloc_content(&(property->album), value, "s");
			log_dbg("Album is: %s\n", property->album);
		} else if (g_strcmp0(key, "Genre") == 0) {
			_get_malloc_content(&(property->genre), value, "s");
			log_dbg("Genre is: %s\n", property->genre);
		} else if (g_strcmp0(key, "NumberOfTracks") == 0) {
			g_variant_get(value, "u", &(property->number_of_tracks));
			log_dbg("NumberOfTracks is: %d\n", property->number_of_tracks);
		} else if (g_strcmp0(key, "Number") == 0) {
			g_variant_get(value, "u", &(property->number));
			log_dbg("Number is: %d\n", property->number);
		} else if (g_strcmp0(key, "Duration") == 0) {
			g_variant_get(value, "u", &(property->duration));
			log_dbg("Duration is: %d\n", property->duration);
		}
		g_variant_unref(prop_dict);
	}
}

static artik_error _get_property_metadata_content(
		artik_bt_avrcp_item_property **properties, char *object_path)
{
	GVariant *v;

	if (_get_property(object_path,
			DBUS_IF_MEDIAITEM1,
			"Metadata",
			&v) != S_OK) {
		log_err("get Metadata property error!\n");
		return E_BT_ERROR;
	}

	_fill_property_metadata(v, *properties);
	g_variant_unref(v);

	return S_OK;
}

static gint compare_item_object_path(gconstpointer pa, gconstpointer pb)
{
	const char *list_path = (const char *)pa;
	const artik_bt_avrcp_item *play_item = (artik_bt_avrcp_item *)pb;

	return strcmp(list_path, play_item->item_obj_path);
}

artik_error bt_avrcp_controller_get_property(char *item,
				artik_bt_avrcp_item_property **properties)
{
	artik_bt_avrcp_item temp;

	temp.item_obj_path = item;
	if (!item)
		return E_BAD_ARGS;
	if (!g_slist_find_custom(items, &temp, compare_item_object_path))
		return E_BAD_ARGS;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));
	*properties = (artik_bt_avrcp_item_property *)
			malloc(sizeof(artik_bt_avrcp_item_property));
	memset(*properties, 0, sizeof(artik_bt_avrcp_item_property));

	_get_property_malloc_content(&((*properties)->player),
					item, "Player", "o");

	_get_property_malloc_content(&((*properties)->name),
					item, "Name", "s");

	_get_property_malloc_content(&((*properties)->type),
					item, "Type", "s");

	/*Only type is folder, FolderType & Playable is available*/
	if (g_strcmp0((*properties)->type, "folder") == 0) {
		_get_property_malloc_content(&((*properties)->folder),
					item, "FolderType", "s");

		_get_property_bool_content(&((*properties)->playable),
						item, "Playable");
	}

	/*only type is audio and video has below property*/
	if (g_strcmp0((*properties)->type, "audio") == 0
		|| g_strcmp0((*properties)->type, "video") == 0) {
		_get_property_metadata_content(properties, item);
	}

	return S_OK;
}

artik_error bt_avrcp_controller_play_item(char *item)
{
	artik_bt_avrcp_item temp;
	GVariant *result = NULL;
	GError *error = NULL;

	temp.item_obj_path = item;
	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (!g_slist_find_custom(items, &temp, compare_item_object_path))
		return E_BT_ERROR;

	result = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			temp.item_obj_path,
			DBUS_IF_MEDIAITEM1,
			"Play",
			NULL,
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("AVRCP Play failed :%s\n", error->message);
		g_clear_error(&error);
		return E_BT_ERROR;
	}

	g_variant_unref(result);
	return S_OK;
}

artik_error bt_avrcp_controller_add_to_playing(char *item)
{
	artik_bt_avrcp_item temp;

	temp.item_obj_path = item;

	if (!item)
		return E_BAD_ARGS;
	if (!g_slist_find_custom(items, &temp, compare_item_object_path))
		return E_BAD_ARGS;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	GVariant *result = NULL;
	GError *error = NULL;
	char *object_path = item;

	result = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			object_path,
			DBUS_IF_MEDIAITEM1,
			"AddtoNowPlaying",
			NULL,
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT, NULL, &error);

	if (error) {
		log_dbg("AVRCP AddtoNowPlaying failed :%s\n", error->message);
		g_clear_error(&error);
		return E_BT_ERROR;
	}

	g_variant_unref(result);
	return S_OK;
}

artik_error bt_avrcp_controller_get_name(char **name)
{
	char *name_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_player_path(&name_path);
	printf("out namepath\n");
	if (name_path)
		printf("get namepath:%s\n", name_path);
	if (e == S_OK && name_path) {
		e = _get_property(name_path,
			DBUS_IF_MEDIA_PLAYER1, "Name", &v);
		free(name_path);
		if (e == S_OK && v) {
			g_variant_get(v, "s", name);
			g_variant_unref(v);
			return S_OK;
		}
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_get_status(char **status)
{
	char *status_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_player_path(&status_path);
	if (e == S_OK && status_path) {
		e = _get_property(status_path,
			DBUS_IF_MEDIA_PLAYER1, "Status", &v);
		free(status_path);
		if (e == S_OK && v) {
			g_variant_get(v, "s", status);
			g_variant_unref(v);
			return S_OK;
		}
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_get_subtype(char **subtype)
{
	char *subtype_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_player_path(&subtype_path);
	if (e == S_OK && subtype_path) {
		e = _get_property(subtype_path,
			DBUS_IF_MEDIA_PLAYER1, "Subtype", &v);
		free(subtype_path);
		if (e == S_OK && v) {
			g_variant_get(v, "s", subtype);
			g_variant_unref(v);
			return S_OK;
		}
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_get_type(char **type)
{
	char *type_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_player_path(&type_path);
	if (e == S_OK && type_path) {
		e = _get_property(type_path,
			DBUS_IF_MEDIA_PLAYER1, "Type", &v);
		free(type_path);
		if (e == S_OK && v) {
			g_variant_get(v, "s", type);
			g_variant_unref(v);
			return S_OK;
		}
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_get_browsable(bool *is_browsable)
{
	char *browsable_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_player_path(&browsable_path);
	if (e == S_OK && browsable_path) {
		e = _get_property(browsable_path,
			DBUS_IF_MEDIA_PLAYER1, "Browsable", &v);
		free(browsable_path);
		if (e == S_OK && v) {
			g_variant_get(v, "b", is_browsable);
			g_variant_unref(v);
		} else
			*is_browsable = false;


		return S_OK;
	}
	return E_BT_ERROR;
}

artik_error bt_avrcp_controller_get_position(unsigned int *position)
{
	char *position_path = NULL;
	artik_error e = S_OK;
	GVariant *v = NULL;

	e = _get_player_path(&position_path);
	if (e == S_OK && position_path) {
		e = _get_property(position_path,
			DBUS_IF_MEDIA_PLAYER1, "Position", &v);
		free(position_path);
		if (e == S_OK && v) {
			g_variant_get(v, "u", position);
			g_variant_unref(v);
			return S_OK;
		}
	}
	return E_BT_ERROR;
}
