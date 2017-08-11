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

#include <artik_module.h>
#include <artik_log.h>
#include <artik_lwm2m.h>
#include "os_lwm2m.h"

static artik_error client_connect(artik_lwm2m_handle * handle,
				artik_lwm2m_config * config);
static artik_error client_disconnect(artik_lwm2m_handle handle);
static artik_error client_write_resource(artik_lwm2m_handle handle,
		const char *uri, unsigned char *buffer, int length);
static artik_error client_read_resource(artik_lwm2m_handle handle,
		const char *uri, unsigned char *buffer, int *length);
static artik_error set_callback(artik_lwm2m_handle handle,
		artik_lwm2m_event_t event, artik_lwm2m_callback user_callback,
		void *user_data);
static artik_error unset_callback(artik_lwm2m_handle handle,
				artik_lwm2m_event_t event);
static artik_lwm2m_object *create_device_object(const char *manufacturer,
		const char *model, const char *serial, const char *fw_version,
		const char *hw_version, const char *sw_version,
		const char *device_type, int power_source, int power_volt,
		int power_current, int battery_level, int memory_total,
		int memory_free, const char *time_zone, const char *utf_offset,
		const char *binding);
static artik_lwm2m_object *create_firmware_object(bool supported,
		char *pkg_name, char *pkg_version);
static artik_lwm2m_object *create_connectivity_monitoring_object(int netbearer,
		int avlnetbearer, int signalstrength, int linkquality,
		int lenip, const char **ipaddr, int lenroute,
		const char **routeaddr, int linkutilization, const char *apn,
		int cellid, int smnc, int smcc);
static void free_object(artik_lwm2m_object *object);
static artik_error serialize_tlv_int(int *data, int size,
		unsigned char **buffer, int *lenbuffer);
static artik_error serialize_tlv_string(char **data, int size,
		unsigned char **buffer, int *lenbuffer);

const artik_lwm2m_module lwm2m_module = {
	client_connect,
	client_disconnect,
	client_write_resource,
	client_read_resource,
	set_callback,
	unset_callback,
	create_device_object,
	create_firmware_object,
	create_connectivity_monitoring_object,
	free_object,
	serialize_tlv_int,
	serialize_tlv_string
};

artik_error client_connect(artik_lwm2m_handle *handle,
			artik_lwm2m_config *config)
{
	return os_lwm2m_client_connect(handle, config);
}

artik_error client_disconnect(artik_lwm2m_handle handle)
{
	return os_lwm2m_client_disconnect(handle);
}

artik_error client_write_resource(artik_lwm2m_handle handle, const char *uri,
		unsigned char *buffer, int length)
{
	return os_lwm2m_client_write_resource(handle, uri, buffer, length);
}

artik_error client_read_resource(artik_lwm2m_handle handle, const char *uri,
		unsigned char *buffer, int *length)
{
	return os_lwm2m_client_read_resource(handle, uri, buffer, length);
}

artik_error set_callback(artik_lwm2m_handle handle, artik_lwm2m_event_t event,
		artik_lwm2m_callback user_callback, void *user_data)
{
	return os_lwm2m_set_callback(handle, event, user_callback, user_data);
}

artik_error unset_callback(artik_lwm2m_handle handle, artik_lwm2m_event_t event)
{
	return os_lwm2m_unset_callback(handle, event);
}

artik_lwm2m_object *create_device_object(const char *manufacturer,
		const char *model, const char *serial, const char *fw_version,
		const char *hw_version, const char *sw_version,
		const char *device_type, int power_source, int power_volt,
		int power_current, int battery_level, int memory_total,
		int memory_free, const char *time_zone, const char *utc_offset,
		const char *binding)
{
	return os_lwm2m_create_device_object(manufacturer, model, serial,
		fw_version, hw_version, sw_version, device_type, power_source,
		power_volt, power_current, battery_level, memory_total,
		memory_free, time_zone, utc_offset, binding);
}

artik_lwm2m_object *create_firmware_object(bool supported, char *pkg_name,
					char *pkg_version)
{
	return os_lwm2m_create_firmware_object(supported, pkg_name,
						pkg_version);
}

artik_lwm2m_object *create_connectivity_monitoring_object(int netbearer,
		int avlnetbearer, int signalstrength, int linkquality,
		int lenip, const char **ipaddr, int lenroute,
		const char **routeaddr, int linkutilization, const char *apn,
		int cellid, int smnc, int smcc)
{
	return os_lwm2m_create_connectivity_monitoring_object(
		netbearer, avlnetbearer, signalstrength, linkquality,
		lenip, ipaddr, lenroute, routeaddr, linkutilization, apn,
		cellid, smnc, smcc);
}


void free_object(artik_lwm2m_object *object)
{
	os_lwm2m_free_object(object);
}

artik_error serialize_tlv_int(int *data, int size, unsigned char **buffer,
				int *lenbuffer)
{
	return os_serialize_tlv_int(data, size, buffer, lenbuffer);
}

artik_error serialize_tlv_string(char **data, int size, unsigned char **buffer,
				int *lenbuffer)
{
	return os_serialize_tlv_string(data, size, buffer, lenbuffer);
}
