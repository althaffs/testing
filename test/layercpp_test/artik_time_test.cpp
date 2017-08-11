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
#include <signal.h>

#include <artik_time.hh>
#include <artik_loop.hh>

#define MAX_SIZE 128

static unsigned int end = 3;
const char* hostname = "fr.pool.ntp.org";

static artik::Time* time = NULL;
static artik::Alarm *alarm_1 = NULL, *alarm_2 = NULL;
static artik::Loop* loop = NULL;

static void _alarm_callback_1(void *user_data) {
  printf("Callback from alarm second \n");
}

static void _alarm_callback_2(void *user_data) {
  printf("Callback from alarm date \n");
  loop->quit();
}

void sig_handler(int sig) {
  loop->quit();
}

int main(void) {
  artik_error ret = S_OK;
  artik_msecond inittime = 0, valtime = 0, oldtime = 0, nb_seconds = 1500;
  artik_time val;
  signal(SIGINT, (void(*)(int))sig_handler);
  artik_time alarm_date;
  char date[MAX_SIZE] = "";
  loop = new artik::Loop;

  alarm_date.second = 0;
  alarm_date.minute = 22;
  alarm_date.hour = 18;
  alarm_date.day = 31;
  alarm_date.month = 7;
  alarm_date.year = 2017;
  alarm_date.day_of_week = 1;
  alarm_date.msecond = 0;

  time = new artik::Time;

  time->get_time(ARTIK_TIME_UTC, &val);

  printf("[%d] \n", time->set_time(alarm_date, ARTIK_TIME_UTC));
  printf("Launch artik::Time cpp class\n");

  printf("GET TIME : Val year (%u) month(%u) day(%d)"\
          " hour(%d) minute(%d)\n",
          val.year, val.month, val.day, val.hour, val.minute);

  time->get_time_str(date, MAX_SIZE, 0, ARTIK_TIME_GMT2);

  printf("GET TIME STR : %s \n", date);

  time->get_time_str(date, MAX_SIZE, const_cast<char*>("%Y/%m/%d %I:%M:%S"),
      ARTIK_TIME_UTC);

  printf("GET TIME STR : %s \n", date);

  inittime = time->get_tick();

  while (end) {
    valtime = time->get_tick();
    if (((valtime - inittime) > nb_seconds && valtime != oldtime)) {
      time->get_time_str(date, MAX_SIZE, const_cast<char*>(ARTIK_TIME_DFORMAT),
          ARTIK_TIME_GMT2);
      printf("%s\n", date);
      end--;
      oldtime = valtime;
    }
  }

  end = 1;

  alarm_1 = time->create_alarm_second(ARTIK_TIME_GMT2, 5, _alarm_callback_1,
      NULL);
  alarm_2 = time->create_alarm_date(ARTIK_TIME_GMT2, alarm_date,
      _alarm_callback_2, NULL);

  artik_msecond delay;

  alarm_2->get_delay(&delay);

  printf("The delay is %ld s for alarm_2\n", delay);

  printf("Wait for the alarm...(Click on ctrl C for passing to next step)\n");

  loop->run();

  ret = time->sync_ntp(hostname);
  printf("sync_ntp result: (%d)\n", ret);
  printf("Release TIME Module \n");

  delete alarm_1;
  delete alarm_2;
  delete time;
  delete loop;

  return ret;
}
