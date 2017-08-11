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

#ifndef __ARTIK_BT_GATT_H
#define __ARTIK_BT_GATT_H

#ifdef __cplusplus
extern "C" {
#endif

artik_error bt_gatt_add_service(artik_bt_gatt_service svc, int *id);
artik_error bt_gatt_add_characteristic(int svc_id, artik_bt_gatt_chr chr,
		int *id);
artik_error bt_gatt_set_chr_on_read_request(int svc_id, int char_id,
		artik_bt_gatt_req_read callback, void *user_data);
artik_error bt_gatt_set_chr_on_write_request(int svc_id, int char_id,
		artik_bt_gatt_req_write callback, void *user_data);
artik_error bt_gatt_set_chr_on_notify_request(int svc_id, int char_id,
		artik_bt_gatt_req_notify callback, void *user_data);
artik_error bt_gatt_add_descriptor(int service_id, int char_id,
		artik_bt_gatt_desc desc, int *id);
artik_error bt_gatt_set_desc_on_read_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_read callback, void *user_data);
artik_error bt_gatt_set_desc_on_write_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_write callback, void *user_data);
artik_error bt_gatt_register_service(int id);
artik_error bt_gatt_unregister_service(int id);
artik_error bt_gatt_get_service_list(const char *addr,
		artik_bt_uuid **uuid_list, int *len);
artik_error bt_gatt_get_chracteristic_list(const char *addr,
		const char *srv_uuid, artik_bt_uuid **uuid_list, int *len);
artik_error bt_gatt_get_descriptor_list(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_uuid **uuid_list, int *len);
artik_error bt_gatt_char_read_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		unsigned char **byte, int *byte_len);
artik_error bt_gatt_char_write_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		const unsigned char byte[], int byte_len);
artik_error bt_gatt_desc_read_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		const char *desc_uuid, unsigned char **byte, int *byte_len);
artik_error bt_gatt_desc_write_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		const char *desc_uuid, const unsigned char byte[], int byte_len);
artik_error bt_gatt_start_notify(const char *addr, const char *srv_uuid, const char *char_uuid);
artik_error bt_gatt_stop_notify(const char *addr, const char *srv_uuid, const char *char_uuid);
artik_error bt_gatt_get_char_properties(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_gatt_char_properties *properties);
/* Bluez Not supported yet */
artik_error bt_gatt_get_desc_properties(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, artik_bt_gatt_desc_properties *properties);
artik_error bt_gatt_req_set_value(artik_bt_gatt_req request,
		int len, const unsigned char *value);
artik_error bt_gatt_req_set_result(artik_bt_gatt_req request,
		artik_bt_gatt_req_state_type state, const char *err_msg);
artik_error bt_gatt_notify(int svc_id, int char_id, unsigned char *byte, int len);

#ifdef __cplusplus
} $
#endif

#endif /* __ARTIK_BT_GATT_H */
