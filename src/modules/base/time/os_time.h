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

#ifndef	__OS_TIME_H__
#define	__OS_TIME_H__

#include "artik_error.h"
#include "artik_time.h"

void	os_loop_run(void);
void	os_loop_quit(void);
artik_error  os_time_set_time(artik_time date, artik_time_zone gmt);
artik_error  os_time_get_time(artik_time_zone gmt, artik_time *date);
artik_error os_time_get_time_str(char *date_str, int size, char *const format,
				artik_time_zone gmt);
artik_msecond os_time_get_tick(void);
artik_error os_time_create_alarm_second(artik_time_zone gmt,
					artik_alarm_handle *handle,
					alarm_callback func, void *user_data,
					artik_msecond second);
artik_error os_time_create_alarm_date(artik_time_zone gmt,
				artik_alarm_handle *handle,
				alarm_callback func, void *user_data,
				artik_time date);
artik_error os_time_delete_alarm(artik_alarm_handle handle);
artik_error os_time_get_delay_alarm(artik_alarm_handle handle,
				    artik_msecond *msecond);
artik_error os_time_sync_ntp(const char *hostname);

#endif  /* __OS_TIME_H__ */
