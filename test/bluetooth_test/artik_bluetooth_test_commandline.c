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

#include "artik_bluetooth_test_commandline.h"

#define HELP_COMMAND "help"
#define HELP_DESC    "Type '"HELP_COMMAND" [COMMAND]' for more details on a command."
#define UNKNOWN_CMD_MSG "Unknown command. Type '"HELP_COMMAND"' for help."


static command_desc_t *prv_find_command(command_desc_t *command_array,
		char *buffer, size_t length) {
	int i;

	if (length == 0)
		return NULL;

	i = 0;
	while (command_array[i].name != NULL
			&& (strlen(command_array[i].name) != length
					|| strncmp(buffer, command_array[i].name, length)))
		i++;

	if (command_array[i].name == NULL)
		return NULL;
	else
		return &command_array[i];
}

static void prv_display_help(command_desc_t *command_array, char *buffer)
{
	command_desc_t *cmd_p;
	int length;

	length = 0;
	while (buffer[length] != 0 && !isspace(buffer[length] & 0xff))
		length++;

	cmd_p = prv_find_command(command_array, buffer, length);

	if (cmd_p == NULL) {
		int i;

		fprintf(stdout, HELP_COMMAND"\t"HELP_DESC"\r\n");

		for (i = 0; command_array[i].name != NULL; i++) {
			fprintf(stdout, "%s\t%s\r\n", command_array[i].name,
					command_array[i].short_desc);
		}
	} else {
		fprintf(stdout, "%s\r\n",
				cmd_p->long_desc ? cmd_p->long_desc : cmd_p->short_desc);
	}
}


void handle_command(command_desc_t *command_array, char *buffer)
{
	command_desc_t *cmd_p;
	int length;

	length = 0;
	while (buffer[length] != 0 && !isspace(buffer[length] & 0xFF))
		length++;

	cmd_p = prv_find_command(command_array, buffer, length);
	if (cmd_p != NULL) {
		while (buffer[length] != 0 && isspace(buffer[length] & 0xFF))
			length++;
		cmd_p->call_back(buffer + length, cmd_p->user_data);
	} else {
		if (!strncmp(buffer, HELP_COMMAND, length)) {
			while (buffer[length] != 0 && isspace(buffer[length] & 0xFF))
				length++;
			prv_display_help(command_array, buffer + length);
		} else {
			fprintf(stdout, UNKNOWN_CMD_MSG"\r\n");
		}
	}
}

static char *prv_end_of_space(char *buffer)
{
	while (isspace(buffer[0] & 0xff))
		buffer++;
	return buffer;
}

char *get_end_of_arg(char *buffer)
{
	while (buffer[0] != 0 && !isspace(buffer[0] & 0xFF))
		buffer++;
	return buffer;
}

char *get_next_arg(char *buffer, char **end)
{
	buffer = get_end_of_arg(buffer);
	buffer = prv_end_of_space(buffer);
	if (end)
		*end = get_end_of_arg(buffer);

	return buffer;
}

int check_end_of_args(char *buffer)
{
	buffer = prv_end_of_space(buffer);

	return (buffer[0] == 0);
}
