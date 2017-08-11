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

#ifndef __OS_LWM2M_H__
#define __OS_LWM2M_H__

#include <artik_error.h>

artik_error os_lwm2m_client_connect(artik_lwm2m_handle * handle,
	artik_lwm2m_config * config);

artik_error os_lwm2m_client_disconnect(artik_lwm2m_handle handle);

artik_error os_lwm2m_client_write_resource(artik_lwm2m_handle handle,
		const char *uri, unsigned char *buffer, int length);

artik_error os_lwm2m_client_read_resource(artik_lwm2m_handle handle,
		const char *uri, unsigned char *buffer, int *length);

artik_error os_lwm2m_set_callback(artik_lwm2m_handle handle,
		artik_lwm2m_event_t event, artik_lwm2m_callback user_callback,
		void *user_data);

artik_error os_lwm2m_unset_callback(artik_lwm2m_handle handle,
				artik_lwm2m_event_t event);

artik_lwm2m_object *os_lwm2m_create_device_object(
		const char *manufacturer, const char *model, const char *serial,
		const char *fw_version, const char *hw_version,
		const char *sw_version,	const char *device_type,
		int power_source, int power_volt, int power_current,
		int battery_level, int memory_total, int memory_free,
		const char *time_zone, const char *utc_offset,
		const char *binding);
artik_lwm2m_object *os_lwm2m_create_firmware_object(bool supported,
		char *pkg_name, char *pkg_version);
artik_lwm2m_object *os_lwm2m_create_connectivity_monitoring_object(
		int netbearer, int avlnetbearer, int signalstrength,
		int linkquality, int lenip, const char **ipaddr, int lenroute,
		const char **routeaddr, int linkutilization, const char *apn,
		int cellid, int smnc, int smcc);

void os_lwm2m_free_object(artik_lwm2m_object *object);

artik_error os_serialize_tlv_int(int *data, int size, unsigned char **buffer,
		int *lenbuffer);

artik_error os_serialize_tlv_string(char **data, int size,
		unsigned char **buffer, int *lenbuffer);


#endif  /* __OS_LWM2M_H__ */
