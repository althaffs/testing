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

#include	<stdlib.h>
#include	<string.h>

#include	"artik_time.h"
#include	"os_time.h"

static artik_error artik_time_set_time(artik_time date, artik_time_zone gmt);
static artik_error artik_time_get_time(artik_time_zone gmt, artik_time *date);
static artik_error artik_time_get_time_str(char *date_str, int size,
				char *const format, artik_time_zone gmt);
static artik_msecond artik_time_get_tick(void);
static artik_error artik_time_create_alarm_second(artik_time_zone gmt,
						  artik_msecond sec,
						  artik_alarm_handle *handle,
						  alarm_callback func,
						  void *user_data);
static artik_error artik_time_create_alarm_date(artik_time_zone gmt,
						artik_time date,
						artik_alarm_handle *handle,
						alarm_callback func,
						void *user_data);
static artik_error artik_time_delete_alarm(artik_alarm_handle handle);
static artik_error artik_time_get_delay_alarm(artik_alarm_handle handle,
					      artik_msecond *msecond);
static artik_error artik_time_sync_ntp(const char *hostname);
static int artik_time_compare_dates(const artik_time *date1,
		const artik_time *date2);

EXPORT_API artik_time_module time_module = {
	artik_time_set_time,
	artik_time_get_time,
	artik_time_get_time_str,
	artik_time_get_tick,
	artik_time_create_alarm_second,
	artik_time_create_alarm_date,
	artik_time_delete_alarm,
	artik_time_get_delay_alarm,
	artik_time_sync_ntp,
	artik_time_compare_dates
};

static artik_error artik_time_set_time(artik_time date, artik_time_zone gmt)
{
	return os_time_set_time(date, gmt);
}

static artik_error artik_time_get_time(artik_time_zone gmt, artik_time *date)
{
	return os_time_get_time(gmt, date);
}

static artik_error artik_time_get_time_str(char *date_str, int size,
					char *const format, artik_time_zone gmt)
{
	return os_time_get_time_str(date_str, size, format, gmt);
}

static artik_msecond artik_time_get_tick(void)
{
	return os_time_get_tick();
}

artik_error artik_time_create_alarm_second(artik_time_zone gmt,
					   artik_msecond sec,
					   artik_alarm_handle *handle,
					   alarm_callback func,
					   void *user_data)
{
	if (!handle || *handle || sec == 0)
		return E_BAD_ARGS;
	return os_time_create_alarm_second(gmt, handle, func, user_data, sec);
}

static artik_error artik_time_create_alarm_date(artik_time_zone gmt,
						artik_time date,
						artik_alarm_handle *handle,
						alarm_callback func,
						void *user_data)
{
	if (!handle || *handle)
		return E_BAD_ARGS;
	return os_time_create_alarm_date(gmt, handle, func, user_data, date);
}

static artik_error artik_time_delete_alarm(artik_alarm_handle handle)
{
	if (!handle)
		return E_BAD_ARGS;
	return os_time_delete_alarm(handle);
}

static artik_error artik_time_get_delay_alarm(artik_alarm_handle handle,
						artik_msecond *msecond)
{
	if (!handle || !msecond)
		return E_BAD_ARGS;
	return os_time_get_delay_alarm(handle, msecond);
}

static artik_error artik_time_sync_ntp(const char *hostname)
{
	return os_time_sync_ntp(hostname);
}

int artik_time_compare_dates(const artik_time *date1, const artik_time *date2)
{
	if (!date1 || !date2)
		return -2;

	if (date1->year > date2->year)
		return 1;
	else if (date1->year < date2->year)
		return -1;

	if (date1->month > date2->month)
		return 1;
	else if (date1->month < date2->month)
		return -1;

	if (date1->day > date2->day)
		return 1;
	else if (date1->day < date2->day)
		return -1;

	if (date1->hour > date2->hour)
		return 1;
	else if (date1->hour < date2->hour)
		return -1;

	if (date1->minute > date2->minute)
		return 1;
	else if (date1->minute < date2->minute)
		return -1;

	if (date1->second > date2->second)
		return 1;
	else if (date1->second < date2->second)
		return -1;

	if (date1->msecond > date2->msecond)
		return 1;
	else if (date1->msecond < date2->msecond)
		return -1;

	return 0;
}

