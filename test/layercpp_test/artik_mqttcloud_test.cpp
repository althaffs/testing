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

#include <artik_loop.hh>
#include <artik_mqtt.hh>
#include <artik_error.hh>

#include <iostream>
#include <string>

static int MAX_UUID_LEN = 128;

static artik::Loop loop;

#define DEFAULT_DEVICE_ID "< fill up with AKC device ID >"
#define DEFAULT_AKC_TOKEN "< fill up with AKC token >"
#define DEFAULT_PUB_MSG   "< fill up with message to send >"

struct connect_params {
  std::string device_id;
  std::string pub_msg;
};

static void on_connect_subscribe(artik_mqtt_config *client_config,
    void *user_data, artik_error result) {
  artik::Mqtt *client_mqtt = (artik::Mqtt *) client_config->handle;
  struct connect_params *params = reinterpret_cast<struct connect_params*>(
      user_data);
  std::string pub_topic(MAX_UUID_LEN + 128, 0);
  std::string sub_topic(MAX_UUID_LEN + 128, 0);
  artik_error ret;

  if (result == S_OK && client_mqtt) {
    /* Subscribe to receive actions */
    sub_topic = "/v1.1/actions/" + params->device_id;
    ret = client_mqtt->subscribe(0, sub_topic.c_str());
    if (ret == S_OK)
      std::cout << "subscribe success" << std::endl;
    else
      std::cerr << "subscribe err: " << error_msg(ret) << std::endl;

    /* Publish message */
    pub_topic = "/v1.1/messages/" + params->device_id;
    ret = client_mqtt->publish(0, false, pub_topic.data(),
        params->pub_msg.length(), params->pub_msg.data());
    if (ret == S_OK)
      std::cout << "publish success" << std::endl;
    else
      std::cerr << "publish err: " << error_msg(ret) << std::endl;
  }
}

static void on_message_disconnect(artik_mqtt_config *client_config,
    void *user_data, artik_mqtt_msg *msg) {
  artik::Mqtt *client_mqtt = (artik::Mqtt *)client_config->handle;

  if (msg && client_mqtt) {
    std::cout << "topic " << msg->topic << " , content "
        << reinterpret_cast<char*>(msg->payload) << std::endl;
    client_mqtt->disconnect();
  }
}

static void on_disconnect(artik_mqtt_config *client_config, void *user_data,
    artik_error result) {
  artik::Mqtt *client_mqtt = (artik::Mqtt *) client_config->handle;

  if (result == S_OK) {
    std::cout << "disconnected" << std::endl;
    if (client_mqtt) {
      delete client_mqtt;
      loop.quit();
    }
  }
}

static void on_publish(artik_mqtt_config *client_config, void *user_data,
    int result) {
  std::cout << "message published (" << result << ")" << std::endl;
}

int main(int ac, char **av) {
  std::string host = "api.artik.cloud";
  int broker_port = 8883;
  std::string sub_topic(MAX_UUID_LEN + 128, 0);
  std::string token(DEFAULT_AKC_TOKEN);
  std::string device_id(DEFAULT_DEVICE_ID);
  std::string pub_msg(DEFAULT_PUB_MSG);
  artik::Mqtt *mqtt;
  artik_mqtt_config config;
  artik_ssl_config ssl;
  struct connect_params params;
  std::string akc_root_ca =
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

  if (ac > 2) {
    device_id = av[1];
    token = av[2];
    if (ac > 3)
      pub_msg = av[3];
  }
  std::cout << "Using ID: '" << device_id << "'" << std::endl;
  std::cout << "Using token: '" << token << "'" << std::endl;
  std::cout << "Message: '" << pub_msg << "'" << std::endl;

  memset(&config, 0, sizeof(artik_mqtt_config));
  config.client_id = "sub_client";
  config.block = true;
  config.user_name = device_id.data();
  config.pwd = token.c_str();

  /* TLS configuration  */
  memset(&ssl, 0, sizeof(artik_ssl_config));
  ssl.verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
  ssl.ca_cert.data = const_cast<char*>(akc_root_ca.c_str());
  ssl.ca_cert.len = akc_root_ca.length();
  config.tls = &ssl;

  /* Connect to server */
  try {
    mqtt = new artik::Mqtt(config);

    params.device_id = device_id;
    params.pub_msg = pub_msg;

    mqtt->set_connect(on_connect_subscribe, &params);
    mqtt->set_disconnect(on_disconnect, NULL);
    mqtt->set_publish(on_publish, NULL);
    mqtt->set_message(on_message_disconnect, NULL);

    mqtt->connect(host.c_str(), broker_port);

    loop.run();
  } catch (artik::ArtikException &e) {
    std::cerr << "[Exception] : " << e.what() << std::endl;
  }

  return 0;
}
