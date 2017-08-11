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

#ifndef __ARTIK_BT_FTP_H
#define __ARTIK_BT_FTP_H

#include <stdbool.h>
#include <artik_bluetooth.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define OBEXD_CHECK_CMD     "ps -ax | grep obexd |grep -v grep"
#define OBEXD_CMD           "obexd"
#define OBEXD_START_CMD     "/usr/libexec/bluetooth/obexd"
#define OBEXD_DESTROY_CMD   "pkill -9 obexd"

#define OBEXD_WATI_TIME 2
#define OBEXD_BUF_SIZE  100

artik_error bt_ftp_create_session(char *dest_addr);

artik_error bt_ftp_remove_session(void);

artik_error bt_ftp_change_folder(char *folder);

artik_error bt_ftp_create_folder(char *folder);

artik_error bt_ftp_delete_file(char *file);

artik_error bt_ftp_list_folder(artik_bt_ftp_file **file_list);

artik_error bt_ftp_get_file(char *target_file, char *source_file);

artik_error bt_ftp_put_file(char *source_file, char *target_file);

artik_error bt_ftp_cancel_transfer(void);

artik_error bt_ftp_resume_transfer(void);

artik_error bt_ftp_suspend_transfer(void);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_FTP_H */
