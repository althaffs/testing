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


#include <stdio.h>
#include <unistd.h>

#include <artik_module.h>
#include <artik_media.h>
#include <artik_loop.h>

static char *sound_filename = NULL;

static void on_finished(void *userdata)
{
	artik_loop_module *loop = (artik_loop_module *)userdata;

	fprintf(stdout, "TEST: finished playing sound\n");

	loop->quit();
}

static artik_error media_test_sound_playback(void)
{
	artik_error ret = S_OK;
	artik_media_module *media = (artik_media_module *)
					artik_request_api_module("media");
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = media->play_sound_file(sound_filename);
	if (ret != S_OK)
		goto exit;

	ret = media->set_finished_callback(on_finished, (void *)loop);
	if (ret != S_OK)
		goto exit;

	loop->run();

exit:
	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	artik_release_api_module(media);
	artik_release_api_module(loop);

	return ret;
}

int main(int argc, char *argv[])
{
	int opt;
	artik_error ret = S_OK;

	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
		case 'f':
			sound_filename = strndup(optarg, strlen(optarg));
			break;
		default:
			printf("Usage: media-test [-f <filename to play>]\r\n");
			return 0;
		}
	}

	ret = media_test_sound_playback();

	if (sound_filename != NULL)
		free(sound_filename);

	return (ret == S_OK) ? 0 : -1;
}
