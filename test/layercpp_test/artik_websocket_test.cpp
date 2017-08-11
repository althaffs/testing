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
#include <time.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_websocket.hh>

static const char *echo_websocket_root_ca =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\r\n"
  "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\r\n"
  "EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\r\n"
  "ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\r\n"
  "NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\r\n"
  "EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\r\n"
  "AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\r\n"
  "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\r\n"
  "E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\r\n"
  "/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\r\n"
  "DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\r\n"
  "GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\r\n"
  "tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\r\n"
  "AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\r\n"
  "FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\r\n"
  "WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\r\n"
  "9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\r\n"
  "gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\r\n"
  "2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\r\n"
  "LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\r\n"
  "4uJEvlz36hz1\r\n"
  "-----END CERTIFICATE-----\n";

static char *test_message = NULL;

static void on_timeout_callback(void *user_data) {
  artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(user_data);

  fprintf(stdout, "TEST: %s stop scanning, exiting loop\n", __func__);

  loop->quit();
}

void websocket_receive_callback(void *user_data, void *result) {
  char *buffer = reinterpret_cast<char*>(result);
  if (buffer == NULL) {
    fprintf(stdout, "Receive failed\n");
    return;
  }

  artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(
      artik_request_api_module("loop"));

  printf("Received: %s\n", buffer);
  free(result);

  loop->quit();
  artik_release_api_module(loop);
}

void websocket_connection_callback(void *user_data, void *result) {
  artik::Websocket* websocket = (artik::Websocket*)user_data;
  intptr_t ret = reinterpret_cast<intptr_t>(result);

  if (ret == ARTIK_WEBSOCKET_CLOSED) {
    fprintf(stdout, "connection close\n");
    artik_release_api_module(websocket);
    goto exit;
  }

  if (ret == ARTIK_WEBSOCKET_CONNECTED) {
    fprintf(stdout, "Writing: %s\n", test_message);
    websocket->write_stream(test_message);
  } else {
    fprintf(stderr, "TEST failed, handshake error\n");
    artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(
        artik_request_api_module("loop"));
    loop->quit();
    artik_release_api_module(loop);
  }

exit:
  return;
}

int main(int argc, char *argv[]) {
  int opt;
  bool verify = false;
  artik_error ret = S_OK;
  int timeout_ms = 10*1000;
  int timeout_id = 0;
  artik_loop_module *loop = reinterpret_cast<artik_loop_module*>(
      artik_request_api_module("loop"));
  char uri[26] = "ws://echo.websocket.org/";

  while ((opt = getopt(argc, argv, "m:vt")) != -1) {
    switch (opt) {
    case 'm':
      test_message = strndup(optarg, strlen(optarg)+1);
      break;
    case 'v':
      verify = true;
      break;
    case 't':
      snprintf(uri, sizeof(uri), "%s", "wss://echo.websocket.org/");
      break;
    default:
      printf("Usage: websocketcpp-test [-t for using TLS] [-m <message>]"
             " [-v for verifying CA certificate]\n");
      return 0;
    }
  }

  if (!test_message)
    test_message = strndup("ping", 5);

  artik_ssl_config ssl_config = { 0 };

  memset(&ssl_config, 0, sizeof(ssl_config));

  ssl_config.ca_cert.data = strdup(echo_websocket_root_ca);
  ssl_config.ca_cert.len = strlen(echo_websocket_root_ca);

  if (verify)
    ssl_config.verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
  else
    ssl_config.verify_cert = ARTIK_SSL_VERIFY_NONE;

  ssl_config.verify_cert = ARTIK_SSL_VERIFY_REQUIRED;

  artik::Websocket* websocket = new artik::Websocket(uri, &ssl_config);

  ret = websocket->request();
  if (ret != S_OK) {
    fprintf(stderr, "request failed\n");
    goto exit;
  }
  ret = websocket->open_stream();
  if (ret != S_OK) {
    fprintf(stderr, "open_stream failed\n");
    goto exit;
  }

  ret = websocket->set_connection_callback(websocket_connection_callback,
      reinterpret_cast<void*>(websocket));
  if (ret != S_OK) {
    fprintf(stderr, "failed to set connection callback\n");
    goto exit;
  }

  ret = websocket->set_receive_callback(websocket_receive_callback,
      reinterpret_cast<void*>(websocket));
  if (ret != S_OK) {
    fprintf(stderr, "TEST failed, could not open Websocket\n");
    goto exit;
  }

  ret = loop->add_timeout_callback(&timeout_id, timeout_ms, on_timeout_callback,
      reinterpret_cast<void*>(loop));

  loop->run();

exit:

  websocket->close_stream();

  printf("TEST FINISHED: WEBSOCKET_CPP_TEST\n");

  return (ret == S_OK) ? 0 : -1;
}
