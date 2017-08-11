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

#include <artik_bluetooth.h>

#include "core.h"

artik_error bt_set_callback(artik_bt_event event,
		artik_bt_callback user_callback, void *user_data)
{
	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (event >= BT_EVENT_END)
		return E_BAD_ARGS;
	hci.callback[event].fn = user_callback;
	hci.callback[event].data = user_data;

	return S_OK;
}

artik_error bt_unset_callback(artik_bt_event event)
{
	bt_init(G_BUS_TYPE_SYSTEM, &(hci.conn));

	if (event >= BT_EVENT_END)
		return E_BAD_ARGS;

	hci.callback[event].fn = NULL;
	hci.callback[event].data = NULL;

	return S_OK;
}
