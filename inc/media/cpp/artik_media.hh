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

#ifndef MEDIA_CPP_ARTIK_MEDIA_HH_
#define MEDIA_CPP_ARTIK_MEDIA_HH_

#include <artik_module.h>
#include <artik_media.h>

/*! \file artik_media.hh
 *
 *  \brief C++ Wrapper to the Media module
 *
 *  This is a class encapsulation of the C
 *  Media module API \ref artik_media.h
 */

namespace artik {
/*!
 *  \brief Media module C++ Class
 */
class Media {
 private:
  artik_media_module* m_module;

 public:
  Media();
  ~Media();

  artik_error play_sound_file(const char*);
  artik_error set_finished_callback(artik_media_finished_callback user_callback,
      void *user_data);
};

}  // namespace artik

#endif  // MEDIA_CPP_ARTIK_MEDIA_HH_
