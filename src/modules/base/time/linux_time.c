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
#include <stdint.h>
#include <stdio.h>
#include <linux/rtc.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <artik_module.h>
#include <artik_log.h>
#include <artik_time.h>
#include <artik_loop.h>
#include "os_time.h"

#define LEN_PACK 14
#define LEN_FORM 8
#define LEN_FDS 2

#define	POS_WDPACK 5
#define	POS_YPACK 6
#define	POS_MSPACK 10

#define EPOCH_DEF 1900
#define	MAX_EXPIRATION 1

#define	FORMAT_NULL ((uint64_t)506381209866536711LL)

#define NTP_PORT 123
#define NTP_TIMEOUT_SEC 3
#define EPOCH_BALANCE 2208988800U

#define MAX(a, b)	((a > b) ? a : b)

typedef struct {
	char format_link[LEN_FORM];
	char *format_mod;
} artik_time_parser_t;

typedef struct {
	artik_time_zone gmt;
	alarm_callback func;
	artik_msecond date_alarm;
	artik_loop_module *loop;
	int alarm_id;
} artik_time_alarm_t;

static artik_error os_time_struct_empty(void *data, int len)
{
	int *addr = data;
	int size = 0;
	int check_set = 0;

	while (size < len) {
		check_set |= *addr;
		size += sizeof(*addr);
		addr = (void *)((intptr_t)data + size);
	}

	return check_set > 0 ? S_OK : E_BAD_ARGS;
}

static artik_error os_time_get_sys(struct tm *time, artik_time_zone gmt)
{
	struct tm *rtime = NULL;
	struct timeval tval;
	int res = gettimeofday(&tval, NULL);

	if (res < 0)
		return E_BAD_ARGS;

	rtime = gmtime(&tval.tv_sec);

	if (!rtime)
		return E_BAD_ARGS;

	rtime->tm_hour = (rtime->tm_hour+gmt)%24;
	rtime->tm_year += EPOCH_DEF;

	memcpy(time, rtime, sizeof(struct tm));

	return S_OK;
}

artik_error os_time_set_time(artik_time date, artik_time_zone gmt)
{
	if (gmt < ARTIK_TIME_UTC || gmt > ARTIK_TIME_GMT12)
		return E_BAD_ARGS;

	if ((int)date.second < 0 || (int)date.second > 59)
		return E_BAD_ARGS;

	if ((int)date.minute < 0 || (int)date.minute > 59)
		return E_BAD_ARGS;

	if ((int)date.hour < 0 || (int)date.hour > 23)
		return E_BAD_ARGS;

	if ((int)date.day < 1 || (int)date.day > 31)
		return E_BAD_ARGS;

	if ((int)date.month < 1 || (int)date.month > 12)
		return E_BAD_ARGS;

	if ((int)date.year < EPOCH_DEF)
		return E_BAD_ARGS;

	if ((int)date.day_of_week < 0 || (int)date.day_of_week > 6)
		return E_BAD_ARGS;

	if ((int)date.msecond < 0)
		return E_BAD_ARGS;

	struct tm dtime;
	struct timespec stime;
	time_t sec = 0;

	memset(&dtime, 0, sizeof(dtime));
	memset(&dtime, 1, sizeof(date));
	memset(&stime, 0, sizeof(stime));
	memcpy(&dtime, &date, sizeof(date));

	dtime.tm_year -= EPOCH_DEF;
	dtime.tm_mon--;
	dtime.tm_hour -= gmt;

	if (dtime.tm_hour < 0)
		dtime.tm_hour = 23;

	sec = timegm(&dtime);

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
	if (!date)
		return E_BAD_ARGS;

	if (gmt < ARTIK_TIME_UTC || gmt > ARTIK_TIME_GMT12)
		return E_BAD_ARGS;

	struct tm *rtime = NULL;
	struct timeval tval;
	int res = gettimeofday(&tval, NULL);

	if (res < 0)
		return E_INVALID_VALUE;

	rtime = gmtime(&tval.tv_sec);

	if (!rtime)
		return E_INVALID_VALUE;

	rtime->tm_hour = (rtime->tm_hour+gmt)%24;
	rtime->tm_mon++;
	rtime->tm_year += EPOCH_DEF;

	date->second = (unsigned int)rtime->tm_sec;
	date->minute = (unsigned int)rtime->tm_min;
	date->hour = (unsigned int)rtime->tm_hour;
	date->day = (unsigned int)rtime->tm_mday;
	date->month = (unsigned int)rtime->tm_mon;
	date->year = (unsigned int)rtime->tm_year;
	date->day_of_week = (unsigned int)rtime->tm_wday;
	date->msecond = (unsigned int)(tval.tv_usec/1000);

	return S_OK;
}

artik_error os_time_get_time_str(char *date_str, int size, char *const format,
				 artik_time_zone gmt)
{
	if (!date_str)
		return E_BAD_ARGS;

	if (size <= 0)
		return E_BAD_ARGS;

	if (gmt < ARTIK_TIME_UTC || gmt > ARTIK_TIME_GMT12)
		return E_BAD_ARGS;

	struct tm rtime;

	memset(date_str, 0, size);
	memset(&rtime, 0, sizeof(rtime));

	os_time_get_sys(&rtime, gmt);

	rtime.tm_year -= EPOCH_DEF;

	if (strftime(date_str, size, format ? format : ARTIK_TIME_DFORMAT,
							&rtime) == 0)
		return E_BAD_ARGS;

	return S_OK;
}

artik_msecond os_time_get_tick(void)
{
	struct tm val_curr;
	time_t curr_in_sec = 0;
	time_t ms_current = 0;

	memset(&val_curr, 0, sizeof(val_curr));

	os_time_get_sys(&val_curr, ARTIK_TIME_UTC);

	ms_current = val_curr.tm_yday;

	val_curr.tm_yday = 0;
	val_curr.tm_year -= EPOCH_DEF;
	val_curr.tm_mon++;

	curr_in_sec = mktime(&val_curr);

	if (curr_in_sec == -1)
		return S_OK;

	curr_in_sec = (curr_in_sec * 1000L) + ((time_t) ms_current / 1000L);

	return curr_in_sec;
}

artik_error os_time_create_alarm_second(artik_time_zone gmt,
					artik_alarm_handle *handle,
					alarm_callback func,
					void *user_data,
					artik_msecond second)
{
	if (gmt < ARTIK_TIME_UTC || gmt > ARTIK_TIME_GMT12)
		return E_BAD_ARGS;

	if ((int)second < 0)
		return E_BAD_ARGS;

	if (!func)
		return E_BAD_ARGS;

	artik_time_alarm_t *alarm_data = NULL;
	struct tm curr_usr;
	time_t curr_in_sec;

	memset(&curr_usr, 1, sizeof(curr_usr));
	os_time_get_sys(&curr_usr, gmt);
	curr_usr.tm_yday = 0;
	curr_usr.tm_year -= EPOCH_DEF;
	curr_in_sec = mktime(&curr_usr);
	if (curr_in_sec < 0)
		return E_BAD_ARGS;

	curr_in_sec += (time_t) second;

	*handle = malloc(sizeof(artik_time_alarm_t));

	alarm_data = *handle;
	alarm_data->loop = (artik_loop_module *)
					artik_request_api_module("loop");
	alarm_data->func = func;
	alarm_data->gmt = gmt;
	alarm_data->date_alarm = curr_in_sec;

	if (!alarm_data->loop)
		return E_BUSY;

	return alarm_data->loop->add_timeout_callback(&alarm_data->alarm_id,
						(unsigned int)second*1000,
						func, user_data);
}

artik_error os_time_create_alarm_date(artik_time_zone gmt,
				      artik_alarm_handle *handle,
				      alarm_callback func,
				      void *user_data,
				      artik_time date)
{
	if (gmt < ARTIK_TIME_UTC || gmt > ARTIK_TIME_GMT12)
		return E_BAD_ARGS;

	if (os_time_struct_empty(&date, sizeof(date)) != S_OK)
		return E_BAD_ARGS;

	if (!func)
		return E_BAD_ARGS;

	if ((int)date.second < 0 || (int)date.second > 59)
		return E_BAD_ARGS;

	if ((int)date.minute < 0 || (int)date.minute > 59)
		return E_BAD_ARGS;

	if ((int)date.hour < 0 || (int)date.hour > 23)
		return E_BAD_ARGS;

	if ((int)date.day < 1 || (int)date.day > 31)
		return E_BAD_ARGS;

	if ((int)date.month < 1 || (int)date.month > 12)
		return E_BAD_ARGS;

	if ((int)date.year < EPOCH_DEF)
		return E_BAD_ARGS;

	if ((int)date.day_of_week < 0 || (int)date.day_of_week > 6)
		return E_BAD_ARGS;

	if ((int)date.msecond < 0)
		return E_BAD_ARGS;

	time_t date_in_sec = 0, curr_in_sec = 0;
	double diff_t;
	struct tm date_usr, curr_usr;

	memset(&curr_usr, 0, sizeof(curr_usr));
	os_time_get_sys(&curr_usr, gmt);

	curr_usr.tm_yday = 0;
	curr_usr.tm_year -= EPOCH_DEF;
	curr_in_sec = mktime(&curr_usr);

	if (curr_in_sec < 0)
		return E_INVALID_VALUE;

	memset(&date_usr, 0, sizeof(date_usr));
	memcpy(&date_usr, &date, sizeof(date));

	if (date_usr.tm_year > EPOCH_DEF) {
		date_usr.tm_year -= EPOCH_DEF;
		date_usr.tm_mon--;
	}

	date_usr.tm_yday = 0;

	date_in_sec = mktime(&date_usr);

	if (date_in_sec < 0)
		return E_INVALID_VALUE;

	diff_t = difftime(date_in_sec, curr_in_sec);

	return os_time_create_alarm_second(gmt, handle, func, user_data,
		diff_t);
}

artik_error os_time_delete_alarm(artik_alarm_handle handle)
{
	artik_time_alarm_t *alarm_data = handle;

	if (alarm_data)
		free(handle);

	return S_OK;
}

artik_error os_time_get_delay_alarm(artik_alarm_handle handle,
				    artik_msecond *msecond)
{
	artik_time_alarm_t *alarm_data = handle;
	struct tm curr_usr;
	artik_msecond curr_in_sec = 0;
	int res = 0;

	memset(&curr_usr, 1, sizeof(curr_usr));

	artik_error ret = os_time_get_sys(&curr_usr, alarm_data->gmt);

	if (ret != S_OK) {
		*msecond = 0;
		return ret;
	}

	curr_usr.tm_year -= EPOCH_DEF;
	curr_usr.tm_yday = 0;
	res = mktime(&curr_usr);

	if (res == 0)
		*msecond = 0;
	else if (res < 0) {
		*msecond = 0;
		return E_INVALID_VALUE;
	}

	curr_in_sec = res;
	*msecond = alarm_data->date_alarm <= curr_in_sec ?
		0 : (alarm_data->date_alarm - curr_in_sec);

	return S_OK;
}

artik_error os_time_sync_ntp(const char *hostname)
{
	unsigned char msg[48] = { 010, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned long buf[1024];

	int sock;
	int res;

	struct protoent *proto;
	struct sockaddr_in server_addr;
	struct sockaddr saddr;
	socklen_t saddr_l;
	struct hostent *host_resolv;
	struct timeval time_struct = { NTP_TIMEOUT_SEC, 0 };

	log_dbg("");
	if (!hostname)
		return E_BAD_ARGS;
	proto = getprotobyname("udp");
	sock = socket(PF_INET, SOCK_DGRAM, proto->p_proto);

	if (setsockopt
	    (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_struct,
	     sizeof(time_struct)) < 0) {
		log_err("Failed to set socket options");
		return E_BAD_ARGS;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	host_resolv = gethostbyname(hostname);
	if (!host_resolv) {
		log_err("Failed to resolve host name");
		return E_HTTP_ERROR;
	}

	server_addr.sin_addr.s_addr =
	    inet_addr(inet_ntoa
		      (*(struct in_addr *)host_resolv->h_addr_list[0]));
	server_addr.sin_port = htons(NTP_PORT);

	res =
	    sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&server_addr,
		   sizeof(server_addr));
	if (res != 48) {
		log_err("Failed to send request to socket");
		return E_BAD_ARGS;
	}

	saddr_l = sizeof(saddr);
	res = recvfrom(sock, buf, 48, 0, &saddr, &saddr_l);
	if (res != 48) {
		log_err("Timeout on receiving response over the socket");
		return E_BAD_ARGS;
	}

	time_struct.tv_sec = ntohl((time_t) buf[4]) - EPOCH_BALANCE;
	res = settimeofday(&time_struct, NULL);
	if (res != 0) {
		log_err("Failed to set new time");
		return E_BAD_ARGS;
	}

	return S_OK;
}
