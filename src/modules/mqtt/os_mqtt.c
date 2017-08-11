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

#include "linux/mqtt_client.h"
#include "os_mqtt.h"

artik_error os_mqtt_create_client(artik_mqtt_handle *client, artik_mqtt_config
									*config)
{
	if (!config)
		return E_BAD_ARGS;

	*client = mqtt_create_client(config);
	if (!(*client))
		return E_NO_MEM;

	config->handle = *client;
	return S_OK;
}

artik_error os_mqtt_destroy_client(artik_mqtt_handle client)
{
	if (!client)
		return E_BAD_ARGS;

	mqtt_destroy_client(client);
	return S_OK;
}

artik_error os_mqtt_set_willmsg(artik_mqtt_config *config,
		const char *willtopic, const char *willmsg, int qos,
		bool retain)
{
	if (!config || !willtopic || !willmsg)
		return E_BAD_ARGS;

	if (mqtt_set_willmsg(config, willtopic, willmsg, qos, retain)
							!= MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;

	return S_OK;
}

artik_error os_mqtt_clear_willmsg(artik_mqtt_handle client)
{
	if (!client)
		return E_BAD_ARGS;

	if (mqtt_clear_willmsg(client) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;

	return S_OK;
}

artik_error os_mqtt_free_willmsg(artik_mqtt_config *config)
{
	if (!config)
		return E_BAD_ARGS;

	if (mqtt_free_willmsg(config) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;

	return S_OK;
}

artik_error os_mqtt_set_connect(artik_mqtt_handle client, connect_callback cb,
				void *data)
{
	if (!client)
		return E_BAD_ARGS;
	if (mqtt_set_connect(client, cb, data) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;
	return S_OK;
}

artik_error os_mqtt_set_disconnect(artik_mqtt_handle client,
					disconnect_callback cb, void *data)
{
	if (!client)
		return E_BAD_ARGS;
	if (mqtt_set_disconnect(client, cb, data) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;
	return S_OK;
}

artik_error os_mqtt_set_subscribe(artik_mqtt_handle client,
					subscribe_callback cb, void *data)
{
	if (!client)
		return E_BAD_ARGS;
	if (mqtt_set_subscribe(client, cb, data) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;
	return S_OK;
}

artik_error os_mqtt_set_unsubscribe(artik_mqtt_handle client,
					unsubscribe_callback cb, void *data)
{
	if (!client)
		return E_BAD_ARGS;
	if (mqtt_set_unsubscribe(client, cb, data) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;
	return S_OK;
}

artik_error os_mqtt_set_publish(artik_mqtt_handle client,
						publish_callback cb, void *data)
{
	if (!client)
		return E_BAD_ARGS;
	if (mqtt_set_publish(client, cb, data) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;
	return S_OK;
}

artik_error os_mqtt_set_message(artik_mqtt_handle client,
						message_callback cb, void *data)
{
	if (!client)
		return E_BAD_ARGS;
	if (mqtt_set_message(client, cb, data) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;
	return S_OK;
}

artik_error os_mqtt_connect(artik_mqtt_handle client, const char *host,
								int port)
{
	if (!client)
		return E_BAD_ARGS;

	if (mqtt_connect(client, host, port) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;

	return S_OK;
}

artik_error os_mqtt_disconnect(artik_mqtt_handle client)
{
	if (!client)
		return E_BAD_ARGS;

	mqtt_disconnect(client);

	return S_OK;
}

artik_error os_mqtt_subscribe(artik_mqtt_handle client, int qos,
							const char *msgtopic)
{
	if (!client)
		return E_BAD_ARGS;

	if (mqtt_subscribe(client, qos, msgtopic) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;

	return S_OK;
}

artik_error os_mqtt_unsubscribe(artik_mqtt_handle client, const char *msgtopic)
{
	if (!client)
		return E_BAD_ARGS;

	if (mqtt_unsubscribe(client, msgtopic) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;

	return S_OK;
}

artik_error os_mqtt_publish(artik_mqtt_handle client, int qos, bool retain,
		const char *msg_topic, int payload_len, const char *msg_content)
{
	if (!client)
		return E_BAD_ARGS;

	if (mqtt_publish(client, qos, retain, msg_topic,
			payload_len, msg_content) != MQTT_ERROR_SUCCESS)
		return E_MQTT_ERROR;

	return S_OK;
}
