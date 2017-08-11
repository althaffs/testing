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
#include <stdio.h>

#define MAX_MSG_LEN	512
#define MAX_UUID_LEN	128

static artik_mqtt_module *mqtt = NULL;
static artik_loop_module *loop = NULL;

static char device_id[MAX_UUID_LEN] = "< fill up with AKC device ID >";
static char token[MAX_UUID_LEN] = "< fill up with AKC token >";
static char pub_msg[MAX_MSG_LEN] = "< fill up with message to send >";

static const char *akc_root_ca =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\r\n"
	"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\r\n"
	"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\r\n"
	"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\r\n"
	"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\r\n"
	"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\r\n"
	"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\r\n"
	"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\r\n"
	"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\r\n"
	"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\r\n"
	"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\r\n"
	"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\r\n"
	"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\r\n"
	"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\r\n"
	"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\r\n"
	"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\r\n"
	"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\r\n"
	"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\r\n"
	"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\r\n"
	"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\r\n"
	"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\r\n"
	"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\r\n"
	"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\r\n"
	"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\r\n"
	"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\r\n"
	"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\r\n"
	"-----END CERTIFICATE-----\n";

void on_connect_subscribe(artik_mqtt_config *client_config,
					void *user_data, artik_error result)
{
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)
							client_config->handle;
	artik_mqtt_msg *msg = (artik_mqtt_msg *)user_data;
	char pub_topic[MAX_UUID_LEN + 128];
	artik_error ret;

	if (result == S_OK && client_data) {
		/* Subscribe to receive actions */
		ret = mqtt->subscribe(client_data, msg->qos, msg->topic);
		if (ret == S_OK)
			fprintf(stdout, "subscribe success\n");
		else
			fprintf(stderr, "subscribe err: %s\n", error_msg(ret));

		/* Publish message */
		snprintf(pub_topic, sizeof(pub_topic), "/v1.1/messages/%s",
								device_id);
		ret = mqtt->publish(client_data, 0, false, pub_topic,
						strlen(pub_msg), pub_msg);
		if (ret == S_OK)
			fprintf(stdout, "publish success\n");
		else
			fprintf(stderr, "publish err: %s\n", error_msg(ret));
	}
}

void on_message_disconnect(artik_mqtt_config *client_config, void *user_data,
							artik_mqtt_msg *msg)
{
	artik_mqtt_handle *client_data;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *) user_data;

	if (msg && client_config) {
		fprintf(stdout, "topic %s, content %s\n", msg->topic,
							(char *)msg->payload);
		client_data = (artik_mqtt_handle *) client_config->handle;
		user_mqtt->disconnect(client_data);
	}
}

void on_disconnect(artik_mqtt_config *client_config, void *user_data,
							artik_error result)
{
	artik_mqtt_handle *client_data = (artik_mqtt_handle *)
							client_config->handle;
	artik_mqtt_module *user_mqtt = (artik_mqtt_module *) user_data;

	if (client_data) {
		user_mqtt->destroy_client(client_data);
		client_data = NULL;
		loop->quit();
	}
}

void on_publish(artik_mqtt_config *client_config, void *user_data, int result)
{
	fprintf(stdout, "message published (%d)\n", result);
}

int main(int argc, char *argv[])
{
	int broker_port = 8883;
	char sub_topic[MAX_UUID_LEN + 128];
	artik_mqtt_config config;
	artik_mqtt_msg subscribe_msg;
	artik_mqtt_handle client;
	artik_ssl_config ssl;

	/* Use parameters if provided, keep defaults otherwise */
	if (argc > 2) {
		memset(device_id, 0, MAX_UUID_LEN);
		strncpy(device_id, argv[1], MAX_UUID_LEN);
		memset(token, 0, MAX_UUID_LEN);
		strncpy(token, argv[2], MAX_UUID_LEN);

		if (argc > 3) {
			memset(pub_msg, 0, MAX_MSG_LEN);
			strncpy(pub_msg, argv[3], MAX_MSG_LEN);
		}
	}

	fprintf(stdout, "Using ID: %s\n", device_id);
	fprintf(stdout, "Using token: %s\n", token);
	fprintf(stdout, "Message: %s\n", pub_msg);

	mqtt = (artik_mqtt_module *)artik_request_api_module("mqtt");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	memset(&subscribe_msg, 0, sizeof(artik_mqtt_msg));
	snprintf(sub_topic, sizeof(sub_topic), "/v1.1/actions/%s", device_id);
	subscribe_msg.topic = sub_topic;
	subscribe_msg.qos = 0;

	memset(&config, 0, sizeof(artik_mqtt_config));
	config.client_id = "sub_client";
	config.block = true;
	config.user_name = device_id;
	config.pwd = token;

	/* TLS configuration  */
	memset(&ssl, 0, sizeof(artik_ssl_config));
	ssl.verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
	ssl.ca_cert.data = (char *)akc_root_ca;
	ssl.ca_cert.len = strlen(akc_root_ca);
	config.tls = &ssl;

	/* Connect to server */
	mqtt->create_client(&client, &config);
	mqtt->set_connect(client, on_connect_subscribe, &subscribe_msg);
	mqtt->set_disconnect(client, on_disconnect, mqtt);
	mqtt->set_publish(client, on_publish, mqtt);
	mqtt->set_message(client, on_message_disconnect, mqtt);

	mqtt->connect(client, "api.artik.cloud", broker_port);

	loop->run();

	artik_release_api_module(mqtt);
	artik_release_api_module(loop);

	return 0;
}
