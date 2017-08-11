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

#include "artik_spi.h"
#include "artik_list.h"
#include "os_spi.h"

static artik_error artik_spi_request(artik_spi_handle * handle,
				     artik_spi_config * config);
static artik_error artik_spi_release(artik_spi_handle handle);
static artik_error artik_spi_read(artik_spi_handle handle, char *buf, int len);
static artik_error artik_spi_write(artik_spi_handle handle, char *buf, int len);
static artik_error artik_spi_read_write(artik_spi_handle handle,
					   char *tx_buf, char *rx_buf, int len);

const artik_spi_module spi_module = {
	artik_spi_request,
	artik_spi_release,
	artik_spi_read,
	artik_spi_write,
	artik_spi_read_write,
};

typedef struct {
	artik_list node;
	artik_spi_config config;
} spi_node;

static artik_list *requested_node = NULL;

static int check_exist(spi_node *elem, unsigned int val_bus)
{
	if (elem->config.bus == val_bus)
		return 1;
	return 0;
}

artik_error artik_spi_request(artik_spi_handle *handle,
			      artik_spi_config *config)
{
	spi_node *node = (spi_node *)artik_list_get_by_check(requested_node,
			(ARTIK_LIST_FUNCB)&check_exist, (void *)(intptr_t)config->bus);
	artik_error ret = S_OK;

	if (node)
		return E_BUSY;
	node = (spi_node *) artik_list_add(&requested_node, 0,
							sizeof(spi_node));
	if (!node) {
		/* node memory to consume */
		return E_NO_MEM;
	}
	ret = os_spi_request(config);
	if (ret == S_OK) {
		node->node.handle = (ARTIK_LIST_HANDLE) node;
		memcpy(&node->config, config, sizeof(node->config));
		*handle = (artik_spi_handle)node;
	} else {
		/* node request failed */
		artik_list_delete_node(&requested_node, (artik_list *)node);
	}
	return ret;
}

artik_error artik_spi_release(artik_spi_handle handle)
{
	spi_node *node = (spi_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);
	artik_error ret = S_OK;

	if (!node)
		return E_BAD_ARGS;
	ret = os_spi_release(&node->config);
	if (ret != S_OK)
		return ret;
	artik_list_delete_node(&requested_node, (artik_list *)node);
	return ret;
}

artik_error artik_spi_read(artik_spi_handle handle, char *buf, int len)
{
	spi_node *node = (spi_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_spi_read(&node->config, buf, len);
}

artik_error artik_spi_write(artik_spi_handle handle, char *buf, int len)
{
	spi_node *node = (spi_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_spi_write(&node->config, buf, len);
}

artik_error artik_spi_read_write(artik_spi_handle handle, char *tx_buf,
				    char *rx_buf, int len)
{
	spi_node *node = (spi_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_spi_read_write(&node->config, tx_buf, rx_buf, len);
}

