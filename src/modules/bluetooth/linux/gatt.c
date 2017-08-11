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

#include "core.h"
#include "helper.h"
#include "assigned_numbers.h"
#include "gatt.h"

artik_error bt_gatt_get_service_list(const char *addr, artik_bt_uuid * *uuid_list, int *len)
{
	gchar *dev_path;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_object_path(addr, &dev_path);
	if (dev_path == NULL)
		return E_BT_ERROR;

	_get_gatt_uuid_list(dev_path, DBUS_IF_GATTSERVICE1, uuid_list, len);

	g_free(dev_path);

	return S_OK;
}

artik_error bt_gatt_get_chracteristic_list(const char *addr, const char *srv_uuid,
		artik_bt_uuid **uuid_list, int *len)
{
	gchar *srv_path;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);

	if (srv_path == NULL)
		return E_BT_ERROR;

	_get_gatt_uuid_list(srv_path, DBUS_IF_GATTCHARACTERISTIC1,
			uuid_list, len);

	g_free(srv_path);

	return S_OK;
}

artik_error bt_gatt_get_descriptor_list(const char *addr,
		const char *srv_uuid, const char *char_uuid, artik_bt_uuid **uuid_list, int *len)
{
	gchar *char_path = NULL;
	gchar *srv_path = NULL;

	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	_get_gatt_path(addr, DBUS_IF_GATTSERVICE1, srv_uuid, NULL, NULL, &srv_path);
	if (srv_path == NULL)
		return E_BT_ERROR;

	_get_gatt_path(addr, DBUS_IF_GATTCHARACTERISTIC1, char_uuid, "Service", srv_path, &char_path);
	if (char_path == NULL)
		return E_BT_ERROR;

	_get_gatt_uuid_list(char_path, DBUS_IF_GATTDESCRIPTOR1, uuid_list, len);

	g_free(char_path);
	return S_OK;
}
