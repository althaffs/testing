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

#ifndef LPS25HBTR_H_
#define LPS25HBTR_H_

/*! \file LPS25HBTR.h
 *
 *  \brief LPS25HBTR barometer and temperature sensor
 *
 */

#include <artik_sensor.h>

#define LPS25HBTR_ADDR		0x5D

extern artik_sensor_pressure lps25hbtr_barometer_sensor;
extern artik_sensor_temperature lps25hbtr_temperature_sensor;

#endif /* LPS25HBTR_H_ */
