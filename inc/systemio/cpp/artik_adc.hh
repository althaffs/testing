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

#ifndef SYSTEMIO_CPP_ARTIK_ADC_HH_
#define SYSTEMIO_CPP_ARTIK_ADC_HH_

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_adc.h>

/*! \file artik_adc.hh
 *  \brief C++ Wrapper to the ADC module
 *
 *  This is a class encapsulation of the C
 *  ADC module API \ref artik_adc.h
 *  Example of usage see \ref artik_adc_test.cpp
 */

namespace artik {
/*!
 *  \brief Adc Module C++ Class
 */
class Adc {
 private:
  artik_adc_module* m_module;
  artik_adc_handle  m_handle;
  artik_adc_config  m_config;

 public:
  Adc(unsigned int, char*);
  explicit Adc(artik_adc_config&);
  Adc(Adc const &);
  Adc();
  ~Adc();

  Adc &operator=(Adc const &);

 public:
  artik_error release(void);
  artik_error request(void);
  artik_error get_value(int*);

  unsigned int get_pin_num(void) const;
  char* get_name(void) const;

  void set_pin_num(unsigned int);
  void set_name(char*);
};

}  // namespace artik

#endif  // SYSTEMIO_CPP_ARTIK_ADC_HH_
