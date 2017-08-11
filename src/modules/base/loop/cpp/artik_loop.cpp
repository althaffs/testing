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


#include "artik_loop.hh"

artik::Loop::Loop(artik::Loop const &loop) {
  this->m_module = loop.m_module;
}

artik::Loop::Loop() {
  this->m_module = reinterpret_cast<artik_loop_module*>(
      artik_request_api_module("loop"));
}

artik::Loop::~Loop() {
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik::Loop &artik::Loop::operator=(artik::Loop const &loop) {
  this->m_module = loop.m_module;
  return *this;
}

void artik::Loop::run(void) {
  this->m_module->run();
}

void artik::Loop::quit(void) {
  this->m_module->quit();
}

artik_error artik::Loop::add_timeout_callback(int *timeout_id,
    unsigned int msec, timeout_callback func, void *user_data) {
  return this->m_module->add_timeout_callback(timeout_id, msec, func,
      user_data);
}

artik_error artik::Loop::remove_timeout_callback(int timeout_id) {
  return this->m_module->remove_timeout_callback(timeout_id);
}

artik_error artik::Loop::add_periodic_callback(int *periodic_id,
    unsigned int msec, periodic_callback func, void *user_data) {
  return this->m_module->add_periodic_callback(periodic_id, msec, func,
      user_data);
}

artik_error artik::Loop::remove_periodic_callback(int periodic_id) {
  return this->m_module->remove_periodic_callback(periodic_id);
}

artik_error artik::Loop::add_fd_watch(int fd, enum watch_io io,
    watch_callback func, void *user_data, int *watch_id) {
  return this->m_module->add_fd_watch(fd, io, func, user_data, watch_id);
}

artik_error artik::Loop::remove_fd_watch(int watch_id) {
  return this->m_module->remove_fd_watch(watch_id);
}

artik_error artik::Loop::add_signal_watch(int signum, signal_callback func,
    void *user_data, int *signal_id) {
  return this->m_module->add_signal_watch(signum, func, user_data, signal_id);
}

artik_error artik::Loop::remove_signal_watch(int signal_id) {
  return this->m_module->remove_signal_watch(signal_id);
}

artik_error artik::Loop::add_idle_callback(int *idle_id, idle_callback func,
    void *user_data) {
  return this->m_module->add_idle_callback(idle_id, func, user_data);
}

artik_error artik::Loop::remove_idle_callback(int idle_id) {
  return this->m_module->remove_idle_callback(idle_id);
}
