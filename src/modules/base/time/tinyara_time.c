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


#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "artik_time.h"
#include "os_time.h"

#define MAX_SIZE 128

#define LEN_FORM 6
#if defined(CONFIG_LIBC_LOCALTIME) || defined(CONFIG_TIME_EXTENDED)
#  undef  LEN_FORM
#  define LEN_FORM 8
#endif

#define EPOCH_DEF 1900
#define DST_DEF 4

#define MAX(a, b) ((a > b) ? a : b)

#if defined(CONFIG_LIBC_LOCALTIME) || defined(CONFIG_TIME_EXTENDED)
#  define TM_TO_ARTIK_TM(atm, tm) { \
	TM_TO_ARTIK_TM_DFLT(atm, tm); \
	val.day_of_week = tm.tm_wday; \
	val.msecond = tm.tm_yday; \
	tm.tm_isdst; \
}
#else
#  define TM_TO_ARTIK_TM(atm, tm) { \
	TM_TO_ARTIK_TM_DFLT(atm, tm); \
}
#endif


struct time_parser_s {
	char format_link[LEN_FORM];
	char *format_mod;
};

static artik_error os_time_get_sys(struct tm *tp, artik_time_zone gmt, unsigned int *msecond)
{
	struct tm *rtime = NULL;
	struct timeval tval;

	int ret;

	ret = gettimeofday(&tval, NULL);
	if (ret == OK)
		rtime = gmtime(&tval.tv_sec);


	if (ret < 0 || rtime == NULL)
		return E_BAD_ARGS;

	if (msecond)
		*msecond = (unsigned int)(tval.tv_usec/1000);

	memcpy(tp, rtime, sizeof(struct tm));

	return S_OK;
}

artik_error os_time_set_time(artik_time date, artik_time_zone gmt)
{
	struct tm dtime = {
		.tm_sec = 0,   /* Seconds (0-61, allows for leap seconds) */
		.tm_min = 0,   /* Minutes (0-59) */
		.tm_hour = 0,  /* Hours (0-23) */
		.tm_mday = 0,  /* Day of the month (1-31) */
		.tm_mon = 0,   /* Month (0-11) */
		.tm_year = 0,  /* Years since 1900 */
#if defined(CONFIG_LIBC_LOCALTIME) || defined(CONFIG_TIME_EXTENDED)
		.tm_wday = 0,  /* Day of the week (0-6) */
		.tm_yday = 0,  /* Day of the year (0-365) */
		.tm_isdst = 0, /* Non-0 if daylight savings time is in effect */
#endif
	};
	struct timespec stime = { 0, };

	time_t    sec;

	dtime.tm_sec = date.second;
	dtime.tm_min = date.minute;
	dtime.tm_hour = date.hour/* - DST_DEF*/;

	dtime.tm_mday = date.day;
	dtime.tm_mon = date.month - 1;
	dtime.tm_year = date.year - EPOCH_DEF;
#if defined(CONFIG_LIBC_LOCALTIME) || defined(CONFIG_TIME_EXTENDED)
	dtime.tm_wday = date.day_of_week;
	/*date.msecond;*/
#endif

	sec = mktime(&dtime);
	if (sec < 0)
		return E_BAD_ARGS;

	stime.tv_sec = sec;
	stime.tv_nsec = 0;

	if (clock_settime(CLOCK_REALTIME, &stime) < 0) {
		perror(strerror(errno));
		return E_BAD_ARGS;
	}

	return S_OK;
}

artik_error os_time_get_time(artik_time_zone gmt, artik_time *date)
{
	artik_error ret;
	unsigned int msecond = 0;
	struct tm t;

	if (!date)
		return E_BAD_ARGS;

	if (gmt < ARTIK_TIME_UTC || gmt > ARTIK_TIME_GMT12)
		return E_BAD_ARGS;

	ret = os_time_get_sys(&t, gmt, &msecond);
	if (ret != S_OK)
		return ret;


	t.tm_hour = (t.tm_hour + gmt) % 24;
	t.tm_mon++;
	t.tm_year += EPOCH_DEF;

	date->second = (unsigned int)t.tm_sec;
	date->minute = (unsigned int)t.tm_min;
	date->hour = (unsigned int)t.tm_hour;
	date->day = (unsigned int)t.tm_mday;
	date->month = (unsigned int)t.tm_mon;
	date->year = (unsigned int)t.tm_year;

	date->msecond = msecond;

	return S_OK;
}

artik_error os_time_get_time_str(char *date_str, int size,
				char *const format, artik_time_zone gmt)
{
	char *fmt;
	struct tm t;
	artik_error err;

	if (!date_str)
		return E_BAD_ARGS;

	if (gmt < ARTIK_TIME_UTC || gmt > ARTIK_TIME_GMT12)
		return E_BAD_ARGS;

	fmt = (format == NULL || strlen(format) == 0) ? ARTIK_TIME_DFORMAT :
									format;

	err = os_time_get_sys(&t, gmt, NULL);
	if (err < 0)
		return err;

	if (strftime(date_str, size, fmt, &t) == 0)
		return E_BAD_ARGS;

	return S_OK;
}

artik_msecond os_time_get_tick(void)
{
	struct tm val_curr = {0, };
	unsigned int ms_current;

	time_t curr_in_sec  = 0;

	os_time_get_sys(&val_curr, ARTIK_TIME_UTC, &ms_current);

	curr_in_sec = mktime(&val_curr);

	if (curr_in_sec == -1)
		return S_OK;

	curr_in_sec = (curr_in_sec*1000L)+((time_t)ms_current / 1000L);
	return curr_in_sec;
}

artik_error os_time_create_alarm_second(artik_time_zone gmt,
					artik_alarm_handle *handle,
					alarm_callback func,
					void *user_data,
					artik_msecond second)
{
	return E_NOT_SUPPORTED;
}


artik_error os_time_create_alarm_date(artik_time_zone gmt,
					artik_alarm_handle *handle,
					alarm_callback func,
					void *user_data,
					artik_time date)
{
	return E_NOT_SUPPORTED;
}

artik_error os_time_delete_alarm(artik_alarm_handle handle)
{
	return E_NOT_SUPPORTED;
}

artik_error os_time_get_delay_alarm(artik_alarm_handle handle, artik_msecond *msecond)
{
	return E_NOT_SUPPORTED;
}

artik_error os_time_sync_ntp(const char *hostname)
{
	return E_NOT_SUPPORTED;
}
