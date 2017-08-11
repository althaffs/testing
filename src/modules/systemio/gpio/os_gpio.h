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

#ifndef SRC_GPIO_OS_GPIO_H_
#define SRC_GPIO_OS_GPIO_H_

#include "artik_error.h"

artik_error os_gpio_request(artik_gpio_config *config);
artik_error os_gpio_release(artik_gpio_config *config);
int         os_gpio_read(artik_gpio_config *config);
artik_error os_gpio_write(artik_gpio_config *config, int value);
artik_error	os_gpio_set_change_callback(artik_gpio_config *config,
				artik_gpio_callback callback, void *user_data);
void	os_gpio_unset_change_callback(artik_gpio_config *config);

#endif /* SRC_GPIO_OS_GPIO_H_ */
