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

#ifndef HTS221_H_
#define HTS221_H_

/*! \file HTS221.h
 *
 *  \brief HTS221 temperature and humidity sensor
 *
 */

#include <artik_sensor.h>

#define	HTS221_ADDR	0x5F

extern artik_sensor_humidity hts221_humidity_sensor;
extern artik_sensor_temperature hts221_temp_sensor;


#endif /* HTS221_H_ */
