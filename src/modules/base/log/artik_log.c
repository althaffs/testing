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

#include <artik_log.h>
#include "os_log.h"

static artik_error artik_log_set_system(enum artik_log_system system);
static artik_error artik_log_set_handler(artik_log_handler handler,
					void *user_data);
static void artik_log_set_prefix_fields(enum artik_log_prefix field_set);
static enum artik_log_prefix artik_log_get_prefix_fields(void);
static void artik_log_print(enum artik_log_level level, const char *filename,
		const char *funcname, int line, const char *format, ...);

EXPORT_API const artik_log_module log_module = {
		artik_log_set_system,
		artik_log_set_handler,
		artik_log_set_prefix_fields,
		artik_log_get_prefix_fields,
		artik_log_print,
};

artik_error artik_log_set_system(enum artik_log_system system)
{
	return os_log_set_system(system);
}

artik_error artik_log_set_handler(artik_log_handler handler, void *user_data)
{
	return os_log_set_handler(handler, user_data);
}

void artik_log_set_prefix_fields(enum artik_log_prefix field_set)
{
	os_log_set_prefix_fields(field_set);
}

enum artik_log_prefix artik_log_get_prefix_fields(void)
{
	return os_log_get_prefix_fields();
}

void artik_log_print(enum artik_log_level level, const char *filename,
		const char *funcname, int line, const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	os_log_print(level, filename, funcname, line, format, arg);
	va_end(arg);
}
