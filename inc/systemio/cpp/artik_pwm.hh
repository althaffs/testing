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

#ifndef SYSTEMIO_CPP_ARTIK_PWM_HH_
#define SYSTEMIO_CPP_ARTIK_PWM_HH_

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_pwm.h>

/*! \file artik_pwm.hh
 *  \brief C++ Wrapper to the PWM module
 *
 *  This is a class encapsulation of the C
 *  PWM module API \ref artik_pwm.h
 *  Example of usage see \ref artik_pwm_test.cpp
 */

namespace artik {
/*!
 *  \brief Pwm Module C++ Class
 */
class Pwm {
 private:
  artik_pwm_module* m_module;
  artik_pwm_handle  m_handle;
  artik_pwm_config  m_config;

 public:
  Pwm(unsigned int, char*, unsigned int, artik_pwm_polarity_t, unsigned int);
  explicit Pwm(artik_pwm_config&);
  explicit Pwm(Pwm const &);
  Pwm();
  ~Pwm();

  Pwm &operator=(Pwm const &);

 public:
  artik_error release(void);
  artik_error request(void);
  artik_error enable(void);
  artik_error disable(void);
  artik_error set_period(unsigned int);
  artik_error set_polarity(artik_pwm_polarity_t);
  artik_error set_duty_cycle(unsigned int);

  unsigned int get_pin_num(void) const;
  char* get_name(void) const;
  unsigned int get_period(void) const;
  artik_pwm_polarity_t get_polarity(void) const;
  unsigned int get_duty_cycle(void) const;

  void set_pin_num(unsigned int);
  void set_name(char*);
};

}  // namespace artik

#endif  // SYSTEMIO_CPP_ARTIK_PWM_HH_
