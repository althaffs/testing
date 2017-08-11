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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <inttypes.h>

#include <artik_lwm2m.h>
#include <artik_module.h>
#include <artik_platform.h>
#include <artik_log.h>
#include "artik_lwm2m_test_common.h"

#define HELP_COMMAND "help"
#define HELP_DESC    "Type '"HELP_COMMAND" [COMMAND]' for more details."
#define UNKNOWN_CMD_MSG "Unknown command. Type '"HELP_COMMAND"' for help."


void prv_init_command(command *cmd, char *buffer)
{
	cmd->buffer = buffer;
	cmd->offset = 0;
}

bool prv_isspace(char c)
{
	return isspace(c&0xff);
}

int prv_next_str(command *cmd)
{
	int offset = cmd->offset;
	char c = 0;
	int result = 0;

	do {
		offset++;
		c = cmd->buffer[offset];
		if (c == 0) {
			result = -1;
			break;
		}

		if (prv_isspace(c) == false) {
			cmd->offset = offset;
			break;
		}
	} while (true);

	return result;
}

int prv_next_space(command *cmd)
{
	int offset = cmd->offset;
	char c = 0;
	int result = 0;

	do {
		offset++;
		c = cmd->buffer[offset];
		if (c == 0) {
			result = -1;
			break;
		}

		if (prv_isspace(c) == true) {
			cmd->offset = offset;
			break;
		}
	} while (true);

	return result;
}

int prv_read_id(command *cmd, unsigned short *id)
{
	char *str = NULL;

	str = cmd->buffer + cmd->offset;
	if (str[0] == 0)
		return -1;

	*id = atoi(str);

	prv_next_space(cmd);
	prv_next_str(cmd);

	return 0;
}

int prv_read_uri(command *cmd, char *uri)
{
	char *str = NULL;
	char c = 0;
	int i = 0;

	str = cmd->buffer + cmd->offset;
	for (i = 0; ; i++) {
		c = str[i];
		if (c == 0 || prv_isspace(c)) {
			uri[i] = 0;
			break;
		}

		uri[i] = c;
	}

	if (i == 0)
		return -1;

	cmd->offset += i;
	prv_next_str(cmd);

	return 0;
}

int prv_read_data(command *cmd, char *data)
{
	char *str = NULL;
	char c = 0;
	int i = 0;

	str = cmd->buffer + cmd->offset;
	for (i = 0;; i++) {
		c = str[i];
		if (c == 0 || prv_isspace(c) == true) {
			data[i] = 0;
			break;
		}

		data[i] = c;
	}

	if (i == 0)
		return -1;

	cmd->offset += i;

	return 0;
}

static struct command_desc_t *prv_find_command(
		struct command_desc_t *commands, char *buffer, size_t length)
{
	int i = 0;

	if (!length)
		return NULL;

	while (commands[i].name &&
		   (strlen(commands[i].name) != length	||
		    strncmp(buffer, commands[i].name, length))) {
		i++;
	}

	return commands[i].name ? &commands[i] : NULL;
}

static void prv_displayHelp(struct command_desc_t *commands, char *buffer)
{
	struct command_desc_t *cmd = NULL;
	int length = 0;

	while (buffer[length] && !isspace(buffer[length] & 0xff))
		length++;

	cmd = prv_find_command(commands, buffer, length);

	if (!cmd) {
		int i = 0;

		fprintf(stdout, HELP_COMMAND"\t"HELP_DESC"\r\n");

		for (i = 0; commands[i].name != NULL; i++) {
			fprintf(stdout, "%s\t%s\r\n", commands[i].name,
					commands[i].shortDesc);
		}
	} else {
		fprintf(stdout, "%s\r\n",
				cmd->longDesc ? cmd->longDesc : cmd->shortDesc);
	}
}

void handle_command(struct command_desc_t *commands, char *buffer)
{
	struct command_desc_t *cmd = NULL;
	int length = 0;

	while (buffer[length] && !isspace(buffer[length] & 0xff))
		length++;

	cmd = prv_find_command(commands, buffer, length);
	if (cmd) {
		while (buffer[length] && isspace(buffer[length] & 0xff))
			length++;
		cmd->callback(buffer + length, cmd->user_data);
	} else {
		if (!strncmp(buffer, HELP_COMMAND, length)) {
			while (buffer[length] && isspace(buffer[length] & 0xff))
				length++;
			prv_displayHelp(commands, buffer + length);
		} else {
			fprintf(stdout, UNKNOWN_CMD_MSG"\r\n");
		}
	}
}

static void print_indent(FILE *stream, int num)
{
	int i = 0;

	for (i = 0; i < num; i++)
		fprintf(stream, "    ");
}

void output_buffer(FILE *stream, unsigned char *buffer, int length, int indent)
{
	int i = 0;

	if (length == 0)
		fprintf(stream, "\n");

	while (i < length) {
		unsigned char array[16];
		int j = 0;

		print_indent(stream, indent);
		memcpy(array, buffer + i, 16);
		for (j = 0; j < 16 && i + j < length; j++) {
			fprintf(stream, "%02X ", array[j]);
			if (j % 4 == 3)
				fprintf(stream, " ");
		}
		if (length > 16) {
			while (j < 16) {
				fprintf(stream, "   ");
				if (j % 4 == 3)
					fprintf(stream, " ");
				j++;
			}
		}
		fprintf(stream, " ");
		for (j = 0; j < 16 && i + j < length; j++) {
			if (isprint(array[j]))
				fprintf(stream, "%c", array[j]);
			else
				fprintf(stream, ".");
		}
		fprintf(stream, "\n");
		i += 16;
	}
}
