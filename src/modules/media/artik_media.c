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

#include <artik_module.h>
#include <artik_media.h>
#include "os_media.h"

static artik_error play_sound_file(const char *file_path);
static artik_error set_finished_callback(
			artik_media_finished_callback user_callback,
			void *user_data);

const artik_media_module media_module = {
	play_sound_file,
	set_finished_callback
};

artik_error play_sound_file(const char *file_path)
{
	return os_play_sound_file(file_path);
}


artik_error set_finished_callback(artik_media_finished_callback user_callback,
			void *user_data)
{
	return os_set_finished_callback(user_callback, user_data);
}
