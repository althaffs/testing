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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include	"artik_pwm.h"
#include	"artik_list.h"
#include	"os_pwm.h"

static artik_error artik_pwm_request(artik_pwm_handle * handle,
					artik_pwm_config * config);
static artik_error artik_pwm_release(artik_pwm_handle handle);
static artik_error artik_pwm_enable(artik_pwm_handle handle);
static artik_error artik_pwm_disable(artik_pwm_handle handle);
static artik_error artik_pwm_set_period(artik_pwm_handle handle,
					unsigned int value);
static artik_error artik_pwm_set_polarity(artik_pwm_handle handle,
					artik_pwm_polarity_t value);
static artik_error artik_pwm_set_duty_cycle(artik_pwm_handle handle,
					unsigned int value);

artik_pwm_module pwm_module = {
	artik_pwm_request,
	artik_pwm_release,
	artik_pwm_enable,
	artik_pwm_disable,
	artik_pwm_set_period,
	artik_pwm_set_polarity,
	artik_pwm_set_duty_cycle
};

typedef struct {
	artik_list node;
	artik_pwm_config config;
} pwm_node;

static artik_list *requested_node = NULL;

static int check_exist(pwm_node *elem, int val_pin)
{
	if (elem->config.pin_num == val_pin)
		return 1;
	return 0;
}

artik_error artik_pwm_request(artik_pwm_handle *handle,
				artik_pwm_config *config)
{
	pwm_node *node = (pwm_node *)artik_list_get_by_check(requested_node,
		(ARTIK_LIST_FUNCB)&check_exist, (void *)(intptr_t)config->pin_num);

	artik_error ret = S_OK;

	if (node)
		return E_BUSY;
	node = (pwm_node *) artik_list_add(&requested_node, 0,
							sizeof(pwm_node));
	if (!node) {
		/* node no memory to consume */
		return E_NO_MEM;
	}
	ret = os_pwm_request(config);
	if (ret == S_OK) {
		node->node.handle = (ARTIK_LIST_HANDLE) node;
		memcpy(&node->config, config, sizeof(node->config));
		*handle = (artik_pwm_handle)node;
	} else {
		/* node request failed */
		artik_list_delete_node(&requested_node, (artik_list *)node);
	}
	return ret;
}

artik_error artik_pwm_release(artik_pwm_handle handle)
{
	pwm_node *node = (pwm_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);
	artik_error ret = S_OK;

	if (!node)
		return E_BAD_ARGS;
	ret = os_pwm_release(&node->config);
	if (ret != S_OK)
		return ret;
	artik_list_delete_node(&requested_node, (artik_list *)node);
	return ret;
}

artik_error artik_pwm_enable(artik_pwm_handle handle)
{
	pwm_node *node = (pwm_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_pwm_enable(&node->config, '1');
}

artik_error artik_pwm_disable(artik_pwm_handle handle)
{
	pwm_node *node = (pwm_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_pwm_enable(&node->config, '0');
}

artik_error artik_pwm_set_period(artik_pwm_handle handle, unsigned int value)
{
	pwm_node *node = (pwm_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_pwm_set_period(&node->config, value);
}

artik_error artik_pwm_set_polarity(artik_pwm_handle handle,
					artik_pwm_polarity_t value)
{
	pwm_node *node = (pwm_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_pwm_set_polarity(&node->config, value);
}

artik_error artik_pwm_set_duty_cycle(artik_pwm_handle handle,
					unsigned int value)
{
	pwm_node *node = (pwm_node *)artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;

	return os_pwm_set_duty_cycle(&node->config, value);
}
