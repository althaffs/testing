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

#ifndef BASE_CPP_ARTIK_TIME_HH_
#define BASE_CPP_ARTIK_TIME_HH_

#include <artik_module.h>
#include <artik_time.h>

/*! \file artik_time.hh
 *  \brief C++ Wrapper to the Time module
 *
 *  This is a class encapsulation of the C
 *  Time module API \ref artik_time.h
 */

namespace artik {
/*!
 *  \brief Alarm C++ Class
 */
class Alarm {
 private:
  artik_time_module* m_module;
  artik_alarm_handle m_handle;

 protected:
  artik_alarm_handle *get_handle(void);
  artik_alarm_handle *get_handle(void) const;

 public:
  Alarm(Alarm const &);
  Alarm(artik_time_zone, artik_time &, alarm_callback, void *,
      artik_time_module *);
  Alarm(artik_time_zone, artik_msecond, alarm_callback, void *,
      artik_time_module *);

  Alarm();
  ~Alarm();

  Alarm &operator=(Alarm const &);

  artik_error get_delay(artik_msecond *);
};

/*!
 *  \brief Time C++ Class
 */
class Time {
 private:
  artik_time_module* m_module;

 public:
  Time(Time const &);
  Time();
  ~Time();

  Time &operator=(Time const &);

 public:
  artik_error set_time(artik_time, artik_time_zone);
  artik_error get_time(artik_time_zone, artik_time*) const;
  artik_error get_time_str(char *, int, char *const, artik_time_zone) const;
  artik_msecond get_tick(void) const;
  artik_error sync_ntp(const char*);
  Alarm *create_alarm_second(artik_time_zone, artik_msecond, alarm_callback,
      void *);
  Alarm *create_alarm_date(artik_time_zone, artik_time, alarm_callback, void *);
  int compare_dates(const artik_time *date1, const artik_time *date2);
};

}  // namespace artik

#endif  // BASE_CPP_ARTIK_TIME_HH_
