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

#ifndef SRC_I2C_OS_GPIO_H_
#define SRC_I2C_OS_GPIO_H_

#include "artik_error.h"

artik_error os_i2c_request(artik_i2c_config *config);
artik_error os_i2c_release(artik_i2c_config *config);
artik_error os_i2c_read(artik_i2c_config *config, char *buf, int len);
artik_error os_i2c_write(artik_i2c_config *config, char *buf, int len);
artik_error os_i2c_read_register(artik_i2c_config *config, unsigned int reg,
				char *buf, int len);
artik_error os_i2c_write_register(artik_i2c_config *config, unsigned int reg,
				char *buf, int len);

#endif /* SRC_I2C_OS_GPIO_H_ */
