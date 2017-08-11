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

#ifndef	__OS_PWM_H__
#define	__OS_PWM_H__

#include "artik_error.h"

artik_error os_pwm_request(artik_pwm_config *config);
artik_error os_pwm_release(artik_pwm_config *config);
artik_error os_pwm_enable(artik_pwm_config *config, char value);
artik_error os_pwm_set_period(artik_pwm_config *config, unsigned int value);
artik_error os_pwm_set_polarity(artik_pwm_config *config,
				artik_pwm_polarity_t value);
artik_error os_pwm_set_duty_cycle(artik_pwm_config *config, unsigned int value);

#endif  /* __OS_PWM_H__ */
