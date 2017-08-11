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

#include "artik_i2c.h"
#include "artik_list.h"
#include "os_i2c.h"

static artik_error artik_i2c_request(artik_i2c_handle * handle,
				     artik_i2c_config * config);
static artik_error artik_i2c_release(artik_i2c_handle handle);
static artik_error artik_i2c_read(artik_i2c_handle handle, char *buf, int len);
static artik_error artik_i2c_write(artik_i2c_handle handle, char *buf, int len);
static artik_error artik_i2c_read_register(artik_i2c_handle handle,
					   unsigned int addr, char *buf,
					   int len);
static artik_error artik_i2c_write_register(artik_i2c_handle handle,
					    unsigned int addr, char *buf,
					    int len);

const artik_i2c_module i2c_module = {
	artik_i2c_request,
	artik_i2c_release,
	artik_i2c_read,
	artik_i2c_write,
	artik_i2c_read_register,
	artik_i2c_write_register
};

typedef struct {
	artik_list node;
	artik_i2c_config config;
} i2c_node;

static artik_list *requested_node = NULL;

static int check_exist(i2c_node *elem, artik_i2c_config *config)
{
	if ((elem->config.id == config->id)
	    && (elem->config.address == config->address))
		return 1;
	return 0;
}

artik_error artik_i2c_request(artik_i2c_handle *handle,
			      artik_i2c_config *config)
{
	i2c_node *node = (i2c_node *)artik_list_get_by_check(requested_node,
				(ARTIK_LIST_FUNCB)&check_exist, (void *)config);
	artik_error ret = S_OK;

	if (node)
		return E_BUSY;
	node = (i2c_node *) artik_list_add(&requested_node, 0,
					sizeof(i2c_node));
	if (!node) {
		/* node no memory to consume */
		return E_NO_MEM;
	}
	ret = os_i2c_request(config);
	if (ret == S_OK) {
		node->node.handle = (ARTIK_LIST_HANDLE) node;
		memcpy(&node->config, config, sizeof(node->config));
		*handle = (artik_i2c_handle)node;
	} else {
		/* node request failed */
		artik_list_delete_node(&requested_node, (artik_list *)node);
	}
	return ret;
}

artik_error artik_i2c_release(artik_i2c_handle handle)
{
	i2c_node *node = (i2c_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);
	artik_error ret = S_OK;

	if (!node)
		return E_BAD_ARGS;
	ret = os_i2c_release(&node->config);
	if (ret != S_OK)
		return ret;
	artik_list_delete_node(&requested_node, (artik_list *)node);
	return ret;
}

artik_error artik_i2c_read(artik_i2c_handle handle, char *buf, int len)
{
	i2c_node *node = (i2c_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_i2c_read(&node->config, buf, len);
}

artik_error artik_i2c_write(artik_i2c_handle handle, char *buf, int len)
{
	i2c_node *node = (i2c_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_i2c_write(&node->config, buf, len);
}

artik_error artik_i2c_read_register(artik_i2c_handle handle, unsigned int reg,
				    char *buf, int len)
{
	i2c_node *node = (i2c_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_i2c_read_register(&node->config, reg, buf, len);
}

artik_error artik_i2c_write_register(artik_i2c_handle handle, unsigned int reg,
				     char *buf, int len)
{
	i2c_node *node = (i2c_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_i2c_write_register(&node->config, reg, buf, len);
}
