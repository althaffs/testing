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

/*! \file artik_time_test.c
 *
 *  \brief TIME Test example in C
 *
 *  Instance of usage TIME module with
 *  a program developed in C.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <artik_module.h>
#include <artik_loop.h>
#include <artik_time.h>

#define MAX_SIZE 128

static int end = 1;
artik_time_module *time_module_p;
artik_loop_module *loop;
artik_alarm_handle handle_date, handle_second;
char *hostname = "fr.pool.ntp.org";

static void sig_handler(int sig)
{
	loop->quit();
}

static void _alarm_callback_1(void *user_data)
{
	printf("Callback from alarm second\n");
}

static void _alarm_callback_2(void *user_data)
{
	printf("Callback from alarm date\n");
	loop->quit();
}

typedef void (*t_ptr_func) (int);

static artik_error test_time_loopback(void)
{
	artik_error ret = S_OK;
	artik_msecond inittime = 0, valtime = 0, oldtime = 0, nb_seconds = 1500;
	artik_time val;
	artik_time alarm_date;
	char date[MAX_SIZE] = "";

	loop = (artik_loop_module *)artik_request_api_module("loop");

	alarm_date.second = 0;
	alarm_date.minute = 18;
	alarm_date.hour = 18;
	alarm_date.day = 31;
	alarm_date.month = 7;
	alarm_date.year = 2017;
	alarm_date.day_of_week = 1;
	alarm_date.msecond = 0;

	time_module_p->get_time(ARTIK_TIME_UTC, &val);

	time_module_p->get_time_str(date, MAX_SIZE, 0, ARTIK_TIME_UTC);

	fprintf(stdout, "UTC GET TIME STR : %s\n", date);

	fprintf(stdout, "Succeed set time [%d]\n",
		time_module_p->set_time(alarm_date, ARTIK_TIME_UTC));

	fprintf(stdout, "GET TIME : Val year (%u) month(%u) day(%d)"\
		" hour(%d) minute(%d)\n",
		val.year, val.month, val.day, val.hour, val.minute);

	time_module_p->get_time_str(date, MAX_SIZE, 0, ARTIK_TIME_GMT2);

	fprintf(stdout, "GMT GET TIME STR : %s\n", date);

	time_module_p->get_time_str(date, MAX_SIZE, "%Y/%m/%d %I:%M:%S",
				    ARTIK_TIME_UTC);

	fprintf(stdout, "UTC GET TIME STR : %s\n", date);

	inittime = time_module_p->get_tick();

	while (end) {
		valtime = time_module_p->get_tick();
		if (((valtime - inittime) > nb_seconds && valtime != oldtime)) {
			time_module_p->get_time_str(date, MAX_SIZE,
						    ARTIK_TIME_DFORMAT,
						    ARTIK_TIME_GMT2);
			fprintf(stdout, "%s\n", date);
			end--;
			oldtime = valtime;
		}
	}

	end = 1;

	time_module_p->create_alarm_second(ARTIK_TIME_GMT2, 5, &handle_second,
						&_alarm_callback_1, NULL);

	time_module_p->create_alarm_date(ARTIK_TIME_GMT2, alarm_date,
					&handle_date, &_alarm_callback_2, NULL);

	artik_msecond delay;

	time_module_p->get_delay_alarm(handle_date, &delay);

	fprintf(stdout, "The delay is %ld s for alarm_date\n", delay);

	fprintf(stdout,
		"Wait for the alarms...(Click on ctrl C for passing"\
		" to next step)\n");

	signal(SIGINT, sig_handler);

	loop->run();

	time_module_p->delete_alarm(handle_date);

	time_module_p->delete_alarm(handle_second);

	fprintf(stdout, "Release TIME Module\n");

	return ret;
}

artik_error test_time_sync_ntp(void)
{
	artik_error ret;
	time_t curr_time;

	fprintf(stdout, "TEST: %s started\n", __func__);

	curr_time = time(0);
	fprintf(stdout, "Current system time: %s", ctime(&curr_time));

	ret = time_module_p->sync_ntp(hostname);
	if (ret != S_OK) {
		fprintf(stdout, "TEST: %s failed: ERROR(%d)\n", __func__,
			ret);
		return ret;
	}

	curr_time = time(0);

	fprintf(stdout, "Modified system time: %s", ctime(&curr_time));
	fprintf(stdout, "TEST: %s finished\n", __func__);

	return ret;
}

int main(void)
{
	artik_error ret = S_OK;

	time_module_p = (artik_time_module *)artik_request_api_module("time");

	ret = test_time_loopback();
	if (ret != S_OK)
		goto exit;

	ret = test_time_sync_ntp();

exit:
	artik_release_api_module(time_module_p);

	return (ret == S_OK) ? 0 : -1;
}
