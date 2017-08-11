/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WPA_CLI_H
#define WPA_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

struct wpa_ctrl *get_ctrl(void);
void set_active_scan(const int n);
void set_ctrl_ifname(const char *ifname);

void set_scan_result_callback(
			wifi_scan_result_callback callback,
			void *user_data);
void set_connect_callback(
			wifi_connect_callback callback,
			void *user_data);

int wpa_cli_open_connection(const char *ifname, int attach);
void wpa_cli_close_connection(void);
int str_starts(const char *src, const char *match);
void wpa_cli_terminate(int sig, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* WPA_CLI_H */
