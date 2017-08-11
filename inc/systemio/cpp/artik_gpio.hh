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

#ifndef SYSTEMIO_CPP_ARTIK_GPIO_HH_
#define SYSTEMIO_CPP_ARTIK_GPIO_HH_

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_gpio.h>

/*! \file artik_gpio.hh
 *
 *  \brief C++ Wrapper to the GPIO module
 *
 *  This is a class encapsulation of the C
 *  GPIO module API \ref artik_gpio.h
 */
namespace artik {
/*!
 *  \brief Gpio Module C++ Class
 */
class Gpio {
 private:
  artik_gpio_module* m_module;
  artik_gpio_handle  m_handle;
  artik_gpio_config  m_config;

 public:
  Gpio(artik_gpio_id id, char* name, artik_gpio_dir_t dir,
      artik_gpio_edge_t edge, int initial_value);
  ~Gpio();

  artik_error request(void);
  artik_error release(void);
  int read(void);
  artik_error write(int value);
  char* get_name(void);
  artik_gpio_dir_t get_direction(void);
  artik_gpio_id get_id(void);
  artik_error set_change_callback(artik_gpio_callback, void*);
  void unset_change_callback();
};

}  // namespace artik

#endif  // SYSTEMIO_CPP_ARTIK_GPIO_HH_
