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

#ifndef SYSTEMIO_CPP_ARTIK_SPI_HH_
#define SYSTEMIO_CPP_ARTIK_SPI_HH_

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_spi.h>

/*! \file artik_spi.hh
 *
 *  \brief C++ Wrapper to the SPI module
 *
 *  This is a class encapsulation of the C
 *  SPI module API \ref artik_spi.h
 */

namespace artik {
/*!
 *  \brief Spi Module C++ Class
 */
class Spi {
 private:
  artik_spi_module* m_module;
  artik_spi_handle  m_handle;
  artik_spi_config  m_config;

 public:
  Spi(unsigned int bus, unsigned int cs, artik_spi_mode mode,
  unsigned int bits_per_word, unsigned int speed);
  ~Spi();

  artik_error request(void);
  artik_error release(void);
  artik_error read(char*, int);
  artik_error write(char*, int);
  artik_error read_write(char*, char*, int);
};

}  // namespace artik

#endif  // SYSTEMIO_CPP_ARTIK_SPI_HH_
