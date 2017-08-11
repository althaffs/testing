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
#include <stdlib.h>
#include <string.h>

#include <artik_log.h>

#define MAX_FIELDSIZE_FILENAME 30
#define MAX_FIELDSIZE_FUNCNAME 30

static enum artik_log_system _log_system = LOG_SYSTEM_STDERR;
static enum artik_log_prefix _log_prefix_fields = (LOG_PREFIX_LEVEL |
					LOG_PREFIX_FUNCTION | LOG_PREFIX_LINE);
static artik_log_handler _log_handler;
static void *_log_handler_user_data;
static int _log_override_enabled;
static int _log_override_checked;

/* Initialization must follow artik_log_level order form artik_log.h */
static char _log_level_map[] = { 'E', 'W', 'I', 'D' };

static void _log_check_override(void)
{
	if (_log_override_checked)
		return;

	_log_override_checked = TRUE;

	/* No override method implemented so far */
}

static int _log_make_prefix(char *prefix, int prefix_len,
	enum artik_log_level level, const char *filename, const char *funcname,
	int line)
{
	const char *pretty_filename = NULL;
	int len = 0;

	if (_log_prefix_fields & LOG_PREFIX_LEVEL) {
		prefix[len++] = _log_level_map[level];
		prefix[len++] = ' ';
	}

	if (_log_prefix_fields & LOG_PREFIX_FILENAME) {
		pretty_filename = strrchr(filename, '/');
		if (!pretty_filename)
			pretty_filename = filename;
		else
			pretty_filename++;
	}

	if (_log_prefix_fields & LOG_PREFIX_FILEPATH)
		pretty_filename = filename;

	if (pretty_filename) {
		size_t field_len;

		field_len = strlen(pretty_filename);
		if (field_len > MAX_FIELDSIZE_FILENAME)
			len += snprintf(prefix + len,
				MAX_FIELDSIZE_FILENAME + 5, "<~%s> ",
				pretty_filename + field_len -
				MAX_FIELDSIZE_FILENAME);
		else
			len += snprintf(prefix + len, field_len + 5, "<%s> ",
				pretty_filename);
		/* Filename with line number */
		if (_log_prefix_fields & LOG_PREFIX_LINE) {
			len--;
			len--;
			len += snprintf(prefix + len, 9, ":%d> ", line);
			*(prefix + len - 1) = ' ';
		}
	} else {
		/* Standalone line number */
		if (_log_prefix_fields & LOG_PREFIX_LINE) {
			len += snprintf(prefix + len, 9, "<%d> ", line);
			*(prefix + len - 1) = ' ';
		}
	}

	if ((_log_prefix_fields & LOG_PREFIX_FUNCTION) && funcname) {
		size_t field_len;

		field_len = strlen(funcname);
		if (field_len > MAX_FIELDSIZE_FUNCNAME)
			len += snprintf(prefix + len,
				MAX_FIELDSIZE_FUNCNAME + 3, "~%s ", funcname +
				field_len - MAX_FIELDSIZE_FUNCNAME);
		else
			len += snprintf(prefix + len, field_len + 2, "%s ",
				funcname);
	}

	/* Remove last space */
	if (len > 0) {
		if (*(prefix + len - 1) == ' ') {
			*(prefix + len - 1) = 0;
			len--;
		}
	}

	return len;
}

static void _log_formatted(enum artik_log_level level, const char *filename,
			   const char *funcname, int line, const char *format,
			   va_list arg)
{
	char prefix[32] = { 0 };
	int len = 0;

	if (_log_prefix_fields > LOG_PREFIX_NONE)
		len = _log_make_prefix(prefix, 32, level, filename, funcname,
					line);

	if (_log_system == LOG_SYSTEM_STDERR) {
		if (len > 0)
			fprintf(stderr, "%s ", prefix);
		vfprintf(stderr, format, arg);
		fputc('\n', stderr);
		fflush(stderr);
	} else if (_log_system == LOG_SYSTEM_CUSTOM && _log_handler) {
		char msg[256];

		vsnprintf(msg, 256, format, arg);
		_log_handler(level, prefix, msg, _log_handler_user_data);
	}
}

void os_log_print(enum artik_log_level level, const char *filename,
		const char *funcname, int line, const char *format, va_list arg)
{
	if (!_log_override_checked)
		_log_check_override();

	switch (_log_system) {
	case LOG_SYSTEM_STDERR:
	case LOG_SYSTEM_CUSTOM:
		_log_formatted(level, filename, funcname, line, format, arg);
		break;
	case LOG_SYSTEM_NONE:
	default:
		break;
	}
}

artik_error os_log_set_system(enum artik_log_system system)
{
	if (system > LOG_SYSTEM_CUSTOM) {
		log_err("invalid system(%d)", system);
		return E_BAD_ARGS;
	}

	if (_log_override_enabled)
		return 0;
	_log_system = system;

	return S_OK;
}

artik_error os_log_set_handler(artik_log_handler handler, void *user_data)
{
	if (!handler) {
		log_err("handler is NULL");
		return E_BAD_ARGS;
	}

	if (_log_override_enabled)
		return S_OK;

	_log_system = LOG_SYSTEM_CUSTOM;
	_log_handler = handler;
	_log_handler_user_data = user_data;

	return S_OK;
}

void os_log_set_prefix_fields(enum artik_log_prefix field_set)
{
	 _log_prefix_fields = field_set;
}

enum artik_log_prefix os_log_get_prefix_fields(void)
{
	return _log_prefix_fields;
}
