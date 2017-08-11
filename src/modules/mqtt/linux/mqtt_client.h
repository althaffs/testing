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

#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include "artik_mqtt.h"

enum MQTT_ERROR_CODE {
	MQTT_ERROR_SUCCESS = 0,
	MQTT_ERROR_PARAM,
	MQTT_ERROR_NOMEM,
	MQTT_ERROR_LIB
};

artik_mqtt_handle mqtt_create_client(artik_mqtt_config *config);
void mqtt_destroy_client(artik_mqtt_handle client);

int mqtt_set_willmsg(artik_mqtt_config *config, const char *willtopic,
			const char *willmsg, int qos, bool retain);
int mqtt_free_willmsg(artik_mqtt_config *config);
int mqtt_clear_willmsg(artik_mqtt_handle client);

int mqtt_set_connect(artik_mqtt_handle client, connect_callback cb,
			void *user_connect_data);

int mqtt_set_disconnect(artik_mqtt_handle client, disconnect_callback cb,
			void *user_disconnect_data);

int mqtt_set_subscribe(artik_mqtt_handle client, subscribe_callback cb,
			void *user_subscribe_data);

int mqtt_set_unsubscribe(artik_mqtt_handle client, unsubscribe_callback cb,
			void *user_unsubscribe_data);

int mqtt_set_publish(artik_mqtt_handle client, publish_callback cb,
			void *user_publish_data);

int mqtt_set_message(artik_mqtt_handle client, message_callback cb,
			void *user_message_data);

int mqtt_connect(artik_mqtt_handle client, const char *host, int port);
void mqtt_disconnect(artik_mqtt_handle client);
int mqtt_subscribe(artik_mqtt_handle client, int qos, const char *msgtopic);
int mqtt_unsubscribe(artik_mqtt_handle client, const char *msgtopic);
int mqtt_publish(artik_mqtt_handle client, int qos, bool retain,
					  const char *msg_topic,
					  int payload_len,
					  const char *msg_content);

#endif
