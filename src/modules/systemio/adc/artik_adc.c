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

#include	<stdlib.h>
#include	<string.h>
#include	<stdint.h>

#include	"artik_adc.h"
#include	"os_adc.h"

static artik_error artik_adc_request(artik_adc_handle * handle,
				     artik_adc_config * config);
static artik_error artik_adc_release(artik_adc_handle handle);
static artik_error artik_adc_get_value(artik_adc_handle handle, int *value);

artik_adc_module adc_module = {
	artik_adc_request,
	artik_adc_release,
	artik_adc_get_value
};

typedef struct {
	artik_list node;
	artik_adc_config config;

} adc_node;

static artik_list *requested_node = NULL;

static int check_exist(adc_node *elem, int val_pin)
{
	if (elem->config.pin_num == val_pin)
		return 1;
	return 0;
}

static artik_error artik_adc_request(artik_adc_handle *handle,
				     artik_adc_config *config)
{
	adc_node *node =
	    (adc_node *) artik_list_get_by_check(requested_node,
						 (ARTIK_LIST_FUNCB) &
						 check_exist,
						 (void *)((intptr_t)config->pin_num));
	artik_error res = S_OK;

	if (node)
		return E_BUSY;
	res = os_adc_request(config);
	if (res != S_OK)
		return res;
	node =
	    (adc_node *) artik_list_add(&requested_node, 0, sizeof(adc_node));
	node->node.handle = (ARTIK_LIST_HANDLE) node;
	memcpy(&node->config, config, sizeof(node->config));
	*handle = (artik_adc_handle) node;
	return S_OK;
}

static artik_error artik_adc_release(artik_adc_handle handle)
{
	adc_node *node = (adc_node *) artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);
	artik_error ret = S_OK;

	if (!node)
		return E_BAD_ARGS;
	ret = os_adc_release(&node->config);
	if (ret == S_OK)
		artik_list_delete_node(&requested_node, (artik_list *) node);
	return ret;
}

static artik_error artik_adc_get_value(artik_adc_handle handle, int *value)
{
	adc_node *node = (adc_node *) artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	return !node ? E_BAD_ARGS : os_adc_get_value(&node->config, value);
}
