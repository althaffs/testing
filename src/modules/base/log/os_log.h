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

#ifndef	__OS_LOG_H
#define	__OS_LOG_H

#include <stdarg.h>
#include "artik_error.h"

artik_error os_log_set_system(enum artik_log_system system);
artik_error os_log_set_handler(artik_log_handler handler, void *user_data);
void os_log_set_prefix_fields(enum artik_log_prefix field_set);
enum artik_log_prefix os_log_get_prefix_fields(void);
void os_log_print(enum artik_log_level level, const char *filename,
		const char *funcname, int line, const char *format,
		va_list arg);

#endif	/* __OS_LOG_H */
