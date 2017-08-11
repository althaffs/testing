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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <gst/gst.h>
#pragma GCC diagnostic pop

#include <stdio.h>
#include <sys/stat.h>

#include <artik_media.h>
#include <artik_log.h>
#include "os_media.h"

struct CustomData {
	GstElement *playbin;
	artik_media_finished_callback finish_cbk;
	void *cbk_user_data;
};

enum GstPlayFlags {
	GST_PLAY_FLAG_VIDEO = (1 << 0),	/* We want video output */
	GST_PLAY_FLAG_AUDIO = (1 << 1),	/* We want audio output */
	GST_PLAY_FLAG_TEXT = (1 << 2)	/* We want subtitle output */
};

static struct CustomData *data = NULL;

static gboolean handle_message(GstBus *bus, GstMessage *msg,
				void *user)
{
	GError *err = NULL;
	gchar *debug_info = NULL;

	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_ERROR:
		g_clear_error(&err);
		g_free(debug_info);
		break;
	case GST_MESSAGE_EOS:
		if (data) {
			gst_element_set_state(data->playbin, GST_STATE_NULL);
			gst_object_unref(data->playbin);
			if (data->finish_cbk)
				data->finish_cbk(data->cbk_user_data);

			free(data);
			data = NULL;
		}
		break;
	default:
		break;
	}

	return TRUE;
}

artik_error os_play_sound_file(const char *file_path)
{
	int ret = 0;
	char uri[MAX_FILEPATH_LEN + 7];
	gint flags = 0;
	GstBus *bus = NULL;
	struct stat info_file;

	log_dbg("");

	if (!file_path || (file_path && stat(file_path, &info_file) == -1))
		return E_BAD_ARGS;

	/* Do not allow playing two sounds at the same time */
	if (data)
		return E_BUSY;

	/* Alllocate custom data */
	data = (struct CustomData *)malloc(sizeof(*data));
	if (!data)
		return E_NO_MEM;

	memset(data, 0, sizeof(*data));

	/* Build uri string */
	snprintf(uri, MAX_FILEPATH_LEN, "file://%s", file_path);

	/* Initialize gstreamer */
	gst_init(NULL, NULL);

	/* Get the playbin element */
	data->playbin = gst_element_factory_make("playbin", "playbin");
	if (!data->playbin) {
		free(data);
		data = NULL;
		return E_BAD_ARGS;
	}

	/* Configure playbin */
	g_object_set(data->playbin, "uri", uri, NULL);
	g_object_get(data->playbin, "flags", &flags, NULL);
	flags |= GST_PLAY_FLAG_AUDIO;
	flags &= ~(GST_PLAY_FLAG_TEXT | GST_PLAY_FLAG_VIDEO);
	g_object_set(data->playbin, "flags", flags, NULL);

	/* Set up the message callback */
	bus = gst_pipeline_get_bus(GST_PIPELINE(data->playbin));
	gst_bus_add_watch(bus, (GstBusFunc) handle_message, NULL);
	gst_object_unref(bus);

	/* Start the stream */
	ret = gst_element_set_state(data->playbin, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		gst_object_unref(data->playbin);
		free(data);
		data = NULL;
		return E_BUSY;
	}

	return S_OK;
}

artik_error os_set_finished_callback(
				artik_media_finished_callback user_callback,
				void *user_data)
{
	log_dbg("");

	if (!data)
		return E_NOT_INITIALIZED;

	if (user_callback) {
		data->finish_cbk = user_callback;
		data->cbk_user_data = user_data;
	} else {
		data->finish_cbk = NULL;
		data->cbk_user_data = NULL;
	}

	return S_OK;
}
