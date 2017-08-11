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

#ifndef __OS_MQTT_H__
#define __OS_MQTT_H__

#include <artik_mqtt.h>
#include <artik_error.h>

artik_error os_mqtt_create_client(artik_mqtt_handle * client,
		artik_mqtt_config * config);

artik_error os_mqtt_destroy_client(artik_mqtt_handle client);

artik_error os_mqtt_set_willmsg(artik_mqtt_config *config,
		const char *willtopic, const char *willmsg, int qos,
		bool retain);

artik_error os_mqtt_clear_willmsg(artik_mqtt_handle client);

artik_error os_mqtt_free_willmsg(artik_mqtt_config *config);

artik_error os_mqtt_set_connect(artik_mqtt_handle client, connect_callback cb,
		void *user_connect_data);

artik_error os_mqtt_set_disconnect(artik_mqtt_handle client,
		disconnect_callback cb, void *user_disconnect_data);

artik_error os_mqtt_set_subscribe(artik_mqtt_handle client,
		subscribe_callback cb, void *user_subscribe_data);

artik_error os_mqtt_set_unsubscribe(artik_mqtt_handle client,
		unsubscribe_callback cb, void *user_unsubscribe_data);

artik_error os_mqtt_set_publish(artik_mqtt_handle client, publish_callback cb,
		void *user_publish_data);

artik_error os_mqtt_set_message(artik_mqtt_handle client, message_callback cb,
		void *user_message_data);

artik_error os_mqtt_connect(artik_mqtt_handle client,
		const char *host, int port);

artik_error os_mqtt_disconnect(artik_mqtt_handle client);

artik_error os_mqtt_subscribe(artik_mqtt_handle client, int qos,
		const char *msgtopic);

artik_error os_mqtt_unsubscribe(artik_mqtt_handle client, const char *msgtopic);

artik_error os_mqtt_publish(artik_mqtt_handle client, int qos, bool retain,
		const char *msg_topic, int payload_len,
		const char *msg_content);

#endif  /* __OS_MQTT_H__ */
