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

#include <string.h>

#include "core.h"
#include "assigned_numbers.h"
#include "helper.h"
#include "device.h"
#include <inttypes.h>

bt_handler hci = {0};
char session_path[SESSION_PATH_LEN];
artik_bt_ftp_property transfer_property = {0};

static void _free_func(gpointer data)
{
	g_free(data);
}

void _set_device_class(artik_bt_class *class, uint32_t cod)
{
	class->major = (uint16_t)(cod & 0x00001F00) >> 8;
	class->minor = (uint16_t)(cod & 0x000000FC);
	class->service_class = (uint32_t)(cod & 0x00FF0000);

	if (cod & 0x002000)
		class->service_class |= BT_SERVICE_CLASS_LIMITED_DISCOVERABLE_MODE;
}

static void _on_gatt_data_received(GVariant *properties, gchar *srv_uuid, gchar *char_uuid)
{
	GVariant *prop = NULL, *v = NULL, *v1 = NULL;
	gchar *key = NULL;
	guint i = 0, len = 0;
	artik_bt_gatt_data data;

	log_dbg("%s [%s]", __func__, char_uuid);

	memset(&data, 0x00, sizeof(artik_bt_gatt_data));

	prop = g_variant_get_child_value(properties, 0);
	g_variant_get(prop, "{&sv}", &key, &v);
	if (g_strcmp0(key, "Value") != 0)
		return;

	len = g_variant_n_children(v);

	data.srv_uuid = srv_uuid;
	data.char_uuid = char_uuid;
	data.length = len;
	data.bytes = (unsigned char *)malloc(sizeof(unsigned char) * len);

	for (i = 0; i < len; i++) {
		v1 = g_variant_get_child_value(v, i);
		data.bytes[i] = g_variant_get_byte(v1);
	}

	_user_callback(BT_EVENT_PF_CUSTOM, &data);

	g_variant_unref(prop);
	g_variant_unref(v);
	g_variant_unref(v1);
	g_free(data.bytes);
}

static void _on_hrp_measurement_received(GVariant *properties)
{
	GVariant *prop, *val, *v0, *v1, *v2, *v3;
	gchar *key;
	guchar flags = 0, hr = 0, ee = 0, ee_val = 0, format = 0,
			sc_status = 0, ee_status = 0;
	artik_bt_hrp_data data;

	memset(&data, 0x00, sizeof(artik_bt_hrp_data));

	prop = g_variant_get_child_value(properties, 0);
	g_variant_get(prop, "{&sv}", &key, &val);
	if (g_strcmp0(key, "Value") != 0)
		return;

	if (g_variant_n_children(val) < 3)
		return;
	v0 = g_variant_get_child_value(val, 0);
	v1 = g_variant_get_child_value(val, 1);
	v2 = g_variant_get_child_value(val, 2);
	v3 = g_variant_get_child_value(val, 3);

	g_variant_get(v0, "y", &flags);
	g_variant_get(v1, "y", &hr);
	g_variant_get(v2, "y", &ee);
	g_variant_get(v3, "y", &ee_val);

	format = flags & 0x01;
	sc_status = (flags >> 1) & 0x03;
	ee_status = flags & 0x08;
	log_dbg("flags: 0x%02x, format: 0x%02x, sc_status: 0x%02x, ee_status: 0x%02x",
			flags, format, sc_status, ee_status);

	if (format != 0x00)
		hr = hr | (ee << 8);

	if (ee_status) {
		log_dbg("energy extended : %d", ee_val);
		data.energy = ee_val;
	}

	log_dbg("heart rate: %d", hr);

	data.bpm = hr;
	if (sc_status == 3)
		data.contact = true;
	else
		data.contact = false;

	_user_callback(BT_EVENT_PF_HEARTRATE, &data);

	g_variant_unref(prop);
	g_variant_unref(val);
	g_variant_unref(v0);
	g_variant_unref(v1);
	g_variant_unref(v2);
	g_variant_unref(v3);
}

artik_error bt_init(GBusType dbus_type, GDBusConnection **connection)
{
	GError *e = NULL;
	guint subscribe_id;

	if (!connection)
		return E_INVALID_VALUE;

	if (*connection)
		return S_OK;

	*connection = g_bus_get_sync(dbus_type, NULL, &e);

	if (e) {
		g_warning("Failed to get bus: %s", e->message);
		g_clear_error(&e);
		return E_BT_ERROR;
	}

	if (hci.subscribe_ids == NULL)
		hci.subscribe_ids = g_hash_table_new(g_str_hash, g_str_equal);
	if (hci.registration_ids == NULL)
		hci.registration_ids = g_hash_table_new(g_str_hash, g_str_equal);

	subscribe_id = g_dbus_connection_signal_subscribe(*connection,
			NULL, DBUS_IF_OBJECT_MANAGER, "InterfacesAdded",
			NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _dbus_signal_callback,
			NULL, NULL);

	g_hash_table_insert(hci.subscribe_ids,
			g_strdup("InterfacesAdded"), GUINT_TO_POINTER(subscribe_id));

	subscribe_id = g_dbus_connection_signal_subscribe(*connection,
			NULL, DBUS_IF_OBJECT_MANAGER, "InterfacesRemoved",
			NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _dbus_signal_callback,
			NULL, NULL);

	g_hash_table_insert(hci.subscribe_ids,
			g_strdup("InterfacesRemoved"), GUINT_TO_POINTER(subscribe_id));

	subscribe_id = g_dbus_connection_signal_subscribe(*connection,
			NULL, DBUS_IF_PROPERTIES, "PropertiesChanged",
			NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _dbus_signal_callback,
			NULL, NULL);

	g_hash_table_insert(hci.subscribe_ids,
		g_strdup("PropertiesChanged"), GUINT_TO_POINTER(subscribe_id));

	return S_OK;
}

artik_error bt_deinit(void)
{
	g_dbus_connection_signal_unsubscribe(hci.conn,
		GPOINTER_TO_INT(g_hash_table_lookup(hci.subscribe_ids,
			"InterfacesAdded")));

	g_dbus_connection_signal_unsubscribe(hci.conn,
		GPOINTER_TO_INT(g_hash_table_lookup(hci.subscribe_ids,
			"PropertiesChanged")));

	return S_OK;
}

void _user_callback(artik_bt_event event, void *data)
{
	void *user_data = NULL;

	if (hci.callback[event].fn == NULL)
		return;

	log_dbg("%s [%d]", __func__, event);

	switch (event) {
	case BT_EVENT_SCAN:
		user_data = (artik_bt_device *)data;
		break;
	case BT_EVENT_BOND:
	case BT_EVENT_CONNECT:
		user_data = (gboolean *)data;
		break;
	case BT_EVENT_PROXIMITY:
		user_data = (artik_bt_gatt_data *)data;
		break;
	case BT_EVENT_PF_HEARTRATE:
		user_data = (artik_bt_hrp_data *)data;
		break;
	case BT_EVENT_FTP:
		user_data = (artik_bt_ftp_property *)data;
		break;
	case BT_EVENT_GATT_PROPERTY:
		break;
	case BT_EVENT_PF_CUSTOM:
		user_data = (artik_bt_gatt_data *)data;
		break;
	default:
		break;
	}

	hci.callback[event].fn(event, user_data, hci.callback[event].data);
}

static void _device_properties_changed(GVariant *properties)
{
	GVariant *val;
	GVariantIter *iter;
	gchar *key;
	gboolean connected = false;
	gboolean paired = false;

	g_variant_get(properties, "a{sv}", &iter);
	while (g_variant_iter_loop(iter, "{&sv}", &key, &val)) {
		if (g_strcmp0(key, "Connected") == 0) {
			connected = g_variant_get_boolean(val);
			if (hci.state == BT_DEVICE_STATE_IDLE)
				_user_callback(BT_EVENT_CONNECT, &connected);
			g_variant_unref(val);
			break;
		} else if (g_strcmp0(key, "Paired") == 0) {
			paired = g_variant_get_boolean(val);
			if (hci.state == BT_DEVICE_STATE_IDLE)
				_user_callback(BT_EVENT_BOND, &paired);
			g_variant_unref(val);
			break;
		} else if (g_strcmp0(key, "ServiceResolved") == 0) {
			g_variant_unref(val);
			break;
		}
	}

	g_variant_iter_free(iter);
}

static void _proximity_properties_changed(GVariant *variant)
{
	GVariant *key_variant, *val_variant;

	artik_bt_gatt_data bt_pxp_data;

	key_variant = g_variant_get_child_value(variant, 0);
	val_variant = g_variant_get_child_value(key_variant, 1);
	g_variant_get(g_variant_get_child_value(key_variant, 0), "s",
			&(bt_pxp_data.key));
	g_variant_get(g_variant_get_child_value(val_variant, 0), "s",
			&(bt_pxp_data.value));

	_user_callback(BT_EVENT_PROXIMITY, &(bt_pxp_data));

	g_variant_unref(key_variant);
	g_variant_unref(val_variant);
}

static void _obex_properties_changed(const char *object_path, GVariant *properties)
{
	if (transfer_property.object_path == NULL)
		return;

	if (g_strcmp0(object_path, transfer_property.object_path))
		return;

	_fill_transfer_property(properties);

	_user_callback(BT_EVENT_FTP, &transfer_property);

	if (!g_strcmp0(transfer_property.status, "complete") || !g_strcmp0(transfer_property.status, "error")) {
		g_free(transfer_property.object_path);
		transfer_property.object_path = NULL;
	}
}

static void _pan_properties_changed(GVariant *properties)
{
	GVariant *val;
	GVariantIter *iter;
	gchar *key;
	gboolean connected = false;

	g_variant_get(properties, "a{sv}", &iter);
	while (g_variant_iter_loop(iter, "{&sv}", &key, &val)) {
		if (g_strcmp0(key, "Connected") == 0) {
			connected = g_variant_get_boolean(val);
			log_dbg("_user_callback: %d\n", hci.state);
			_user_callback(BT_EVENT_CONNECT, &connected);
			g_variant_unref(val);
			break;
		}
	}
	g_variant_iter_free(iter);
}


void _get_adapter_properties(GVariant *prop_array, artik_bt_adapter *adapter)
{
	GVariant *prop_dict, *v, *uuid;
	gchar *key = NULL;
	gint i = 0, j = 0, prop_len = 0, uuid_len = 0;
	uint32_t cod;

	if (!adapter)
		return;
	memset(adapter, 0x00, sizeof(artik_bt_adapter));

	prop_len = g_variant_n_children(prop_array);
	if (prop_len < 1)
		return;

	for (i = 0; i < prop_len; i++) {
		prop_dict = g_variant_get_child_value(prop_array, i);
		g_variant_get(prop_dict, "{&sv}", &key, &v);

		if (g_strcmp0(key, "Address") == 0) {
			g_variant_get(v, "s", &adapter->address);
		} else if (g_strcmp0(key, "Name") == 0) {
			g_variant_get(v, "s", &adapter->name);
		} else if (g_strcmp0(key, "Alias") == 0) {
			g_variant_get(v, "s", &adapter->alias);
		} else if (g_strcmp0(key, "Class") == 0) {
			g_variant_get(v, "u", &cod);
			_set_device_class(&adapter->cod, cod);
		} else if (g_strcmp0(key, "Discoverable") == 0) {
			g_variant_get(v, "b", &adapter->discoverable);
		} else if (g_strcmp0(key, "Discovering") == 0) {
			g_variant_get(v, "b", &adapter->discovering);
		} else if (g_strcmp0(key, "Pairable") == 0) {
			g_variant_get(v, "b", &adapter->pairable);
		} else if (g_strcmp0(key, "PairableTimeout") == 0) {
			g_variant_get(v, "u", &adapter->pair_timeout);
		} else if (g_strcmp0(key, "DiscoverableTimeout") == 0) {
			g_variant_get(v, "u", &adapter->discover_timeout);
		} else if (g_strcmp0(key, "UUIDs") == 0) {
			uuid_len = g_variant_n_children(v);
			adapter->uuid_length = uuid_len;
			if (uuid_len > 0) {
				adapter->uuid_list
					= (artik_bt_uuid *)malloc(sizeof(artik_bt_uuid) * uuid_len);
				for (j = 0; j < uuid_len; j++) {
					uuid = g_variant_get_child_value(v, j);

					g_variant_get(uuid, "s", &adapter->uuid_list[j].uuid);
					adapter->uuid_list[j].uuid_name
						= g_strdup(_get_uuid_name(adapter->uuid_list[j].uuid));

					g_variant_unref(uuid);
				}
			}
		}
	}

	g_variant_unref(prop_dict);
	g_variant_unref(v);
}

void _get_device_properties(GVariant *prop_array, artik_bt_device *device)
{
	GVariant *prop_dict, *v, *uuid, *v_mfr, *v_mfr_data, *v_byte,
		*v_svc, *v_svc_data;
	gchar *key = NULL, *svc_uuid = NULL;
	gint i = 0, j = 0, prop_len = 0, uuid_len = 0;
	guint32 cod;
	guint16 mfr_id = 0;

	if (!device)
		return;
	memset(device, 0x00, sizeof(artik_bt_device));

	prop_len = g_variant_n_children(prop_array);
	if (prop_len < 1)
		return;

	for (i = 0; i < prop_len; i++) {
		prop_dict = g_variant_get_child_value(prop_array, i);
		g_variant_get(prop_dict, "{&sv}", &key, &v);

		if (g_strcmp0(key, "Address") == 0) {
			g_variant_get(v, "s", &device->remote_address);
		} else if (g_strcmp0(key, "Name") == 0) {
			g_variant_get(v, "s", &device->remote_name);
		} else if (g_strcmp0(key, "Class") == 0) {
			g_variant_get(v, "u", &cod);
			_set_device_class(&device->cod, cod);
		} else if (g_strcmp0(key, "RSSI") == 0) {
			g_variant_get(v, "n", &device->rssi);
		} else if (g_strcmp0(key, "Paired") == 0) {
			g_variant_get(v, "b", &device->is_bonded);
		} else if (g_strcmp0(key, "Connected") == 0) {
			g_variant_get(v, "b", &device->is_connected);
		} else if (g_strcmp0(key, "UUIDs") == 0) {
			uuid_len = g_variant_n_children(v);
			device->uuid_length = uuid_len;
			if (uuid_len > 0) {
				device->uuid_list
					= (artik_bt_uuid *)malloc(sizeof(artik_bt_uuid) * uuid_len);
				for (j = 0; j < uuid_len; j++) {
					uuid = g_variant_get_child_value(v, j);

					g_variant_get(uuid, "s", &device->uuid_list[j].uuid);
					device->uuid_list[j].uuid_name
						= g_strdup(_get_uuid_name(device->uuid_list[j].uuid));

					g_variant_unref(uuid);
				}
			}
		} else if (g_strcmp0(key, "ManufacturerData") == 0) {
			v_mfr = g_variant_get_child_value(v, 0);
			g_variant_get(v_mfr, "{qv}", &mfr_id, &v_mfr_data);

			device->manufacturer_id = mfr_id;
			strncpy(device->manufacturer_name, _get_company_name(mfr_id),
					MAX_BT_NAME_LEN);

			device->manufacturer_data_len = g_variant_n_children(v_mfr_data);
			if (device->manufacturer_data_len > 0) {
				device->manufacturer_data
						= (char *)malloc(device->manufacturer_data_len);
				for (j = 0; j < device->manufacturer_data_len; j++) {
					v_byte = g_variant_get_child_value(v_mfr_data, j);
					g_variant_get(v_byte, "y", &device->manufacturer_data[j]);
					g_variant_unref(v_byte);
				}
			}
			g_variant_unref(v_mfr);
			g_variant_unref(v_mfr_data);
		} else if (g_strcmp0(key, "ServiceData") == 0) {
			v_svc = g_variant_get_child_value(v, 0);
			g_variant_get(v_svc, "{&sv}", &svc_uuid, &v_svc_data);

			strncpy(device->svc_uuid, svc_uuid, MAX_BT_UUID_LEN);

			device->svc_data_len = g_variant_n_children(v_svc_data);
			if (device->svc_data_len > 0) {
				device->svc_data = (char *)malloc(device->svc_data_len);
				for (j = 0; j < device->svc_data_len; j++) {
					v_byte = g_variant_get_child_value(v_svc_data, j);
					g_variant_get(v_byte, "y", &device->svc_data[j]);
					g_variant_unref(v_byte);
				}
			}
			g_variant_unref(v_svc);
			g_variant_unref(v_svc_data);
		}
	}

	g_variant_unref(prop_dict);
	g_variant_unref(v);
}

artik_error _get_managed_objects(GVariant **variant)
{
	GError *e = NULL;

	*variant = g_dbus_connection_call_sync(hci.conn,
			DBUS_BLUEZ_BUS, "/", DBUS_IF_OBJECT_MANAGER,
			"GetManagedObjects",
			NULL, NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &e);

	return bt_check_error(e);
}

void _get_object_path(const char *addr, char **path)
{
	GVariant *obj1, *ar1, *ar2;
	GVariantIter *iter1, *iter2;
	gchar *dev_path, *itf;
	artik_bt_device device;

	*path = NULL;

	if (_get_managed_objects(&obj1) != S_OK)
		return;

	g_variant_get(obj1, "(a{oa{sa{sv}}})", &iter1);
	while (g_variant_iter_loop(iter1, "{&o@a{sa{sv}}}", &dev_path, &ar1)) {

		if (*path != NULL) {
			g_variant_unref(ar1);
			break;
		}

		g_variant_get(ar1, "a{sa{sv}}", &iter2);
		while (g_variant_iter_loop(iter2, "{&s@a{sv}}", &itf, &ar2)) {

			if (strcasecmp(itf, DBUS_IF_DEVICE1) != 0)
				continue;

			_get_device_properties(ar2, &device);
			if (strcasecmp(device.remote_address, addr) == 0) {
				*path = g_strdup(dev_path);
				g_variant_unref(ar2);
				break;
			}
		}
		g_variant_iter_free(iter2);
	}
	g_variant_iter_free(iter1);
	g_variant_unref(obj1);
}

artik_error _get_devices(bt_device_state state,
		artik_bt_device **device_list, int *count)
{
	GVariant *objects;
	GVariant *path_array, *path_item;
	GVariant *if_array, *if_item;
	GVariant *prop_array;
	gchar *path, *interface;
	gsize path_count = 0, if_count = 0;
	guint i = 0, j = 0;
	int cnt = 0;
	artik_bt_device *tmp_list;
	artik_error ret = S_OK;

	log_dbg("");

	ret = _get_managed_objects(&objects);
	if (ret != S_OK)
		return ret;

	path_array = g_variant_get_child_value(objects, 0);
	path_count = g_variant_n_children(path_array);

	/* ignore AgentManager1 and Adapter1 */
	tmp_list = (artik_bt_device *)malloc(sizeof(artik_bt_device)
			* (path_count - 2));

	for (i = 2; i < path_count; i++) {
		path_item = g_variant_get_child_value(path_array, i);
		g_variant_get(path_item, "{&o@a{sa{sv}}}", &path, &if_array);

		if_count = g_variant_n_children(if_array);

		for (j = 0; j < if_count; j++) {
			if_item = g_variant_get_child_value(if_array, j);
			g_variant_get(if_item, "{&s@a{sv}}", &interface, &prop_array);

			if (strcasecmp(interface, DBUS_IF_DEVICE1) == 0) {
				switch (state) {
				case BT_DEVICE_STATE_PAIRED:
					if (_is_paired(path))
						_get_device_properties(prop_array, &tmp_list[cnt++]);
					break;
				case BT_DEVICE_STATE_CONNECTED:
					if (_is_connected(path))
						_get_device_properties(prop_array, &tmp_list[cnt++]);
					break;
				default:
					_get_device_properties(prop_array, &tmp_list[cnt++]);
					break;
				}
			}
			g_variant_unref(if_item);
			g_variant_unref(prop_array);
		}
		g_variant_unref(path_item);
		g_variant_unref(if_array);
	}

	*count = cnt;
	*device_list = tmp_list;

	g_variant_unref(objects);
	g_variant_unref(path_array);

	return ret;
}

void _get_gatt_path(const char *addr, const char *interface,
		const char *uuid, const char *property, const char *value, gchar **gatt_path)
{
	GVariant *obj1, *ar1, *ar2, *val;
	GVariantIter *iter1, *iter2, *iter3;
	gchar *path, *dev_path, *itf, *key;

	*gatt_path = NULL;

	_get_object_path(addr, &dev_path);
	if (dev_path == NULL)
		return;

	if (_get_managed_objects(&obj1) != S_OK)
		return;

	g_variant_get(obj1, "(a{oa{sa{sv}}})", &iter1);
	while (g_variant_iter_loop(iter1, "{&o@a{sa{sv}}}", &path, &ar1)) {
		if (*gatt_path != NULL) {
			g_variant_unref(ar1);
			break;
		}

		if (!g_str_has_prefix(path, dev_path))
			continue;

		g_variant_get(ar1, "a{sa{sv}}", &iter2);
		while (g_variant_iter_loop(iter2, "{&s@a{sv}}", &itf, &ar2)) {

			if (*gatt_path != NULL) {
				g_variant_unref(ar2);
				break;
			}

			if (g_strcmp0(itf, interface) != 0)
				continue;

			bool is_good_uuid = false;
			bool is_good_property = false;

			if (property == NULL)
				is_good_property = true;

			g_variant_get(ar2, "a{sv}", &iter3);
			while (g_variant_iter_loop(iter3, "{&sv}", &key, &val)) {
				if (!g_strcmp0(key, "UUID")) {
					const gchar *id = g_variant_get_string(val, NULL);

					if (!g_strcmp0(uuid, id))
						is_good_uuid = true;
				}

				if (!g_strcmp0(key, property)) {
					const gchar *str_val = g_variant_get_string(val, NULL);

					if (!g_strcmp0(value, str_val))
						is_good_property = true;
				}

				if (is_good_property && is_good_uuid) {
					*gatt_path = strdup(path);
					g_variant_unref(val);
					break;
				}
			}
			g_variant_iter_free(iter3);
		}
		g_variant_iter_free(iter2);
	}
	g_variant_iter_free(iter1);
	g_variant_unref(obj1);
	g_free(dev_path);
}

void _get_gatt_uuid_list(const char *gatt_path, const char *interface,
		artik_bt_uuid **uuid_list, int *len)
{
	GVariant *obj1, *ar1, *ar2, *val;
	GVariantIter *iter1, *iter2, *iter3;
	gchar *path, *itf, *key;
	const gchar *uuid;
	artik_bt_uuid *uuids;
	GSList *srv_list = NULL;
	guint uuid_len = 0;
	unsigned int i = 0;

	*uuid_list = NULL;
	*len = 0;

	if (_get_managed_objects(&obj1) != S_OK)
		return;

	g_variant_get(obj1, "(a{oa{sa{sv}}})", &iter1);
	while (g_variant_iter_loop(iter1, "{&o@a{sa{sv}}}", &path, &ar1)) {
		if (!g_str_has_prefix(path, gatt_path))
			continue;

		g_variant_get(ar1, "a{sa{sv}}", &iter2);
		while (g_variant_iter_loop(iter2, "{&s@a{sv}}", &itf, &ar2)) {
			if (g_strcmp0(itf, interface) != 0)
				continue;

			g_variant_get(ar2, "a{sv}", &iter3);
			while (g_variant_iter_loop(iter3, "{&sv}", &key, &val)) {
				if (g_strcmp0(key, "UUID") != 0)
					continue;

				uuid = g_variant_get_string(val, NULL);
				srv_list = g_slist_append(srv_list, strdup(uuid));
			}
			g_variant_iter_free(iter3);
		}
		g_variant_iter_free(iter2);
	}
	g_variant_iter_free(iter1);

	uuid_len = g_slist_length(srv_list);
	uuids = (artik_bt_uuid *)malloc(sizeof(artik_bt_uuid) * uuid_len);

	for (i = 0; i < uuid_len; i++) {
		uuids[i].uuid = g_strdup(g_slist_nth_data(srv_list, i));
		uuids[i].uuid_name = g_strdup(_get_uuid_name(uuids[i].uuid));
	}

	*len = uuid_len;
	*uuid_list = uuids;

	g_slist_free_full(srv_list, _free_func);
	g_variant_unref(obj1);
}

static void _process_gatt_service(gchar *path)
{
	/* TODO: process gatt service here */
}

static void _gatt_properties_changed(const gchar *object_path,
		GVariant *properties)
{
	GVariant *r, *v;
	guint i = 0, len = 0;
	bt_gatt_client *client;

	r = g_dbus_connection_call_sync(
			hci.conn,
			DBUS_BLUEZ_BUS,
			object_path,
			DBUS_IF_PROPERTIES,
			"Get",
			g_variant_new("(ss)", DBUS_IF_GATTCHARACTERISTIC1, "UUID"),
			NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, NULL);

	g_variant_get(r, "(v)", &v);
	if (g_strcmp0(g_variant_get_string(v, NULL),
			UUID_HEART_RATE_MEASUREMENT) == 0) {
		_on_hrp_measurement_received(properties);

	} else {
		len = g_slist_length(hci.gatt_clients);
		for (i = 0; i < len; i++) {
			client =  g_slist_nth_data(hci.gatt_clients, i);
			if (g_strcmp0(object_path, client->path) == 0) {
				_on_gatt_data_received(properties, client->srv_uuid, client->char_uuid);
				break;
			}
		}
	}
	g_variant_unref(r);
	g_variant_unref(v);
}

static gboolean _on_timeout(gpointer user_data)
{
	gboolean b = true;

	_user_callback(BT_EVENT_GATT_PROPERTY, &b);

	return G_SOURCE_REMOVE;
}

void _on_interface_added(const gchar *sender_name,
	const gchar *object_path, const gchar *interface_name,
	GVariant *parameters, gpointer user_data)
{
	GVariantIter *iter;
	GVariant *device_array, *prop_array;
	gchar *path = NULL;
	gchar *interface = NULL;
	artik_bt_device *device = NULL;

	g_variant_get(parameters, "(&o@a{sa{sv}})", &path, &device_array);
	log_dbg("InterfacesAdded [%s]", path);

	g_variant_get(device_array, "a{sa{sv}}", &iter);
	while (g_variant_iter_loop(iter, "{&s@a{sv}}", &interface, &prop_array)) {
		if (g_strcmp0(interface, DBUS_IF_DEVICE1) == 0) {
			device = (artik_bt_device *)malloc(sizeof(artik_bt_device));
			_get_device_properties(prop_array, device);
			_user_callback(BT_EVENT_SCAN, device);
			bt_free_device(device);
		} else if (g_strcmp0(interface, DBUS_IF_GATTSERVICE1) == 0) {
			if (hci.source != NULL)
				g_source_destroy(hci.source);

			hci.source = g_timeout_source_new(30);
			g_source_set_callback(hci.source, _on_timeout, NULL, NULL);
			g_source_attach(hci.source, NULL);

			/* TODO: DBUS_IF_GATTSERVICE1 handling */
			_process_gatt_service(path);
			log_dbg("[NEW] ftp added %s\n", path);
		} else if (g_strcmp0(interface, DBUS_IF_OBEX_SESSION) == 0) {
			log_dbg("[NEW] session added %s\n", path);
			strncpy(session_path, path, strlen(path));
			session_path[strlen(path)] = '\0';
		} else if (g_strcmp0(interface, DBUS_IF_OBEX_TRANSFER) == 0) {
			if (transfer_property.object_path != NULL)
				free(transfer_property.object_path);
			transfer_property.object_path = (char *) malloc(strlen(path) + 1);
			strncpy(transfer_property.object_path, path, strlen(path));
			transfer_property.object_path[strlen(path)] = '\0';

			if (transfer_property.file_name != NULL) {
				free(transfer_property.file_name);
				transfer_property.file_name = NULL;
			}

			if (transfer_property.name != NULL) {
				free(transfer_property.name);
				transfer_property.name = NULL;
			}

			if (transfer_property.status != NULL) {
				free(transfer_property.status);
				transfer_property.status = NULL;
			}
			transfer_property.transfered = 0;
			transfer_property.size = 0;
			_fill_transfer_property(prop_array);
		}
	}
	g_variant_iter_free(iter);
}

void _on_interface_removed(const gchar *sender_name,
	const gchar *object_path, const gchar *interface_name,
	GVariant *parameters, gpointer user_data)
{
	GVariantIter *iter;
	gchar *path = NULL;
	gchar *interface = NULL;

	g_variant_get(parameters, "(oas)", &path, &iter);
	log_dbg("InterfacesRemoved [%s]", path);

	while (g_variant_iter_loop(iter, "s", &interface)) {
		if (g_strcmp0(interface, DBUS_IF_OBEX_SESSION) == 0) {
			memset(session_path, 0, SESSION_PATH_LEN);
		} else if (g_strcmp0(interface, DBUS_IF_OBEX_TRANSFER) == 0) {
			if (transfer_property.object_path != NULL) {
				free(transfer_property.object_path);
				transfer_property.object_path = NULL;
			}

			if (transfer_property.file_name != NULL) {
				free(transfer_property.file_name);
				transfer_property.file_name = NULL;
			}

			if (transfer_property.name != NULL) {
				free(transfer_property.name);
				transfer_property.name = NULL;
			}

			if (transfer_property.status != NULL) {
				free(transfer_property.status);
				transfer_property.status = NULL;
			}
			transfer_property.transfered = 0;
			transfer_property.size = 0;
		}
	}
	g_variant_iter_free(iter);
}

void _on_properties_changed(const gchar *sender_name,
	const gchar *object_path, const gchar *interface_name,
	GVariant *parameters, gpointer user_data)
{
	GVariant *properties;
	gchar *interface;

	g_variant_get(parameters, "(&s@a{sv}@as)", &interface, &properties, NULL);

	if (g_str_has_prefix(object_path, DBUS_BLUEZ_OBJECT_PATH)) {
		log_dbg("%s %s %s", __func__, object_path, interface_name);
		print_variant(parameters);
		log_dbg("%s-interface: %s", __func__, interface);

		if (g_strcmp0(DBUS_IF_DEVICE1, interface) == 0) {
			_device_properties_changed(properties);

		} else if (g_strcmp0(DBUS_IF_PROXIMITYREPORTER1, interface) == 0 ||
				g_strcmp0(DBUS_IF_PROXIMITYMONITOR1, interface) == 0) {
			g_print("interface : [%s]\n", interface);
			_proximity_properties_changed(properties);

		} else if (g_strcmp0(DBUS_IF_GATTCHARACTERISTIC1, interface) == 0) {
			_gatt_properties_changed(object_path, properties);

		} else if (g_strcmp0(DBUS_IF_OBEX_TRANSFER, interface) == 0) {
			_obex_properties_changed(object_path, properties);

		} else if (g_strcmp0(DBUS_IF_NETWORK1, interface) == 0) {
			_pan_properties_changed(properties);
		}
	} else if (g_str_has_prefix(object_path, GATT_SERVICE_PREFIX)) {
		log_dbg("%s %s %s", __func__, object_path, interface_name);
		print_variant(parameters);

		if (g_strcmp0(DBUS_IF_GATTCHARACTERISTIC1, interface) == 0) {
			/* TODO: We can pass on changed properties to the higher layer */
			;
		}
	}

	g_variant_unref(properties);
}

void _dbus_signal_callback(GDBusConnection *conn,
	const gchar *sender_name, const gchar *object_path,
	const gchar *interface_name, const gchar *signal_name,
	GVariant *parameters, gpointer user_data)
{
	if (g_strcmp0(signal_name, "InterfacesAdded") == 0) {
		_on_interface_added(sender_name, object_path, interface_name,
			parameters, user_data);
	} else if (g_strcmp0(signal_name, "PropertiesChanged") == 0) {
		_on_properties_changed(sender_name, object_path, interface_name,
			parameters, user_data);
	} else if (g_strcmp0(signal_name, "InterfacesRemoved") == 0) {
		_on_interface_removed(sender_name, object_path, interface_name,
			parameters, user_data);
	}
}

gboolean _is_connected(const char *device_path)
{
	GVariant *rst = NULL;
	GVariant *v = NULL;
	gboolean b = FALSE;
	GError *error = NULL;

	rst = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		device_path,
		DBUS_IF_PROPERTIES,
		"Get",
		g_variant_new("(ss)", DBUS_IF_DEVICE1, "Connected"),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (bt_check_error(error) != S_OK)
		return b;

	g_variant_get(rst, "(v)", &v);
	g_variant_get(v, "b", &b);
	g_variant_unref(rst);
	g_variant_unref(v);

	return b;
}

gboolean _is_paired(const char *device_path)
{
	GVariant *rst = NULL;
	GVariant *v = NULL;
	gboolean b = FALSE;
	GError *error = NULL;

	rst = g_dbus_connection_call_sync(hci.conn,
		DBUS_BLUEZ_BUS,
		device_path,
		DBUS_IF_PROPERTIES,
		"Get",
		g_variant_new("(ss)", DBUS_IF_DEVICE1, "Paired"),
		NULL, G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

	if (bt_check_error(error) != S_OK)
		return b;

	g_variant_get(rst, "(v)", &v);
	g_variant_get(v, "b", &b);
	g_variant_unref(rst);
	g_variant_unref(v);

	return b;
}

void _fill_transfer_property(GVariant *dict)
{
	GVariantIter *iter = NULL;
	gchar *key = NULL;
	GVariant *val = NULL;

	g_variant_get(dict, "a{sv}", &iter);
	while (g_variant_iter_loop(iter, "{&sv}", &key, &val))
	{
		if (g_strcmp0(key, "Filename") == 0) {
			if (transfer_property.file_name != NULL)
				g_free(transfer_property.file_name);

			g_variant_get(val, "s", &transfer_property.file_name);

		} else if (g_strcmp0(key, "Name") == 0) {
			if (transfer_property.name != NULL)
				g_free(transfer_property.name);

			g_variant_get(val, "s", &transfer_property.name);

		} else if (g_strcmp0(key, "Status") == 0) {
			if (transfer_property.status != NULL)
				g_free(transfer_property.status);

			g_variant_get(val, "s", &transfer_property.status);
		} else if (g_strcmp0(key, "Transferred") == 0) {
			g_variant_get(val, "t", &transfer_property.transfered);

		} else if (g_strcmp0(key, "Size") == 0) {
			g_variant_get(val, "t", &transfer_property.size);
		}
	}

	g_variant_iter_free(iter);
}

artik_error bt_check_error(GError *err)
{
	if (!err)
		return S_OK;

	/* Check for "Unit dbus-org.bluez.service not found" error */
	if (g_error_matches(err, g_quark_from_static_string("g-io-error-quark"), 36)) {
		log_dbg(err->message);
		return E_BUSY;
	}

	log_dbg(err->message);
	g_error_free(err);

	return E_BT_ERROR;
}

void _get_device_address(const gchar *path, gchar **address)
{
	GVariant *tuple, *v;

	tuple = g_dbus_connection_call_sync(hci.conn,
					    DBUS_BLUEZ_BUS,
					    path,
					    DBUS_IF_PROPERTIES,
					    "Get",
					    g_variant_new("(ss)", DBUS_IF_DEVICE1, "Address"),
					    NULL,
					    G_DBUS_CALL_FLAGS_NONE,
					    G_MAXINT,
					    NULL,
					    NULL);

	g_variant_get(tuple, "(v)", &v);
	g_variant_get(v, "s", address);

	g_variant_unref(tuple);
	g_variant_unref(v);
}
