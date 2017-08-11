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

#ifndef BASE_CPP_ARTIK_LOOP_HH_
#define BASE_CPP_ARTIK_LOOP_HH_

#include <artik_module.h>
#include <artik_loop.h>

/*! \file artik_loop.hh
 *  \brief C++ Wrapper to the Loop module
 *
 *  This is a class encapsulation of the C
 *  Loop module API \ref artik_loop.h
 */
namespace artik {
/*!
 *  \brief Loop C++ Class
 */
class Loop {
 private:
  artik_loop_module* m_module;

 public:
  explicit Loop(Loop const &);
  Loop();
  ~Loop();

  Loop &operator=(Loop const &);

 public:
  void run(void);
  void quit(void);
  artik_error add_timeout_callback(int *timeout_id, unsigned int msec,
      timeout_callback func, void *user_data);
  artik_error remove_timeout_callback(int timeout_id);
  artik_error add_periodic_callback(int *periodic_id, unsigned int msec,
      periodic_callback func, void *user_data);
  artik_error remove_periodic_callback(int periodic_id);
  artik_error add_fd_watch(int fd, enum watch_io io, watch_callback func,
      void *user_data, int *watch_id);
  artik_error remove_fd_watch(int watch_id);
  artik_error add_signal_watch(int signum, signal_callback func,
      void *user_data, int *signal_id);
  artik_error remove_signal_watch(int signal_id);
  artik_error add_idle_callback(int *idle_id, idle_callback func,
      void *user_data);
  artik_error remove_idle_callback(int idle_id);
};

}  // namespace artik

#endif  // BASE_CPP_ARTIK_LOOP_HH_
