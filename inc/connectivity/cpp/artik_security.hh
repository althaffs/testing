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

#ifndef CONNECTIVITY_CPP_ARTIK_SECURITY_HH_
#define CONNECTIVITY_CPP_ARTIK_SECURITY_HH_

#include <artik_security.h>
#include <artik_module.h>
#include <artik_error.hh>

/*! \file artik_security.hh
 *
 *  \brief C++ Wrapper to the Security module
 *
 *  This is a class encapsulation of the C
 *  Security module API \ref artik_security.h
 */
namespace artik {
/*!
 * \brief Security module C++ class
 */
class Security {
 private:
  artik_security_module *m_module;
  artik_security_handle m_handle;

 public:
  Security();
  ~Security();

  artik_error get_certificate(char **);
  artik_error get_key_from_cert(const char *, char **);
  artik_error get_random_bytes(unsigned char*, int);
  artik_error get_certificate_sn(unsigned char*, unsigned int *);
};

}  // namespace artik

#endif  // CONNECTIVITY_CPP_ARTIK_SECURITY_HH_
