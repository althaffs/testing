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

#ifndef	SRC_MEDIA_OS_MEDIA_H_
#define	SRC_MEDIA_OS_MEDIA_H_

#include "artik_error.h"

artik_error os_play_sound_file(const char *file_path);
artik_error os_set_finished_callback(
				artik_media_finished_callback user_callback,
				void *user_data);

#endif	/* SRC_MEDIA_OS_MEDIA_H_ */
