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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_cloud.hh>

#define ARTIK_WEBSOCKET_STR_MAX 1024
#define MAX_PARAM_LEN     128
#define MAX_MESSAGE_LEN   256

#define TEST_WRITE_LIMIT  (5)
#define TEST_WRITE_PERIODIC_MS  (1*1000)

static char device_id[MAX_PARAM_LEN];
static char access_token[MAX_PARAM_LEN];
char test_message[MAX_MESSAGE_LEN] = "{\"state\": true}";

static void websocket_connection_callback(void *user_data, void *result) {
  intptr_t connected = reinterpret_cast<intptr_t>(result);

  if (connected == ARTIK_WEBSOCKET_CONNECTED) {
    fprintf(stdout, "Websocket connected\n");
  } else if (connected == ARTIK_WEBSOCKET_CLOSED) {
    fprintf(stdout, "Websocket closed\n");

    artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(
        artik_request_api_module("loop"));
    loop->quit();
    artik_release_api_module(loop);
  } else {
    fprintf(stderr, "TEST failed, handshake error\n");

    artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(
        artik_request_api_module("loop"));
    loop->quit();
    artik_release_api_module(loop);
  }
}

void websocket_receive_callback(void *user_data, void *result) {
  char *buffer = reinterpret_cast<char*>(result);
  if (buffer == NULL) {
    fprintf(stdout, "receive failed\n");
    return;
  }
  printf("Received: %s\n", buffer);
  free(result);
}

static void on_timeout_callback(void *user_data) {
  artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(user_data);

  fprintf(stdout, "TEST: %s stop scanning, exiting loop\n", __func__);

  loop->quit();
}

static int count = 0;

static int on_write_periodic_callback(void *user_data) {
  artik::Cloud *cloud = reinterpret_cast<artik::Cloud*>(user_data);

  fprintf(stdout, "Writing: %s\n", test_message);
  cloud->websocket_send_message(test_message);

  count++;

  if (count > TEST_WRITE_LIMIT)
    return 0;

  return 1;
}

int main(int argc, char *argv[]) {
  int opt;
  artik_error ret = S_OK;
  int timeout_ms = 10*1000;
  int timeout_id = 0;
  int write_periodic_id = 0;
  artik_ssl_config ssl_config = {0};
  artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(
      artik_request_api_module("loop"));
  FILE *f;
  int32_t fsize;
  char *root_ca = NULL;

  ssl_config.use_se = false;

  while ((opt = getopt(argc, argv, "t:d:m:r:sv")) != -1) {
    switch (opt) {
    case 't':
      strncpy(access_token, optarg, MAX_PARAM_LEN);
      break;
    case 'd':
      strncpy(device_id, optarg, MAX_PARAM_LEN);
      break;
    case 'm':
      strncpy(test_message, optarg, MAX_MESSAGE_LEN);
      break;
    case 's':
      ssl_config.use_se = true;
      break;
    case 'v':
      ssl_config.verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
      break;
    case 'r':
      f = fopen(optarg, "rb");
      if (!f) {
        printf("File not found for parameter -r\n");
        return -1;
      }
      fseek(f, 0, SEEK_END);
      fsize = ftell(f);
      fseek(f, 0, SEEK_SET);
      root_ca = reinterpret_cast<char*>(malloc(fsize + 1));
      fread(root_ca, fsize, 1, f);
      fclose(f);
      break;
    default:
      printf("Usage: cloudcpp-test [-t <access token>] [-d <device id>]"
             " [-m <JSON type test message>]"
             " [-s for enabling SDR (Secure Device Registered) test]\r\n");
      printf("\t[-r <CA root file>] [-v for verifying root certificate]\r\n");
      return 0;
    }
  }

  if (root_ca) {
    ssl_config.ca_cert.data = strdup(root_ca);
    ssl_config.ca_cert.len = strlen(root_ca);
  }

  artik::Cloud* cloud = new artik::Cloud(access_token);

  ret = cloud->websocket_open_stream(access_token, device_id, &ssl_config);
  if (ret != S_OK) {
    fprintf(stderr, "websocket_open_stream failed\n");
    goto exit;
  }

  ret = cloud->websocket_set_connection_callback(websocket_connection_callback,
      cloud);
  if (ret != S_OK) {
    cloud->websocket_close_stream();
    fprintf(stderr, "websocket_set_connection_callback failed\n");
    goto exit;
  }

  ret = cloud->websocket_set_receive_callback(websocket_receive_callback,
      cloud);
  if (ret != S_OK) {
    cloud->websocket_close_stream();
    fprintf(stderr, "websocket_set_receive_callback failed\n");
    goto exit;
  }

  ret = loop->add_timeout_callback(&timeout_id, timeout_ms, on_timeout_callback,
      reinterpret_cast<void*>(loop));

  ret = loop->add_periodic_callback(&write_periodic_id, TEST_WRITE_PERIODIC_MS,
      on_write_periodic_callback, cloud);
  if (ret != S_OK) {
    fprintf(stderr, "TEST failed, could not add periodic callback (%d)\n", ret);
    goto exit;
  }

  loop->run();

  cloud->websocket_close_stream();

exit:
  printf("TEST FINISHED: CLOUD_CPP_TEST\n");

  return (ret == S_OK) ? 0 : -1;
}
