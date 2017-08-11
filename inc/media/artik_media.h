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

#ifndef INCLUDE_ARTIK_MEDIA_H_
#define INCLUDE_ARTIK_MEDIA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "artik_error.h"
#include "artik_types.h"

/*! \file artik_media.h
 *
 *  \brief Media module definition
 *
 *  Definitions and functions for accessing
 *  the Media module and managing audio and
 *  video streams
 *
 *  \example media_test/artik_media_test.c
 */

/*!
 *  \brief Maximum length for path names
 *
 *  Maximum length allowed for passing
 *  file path names to the Media APIs
 */
#define MAX_FILEPATH_LEN	1024

/*!
 *  \brief Finish callback function type
 *
 *  Type of the callback function called
 *  after a media is finished processing
 */
typedef void (*artik_media_finished_callback)
			(void *userdata);

/*! \struct artik_media_module
 *
 *  \brief Media module operations
 *
 *  Structure containing all the exposed
 *  operations exposed by the Media module
 */
typedef struct {

	/*!
	 *  \brief Play a sound file of any format.
	 *         This function is blocking until the whole
	 *         file has been played.
	 *
	 *  \param[in] file_path Absolute path and filename of
	 *             the sound file to read. The file
	 *             can have any format (auto-detected by the
	 *             function).
	 *
	 *  \return S_OK on success, error code otherwise.
	 *          The function returns E_NOT_SUPPORTED
	 *          if the sound file format does not have associated
	 *          decoder available
	 */
	artik_error(*play_sound_file) (const char *file_path);
	/*!
	 * \brief Registers a callback function to be invoked when the
	 *        media processing finishes
	 *
	 * \param[in] callback The callback function to register. If NULL
	 *            is passed instead, a previously registered callback
	 *            will be unregistered.
	 * \param[in] user_data The user data to be passed to the callback
	 *            function
	 *
	 * \return S_OK on success, otherwise a negative error value.
	 *
	 */
	artik_error(*set_finished_callback)
			(artik_media_finished_callback user_callback,
			void *user_data);

} artik_media_module;

extern const artik_media_module media_module;

#ifdef __cplusplus
}
#endif
#endif				/* INCLUDE_ARTIK_MEDIA_H_ */
