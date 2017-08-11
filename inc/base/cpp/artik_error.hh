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

#ifndef BASE_CPP_ARTIK_ERROR_HH_
#define BASE_CPP_ARTIK_ERROR_HH_

#include <artik_error.h>
#include <stdarg.h>

#if __cplusplus__ > 199711L
#define artik_throw nowexcept
#else
#define artik_throw throw
#endif

/*! \file artik_error.hh
 *
 *  \brief C++ handling of exceptions
 *
 */
namespace artik {
/*!
 *  \brief Exception base class
 */
class ArtikException {
 private:
  int error;

 public:
  explicit ArtikException(int nb) artik_throw() : error(nb)  { this->what(); }
  ArtikException() artik_throw() : error(E_TRY_AGAIN)  { this->what(); }

  virtual ~ArtikException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return error_msg(this->error);
  }
};

/*!
 *  \brief Bad arguments exception class
 */
class ArtikBadArgsException : virtual public ArtikException {
 public:
  ArtikBadArgsException() artik_throw() : ArtikException(E_BAD_ARGS)  {;}
  virtual ~ArtikBadArgsException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Busy exception class
 */
class ArtikBusyException : virtual public ArtikException {
 public:
  ArtikBusyException() artik_throw() : ArtikException(E_BUSY)  {;}
  virtual ~ArtikBusyException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Not Initialized exception class
 */
class ArtikInitException : virtual public ArtikException {
 public:
  ArtikInitException() artik_throw() : ArtikException(E_NOT_INITIALIZED)  {;}
  virtual ~ArtikInitException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Out of memory exception class
 */
class ArtikNoMemException : virtual public ArtikException {
 public:
  ArtikNoMemException() artik_throw() : ArtikException(E_NO_MEM)  {;}
  virtual ~ArtikNoMemException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Not supported exception class
 */
class ArtikSupportException : virtual public ArtikException {
 public:
  ArtikSupportException() artik_throw() : ArtikException(E_NOT_SUPPORTED)  {;}
  virtual ~ArtikSupportException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Overflow exception class
 */
class ArtikOverflowException : virtual public ArtikException {
 public:
  ArtikOverflowException() artik_throw() : ArtikException(E_OVERFLOW)  {;}
  virtual ~ArtikOverflowException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Access denied exception class
 */
class ArtikDeniedException : virtual public ArtikException {
 public:
  ArtikDeniedException() artik_throw() : ArtikException(E_ACCESS_DENIED)  {;}
  virtual ~ArtikDeniedException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Interrupted exception class
 */
class ArtikInterruptException : virtual public ArtikException {
 public:
  ArtikInterruptException() artik_throw() : ArtikException(E_INTERRUPTED)  {;}
  virtual ~ArtikInterruptException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief HTTP error exception class
 */
class ArtikHTTPException : virtual public ArtikException {
 public:
  ArtikHTTPException() artik_throw() : ArtikException(E_HTTP_ERROR)  {;}
  virtual ~ArtikHTTPException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Try again exception class
 */
class ArtikAgainException : virtual public ArtikException {
 public:
  ArtikAgainException() artik_throw() : ArtikException(E_TRY_AGAIN)  {;}
  virtual ~ArtikAgainException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Timeout arguments exception class
 */
class ArtikTimeException : virtual public ArtikException {
 public:
  ArtikTimeException() artik_throw() : ArtikException(E_TIMEOUT)  {;}
  virtual ~ArtikTimeException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

/*!
 *  \brief Bad value exception class
 */
class ArtikBadValException : virtual public ArtikException {
 public:
  ArtikBadValException() artik_throw() : ArtikException(E_INVALID_VALUE)  {;}
  virtual ~ArtikBadValException() artik_throw() {;}

  virtual const char * what(void) artik_throw() {
    return this->ArtikException::what();
  }
};

}  // namespace artik

#endif  // BASE_CPP_ARTIK_ERROR_HH_
