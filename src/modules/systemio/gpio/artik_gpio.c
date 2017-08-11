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
#include <stdint.h>

#include <artik_gpio.h>
#include "os_gpio.h"

static artik_error artik_gpio_request(artik_gpio_handle * handle,
					artik_gpio_config * config);
static artik_error artik_gpio_release(artik_gpio_handle handle);
static int artik_gpio_read(artik_gpio_handle handle);
static artik_error artik_gpio_write(artik_gpio_handle handle, int value);
static char *artik_gpio_get_name(artik_gpio_handle handle);
static artik_gpio_dir_t artik_gpio_get_direction(artik_gpio_handle handle);
static artik_gpio_id artik_gpio_get_id(artik_gpio_handle);
static artik_error artik_gpio_set_change_callback(artik_gpio_handle handle,
					artik_gpio_callback callback, void *);
static void artik_gpio_unset_change_callback(artik_gpio_handle handle);

const artik_gpio_module gpio_module = {
		artik_gpio_request,
		artik_gpio_release,
		artik_gpio_read,
		artik_gpio_write,
		artik_gpio_get_name,
		artik_gpio_get_direction,
		artik_gpio_get_id,
		artik_gpio_set_change_callback,
		artik_gpio_unset_change_callback
};

typedef struct {
	artik_list node;
	artik_gpio_config config;
} gpio_node;

static artik_list *requested_node = NULL;

static int check_exist(gpio_node *elem, unsigned int val_id)
{
	if (elem->config.id == val_id)
		return 1;
	return 0;
}

artik_error artik_gpio_request(artik_gpio_handle *handle,
			       artik_gpio_config *config)
{
	gpio_node *node = (gpio_node *) artik_list_get_by_check(requested_node,
			(ARTIK_LIST_FUNCB)&check_exist, (void *)(intptr_t)config->id);

	if (node)
		return E_BUSY;
	node = (gpio_node *) artik_list_add(&requested_node, 0,
						sizeof(gpio_node));
	if (!node)
		return E_NO_MEM;
	if (os_gpio_request(config) != S_OK) {
		artik_list_delete_node(&requested_node, (artik_list *) node);
		return E_BAD_ARGS;
	}
	node->node.handle = (ARTIK_LIST_HANDLE) node;
	memcpy(&node->config, config, sizeof(node->config));
	*handle = (artik_gpio_handle) node;
	return S_OK;
}

artik_error artik_gpio_release(artik_gpio_handle handle)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);
	artik_error ret;

	if (!node)
		return E_BAD_ARGS;
	ret = os_gpio_release(&node->config);
	if (ret != S_OK)
		return ret;
	artik_list_delete_node(&requested_node, (artik_list *) node);
	return S_OK;
}

int artik_gpio_read(artik_gpio_handle handle)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_gpio_read(&node->config);
}

artik_error artik_gpio_write(artik_gpio_handle handle, int value)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_gpio_write(&node->config, value);
}

char *artik_gpio_get_name(artik_gpio_handle handle)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return NULL;

	return node->config.name;
}

artik_gpio_dir_t artik_gpio_get_direction(artik_gpio_handle handle)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return GPIO_DIR_INVALID;
	return node->config.dir;
}

artik_gpio_id artik_gpio_get_id(artik_gpio_handle handle)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return -1;
	return node->config.id;
}

artik_error artik_gpio_set_change_callback(artik_gpio_handle handle,
				artik_gpio_callback callback, void *user_data)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_gpio_set_change_callback(&node->config, callback, user_data);
}

void artik_gpio_unset_change_callback(artik_gpio_handle handle)
{
	gpio_node *node =
	    (gpio_node *) artik_list_get_by_handle(requested_node,
						   (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return;

	os_gpio_unset_change_callback(&node->config);
}
