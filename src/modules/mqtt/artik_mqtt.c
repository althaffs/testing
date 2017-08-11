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
#include <artik_mqtt.h>
#include "os_mqtt.h"

static artik_error create_client(artik_mqtt_handle * client,
				artik_mqtt_config * config);
static artik_error destroy_client(artik_mqtt_handle client);
static artik_error set_willmsg(artik_mqtt_config *config,
			       const char *willtopic, const char *willmsg,
			       int qos, bool retain);
static artik_error clear_willmsg(artik_mqtt_handle client);
static artik_error free_willmsg(artik_mqtt_config *config);
static artik_error set_connect(artik_mqtt_handle client, connect_callback cb,
				void *data);
static artik_error set_disconnect(artik_mqtt_handle client,
				disconnect_callback cb, void *data);
static artik_error set_subscribe(artik_mqtt_handle client,
				subscribe_callback cb, void *data);
static artik_error set_unsubscribe(artik_mqtt_handle client,
				unsubscribe_callback cb, void *data);
static artik_error set_publish(artik_mqtt_handle client, publish_callback cb,
				void *data);
static artik_error set_message(artik_mqtt_handle client, message_callback cb,
				void *data);
static artik_error connect(artik_mqtt_handle client, const char *host,
				int port);
static artik_error subscribe(artik_mqtt_handle client, int qos,
				const char *msgtopic);
static artik_error unsubscribe(artik_mqtt_handle client, const char *msgtopic);
static artik_error disconnect(artik_mqtt_handle client);
static artik_error publish(artik_mqtt_handle client, int qos, bool retain,
			   const char *msg_topic, int payload_len,
			   const char *msg_content);

const artik_mqtt_module mqtt_module = {
		create_client,
		destroy_client,
		set_willmsg,
		free_willmsg,
		clear_willmsg,
		set_connect,
		set_disconnect,
		set_subscribe,
		set_unsubscribe,
		set_publish,
		set_message,
		connect,
		disconnect,
		subscribe,
		unsubscribe,
		publish
};

static artik_error create_client(artik_mqtt_handle *client,
		artik_mqtt_config *config)
{
	return os_mqtt_create_client(client, config);
}

static artik_error destroy_client(artik_mqtt_handle client)
{
	return os_mqtt_destroy_client(client);
}

static artik_error set_willmsg(artik_mqtt_config *config,
		const char *willtopic, const char *willmsg, int qos,
		bool retain)
{
	return os_mqtt_set_willmsg(config, willtopic, willmsg, qos, retain);
}

static artik_error clear_willmsg(artik_mqtt_handle client)
{
	return os_mqtt_clear_willmsg(client);
}

static artik_error free_willmsg(artik_mqtt_config *config)
{
	return os_mqtt_free_willmsg(config);
}

static artik_error set_connect(artik_mqtt_handle client, connect_callback cb,
				void *data)
{
	return os_mqtt_set_connect(client, cb, data);
}

static artik_error set_disconnect(artik_mqtt_handle client,
				disconnect_callback cb, void *data)
{
	return os_mqtt_set_disconnect(client, cb, data);
}

static artik_error set_subscribe(artik_mqtt_handle client,
				subscribe_callback cb, void *data)
{
	return os_mqtt_set_subscribe(client, cb, data);
}

static artik_error set_unsubscribe(artik_mqtt_handle client,
				unsubscribe_callback cb, void *data)
{
	return os_mqtt_set_unsubscribe(client, cb, data);
}

static artik_error set_publish(artik_mqtt_handle client,
				publish_callback cb, void *data)
{
	return os_mqtt_set_publish(client, cb, data);
}

static artik_error set_message(artik_mqtt_handle client, message_callback cb,
				void *data)
{
	return os_mqtt_set_message(client, cb, data);
}

static artik_error connect(artik_mqtt_handle client, const char *host,
		int port)
{
	return os_mqtt_connect(client, host, port);
}

static artik_error disconnect(artik_mqtt_handle client)
{
	return os_mqtt_disconnect(client);
}

static artik_error subscribe(artik_mqtt_handle client, int qos,
		const char *msgtopic)
{
	return os_mqtt_subscribe(client, qos, msgtopic);
}

static artik_error unsubscribe(artik_mqtt_handle client, const char *msgtopic)
{
	return os_mqtt_unsubscribe(client, msgtopic);
}

static artik_error publish(artik_mqtt_handle client, int qos, bool retain,
		const char *msg_topic, int payload_len, const char *msg_content)
{
	return os_mqtt_publish(client, qos, retain, msg_topic, payload_len,
			msg_content);
}
