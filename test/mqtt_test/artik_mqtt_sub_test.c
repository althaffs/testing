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
#include <artik_platform.h>
#include <artik_loop.h>
#include <artik_mqtt.h>
#include <artik_log.h>
#include <stdio.h>
#define BROKER_PORT 1883

artik_mqtt_module *mqtt;
artik_loop_module *loop;

void on_connect_subscribe(artik_mqtt_config *client_config, void *data_user,
								int result)
{
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)
							client_config->handle;

	artik_mqtt_msg *msg;
	int rc;

	if (result == S_OK && client_data) {
		msg = (artik_mqtt_msg *) data_user;
		rc = mqtt->subscribe(client_data, msg->qos, msg->topic);
		if (rc == 0)
			log_dbg("subscribe success");
	}
}

void on_message_disconnect(artik_mqtt_config *client_config, void *data_user,
							artik_mqtt_msg *msg)
{
	artik_mqtt_handle *client_data;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *) data_user;

	if (msg && client_config) {
		log_dbg("topic %s, content %s\n", msg->topic,
							(char *)msg->payload);
		client_data = (artik_mqtt_handle *) client_config->handle;
		user_mqtt->disconnect(client_data);
	}
}

void on_disconnect(artik_mqtt_config *client_config, void *data_user,
								int result)
{
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)
							client_config->handle;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *) data_user;

	if (result == S_OK) {
		log_dbg("disconnected\n");
		if (client_data) {
			user_mqtt->destroy_client(client_data);
			client_data = NULL;
			loop->quit();
		}
	}
}

int main(int argc, char *argv[])
{
	char *sub_topic;
	char *host;

	artik_mqtt_config config;
	artik_mqtt_msg subscribe_msg;
	artik_mqtt_handle client;

	if (argc < 3) {
		printf("Usage: %s <hostname or ip> <topic>\n", argv[0]);
		return 0;
	}

	host = argv[1];
	sub_topic = argv[2];

	memset(&config, 0, sizeof(artik_mqtt_config));
	config.client_id = "sub_client";
	config.block = true;
	memset(&subscribe_msg, 0, sizeof(artik_mqtt_msg));
	subscribe_msg.topic = sub_topic;
	subscribe_msg.qos = 0;
	mqtt = (artik_mqtt_module *) artik_request_api_module("mqtt");
	mqtt->create_client(&client, &config);

	mqtt->set_connect(client, on_connect_subscribe, &subscribe_msg);
	mqtt->set_disconnect(client, on_disconnect, mqtt);
	mqtt->set_message(client, on_message_disconnect, mqtt);

	mqtt->connect(client, host, BROKER_PORT);
	loop = artik_request_api_module("loop");
	loop->run();
	artik_release_api_module(mqtt);
	artik_release_api_module(loop);
	return 0;
}
