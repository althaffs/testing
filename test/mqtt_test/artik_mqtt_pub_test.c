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

#include <stdio.h>

#include <artik_module.h>
#include <artik_platform.h>
#include <artik_loop.h>
#include <artik_mqtt.h>
#include <artik_log.h>

#define BROKER_PORT 1883

artik_mqtt_module *mqtt;
artik_loop_module *loop;

void on_connect_publish(artik_mqtt_config *client_config, void *user_data,
			int result)
{
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)
							client_config->handle;
	artik_mqtt_msg *msg = (artik_mqtt_msg *)user_data;
	int rc;

	if (result == S_OK) {
		log_dbg("connected\n");
		rc = mqtt->publish(client_data, msg->qos, msg->retain,
				msg->topic,
				msg->payload_len, (char *) msg->payload);

		if (rc == 0)
			log_dbg("publish success\n");
	}
}

void on_publish_disconnect(artik_mqtt_config *client_config, void *user_data,
			int mid)
{
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)
							client_config->handle;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *) user_data;

	if (client_data) {
		log_dbg("published and disconnect\n");
		user_mqtt->disconnect(client_data);
	}
}

void on_disconnect(artik_mqtt_config *client_config, void *user_data,
			int result)
{
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)
							client_config->handle;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *) user_data;

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
	char *pub_topic;
	char *pub_content;
	char *host;

	artik_mqtt_config config;
	artik_mqtt_msg publish_msg;
	artik_mqtt_handle client;

	if (argc < 4) {
		printf("Usage: %s <hostname or ip> <topic> <message>\n",
								argv[0]);
		return 0;
	}

	if (!artik_is_module_available(ARTIK_MODULE_MQTT)) {
		fprintf(stdout,
			"TEST: MQTT module is not available,"\
			" skipping test...\n");
		return -1;
	}

	if (!artik_is_module_available(ARTIK_MODULE_LOOP)) {
		fprintf(stdout,
			"TEST: LOOP module is not available,"\
			" skipping test...\n");
		return -1;
	}

	host = argv[1];
	pub_topic = argv[2];
	pub_content = argv[3];

	memset(&config, 0, sizeof(artik_mqtt_config));
	config.client_id = "pub_client";
	config.block = true;

	memset(&publish_msg, 0, sizeof(artik_mqtt_msg));
	publish_msg.topic = pub_topic;
	publish_msg.payload = pub_content;
	publish_msg.payload_len = strlen(pub_content);
	publish_msg.qos = 0;
	publish_msg.retain = false;

	mqtt = (artik_mqtt_module *) artik_request_api_module("mqtt");
	mqtt->create_client(&client, &config);

	mqtt->set_connect(client, on_connect_publish, &publish_msg);
	mqtt->set_disconnect(client, on_disconnect, mqtt);
	mqtt->set_publish(client, on_publish_disconnect, mqtt);


	mqtt->connect(client, host, BROKER_PORT);
	loop = artik_request_api_module("loop");
	loop->run();
	artik_release_api_module(mqtt);
	artik_release_api_module(loop);
	return 0;
}
