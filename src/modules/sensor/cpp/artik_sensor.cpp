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

#include "artik_sensor.hh"

#include <iostream>
#include <vector>

artik::AccelerometerSensor::AccelerometerSensor(
    artik_sensor_accelerometer*sensor, artik_sensor_config *config,
    artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::AccelerometerSensor::AccelerometerSensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::AccelerometerSensor::AccelerometerSensor(
    artik::AccelerometerSensor const &val) : artik::SensorDevice() {
  *this = val;
}

artik::AccelerometerSensor::~AccelerometerSensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::AccelerometerSensor
&artik::AccelerometerSensor::operator=(artik::AccelerometerSensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::AccelerometerSensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::AccelerometerSensor::get_type(void) {
  return ARTIK_SENSOR_ACCELEROMETER;
}

int artik::AccelerometerSensor::get_index(void) {
  return this->m_index;
}

void artik::AccelerometerSensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::AccelerometerSensor::get_speed_x(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_speed_x(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

int artik::AccelerometerSensor::get_speed_y(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_speed_y(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

int artik::AccelerometerSensor::get_speed_z(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_speed_z(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::GyroSensor::GyroSensor(artik_sensor_gyro*sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::GyroSensor::GyroSensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::GyroSensor::GyroSensor(artik::GyroSensor const &val)
  : artik::SensorDevice() {
  *this = val;
}

artik::GyroSensor::~GyroSensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::GyroSensor &artik::GyroSensor::operator=(artik::GyroSensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char * artik::GyroSensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::GyroSensor::get_type(void) {
  return ARTIK_SENSOR_GYRO;
}

int artik::GyroSensor::get_index(void) {
  return this->m_index;
}

void artik::GyroSensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::GyroSensor::get_yaw(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_yaw(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

int artik::GyroSensor::get_pitch(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_pitch(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

int artik::GyroSensor::get_roll(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_roll(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::HumiditySensor::HumiditySensor(artik_sensor_humidity*sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::HumiditySensor::HumiditySensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::HumiditySensor::HumiditySensor(artik::HumiditySensor const &val)
  : artik::SensorDevice() {
  *this = val;
}

artik::HumiditySensor::~HumiditySensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::HumiditySensor &artik::HumiditySensor::operator=(
    artik::HumiditySensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::HumiditySensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::HumiditySensor::get_type(void) {
  return ARTIK_SENSOR_HUMIDITY;
}

int artik::HumiditySensor::get_index(void) {
  return this->m_index;
}

void artik::HumiditySensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::HumiditySensor::get_humidity(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_humidity(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::LightSensor::LightSensor(artik_sensor_light *sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::LightSensor::LightSensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::LightSensor::LightSensor(artik::LightSensor const &val)
  : artik::SensorDevice() {
  *this = val;
}

artik::LightSensor::~LightSensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::LightSensor &artik::LightSensor::operator=(
    artik::LightSensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::LightSensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::LightSensor::get_type(void) {
  return ARTIK_SENSOR_LIGHT;
}

int artik::LightSensor::get_index(void) {
  return this->m_index;
}

void artik::LightSensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::LightSensor::get_intensity(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_intensity(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::TemperatureSensor::TemperatureSensor(artik_sensor_temperature*sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::TemperatureSensor::TemperatureSensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::TemperatureSensor::TemperatureSensor(
    artik::TemperatureSensor const &val) : artik::SensorDevice() {
  *this = val;
}

artik::TemperatureSensor::~TemperatureSensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::TemperatureSensor &artik::TemperatureSensor::operator=(
    artik::TemperatureSensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::TemperatureSensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::TemperatureSensor::get_type(void) {
  return ARTIK_SENSOR_TEMPERATURE;
}

int artik::TemperatureSensor::get_index(void) {
  return this->m_index;
}

void artik::TemperatureSensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::TemperatureSensor::get_celsius(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_celsius(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

int artik::TemperatureSensor::get_fahrenheit(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_fahrenheit(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::ProximitySensor::ProximitySensor(artik_sensor_proximity*sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::ProximitySensor::ProximitySensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::ProximitySensor::ProximitySensor(artik::ProximitySensor const &val)
  : artik::SensorDevice() {
  *this = val;
}

artik::ProximitySensor::~ProximitySensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::ProximitySensor &artik::ProximitySensor::operator=(
    artik::ProximitySensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::ProximitySensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::ProximitySensor::get_type(void) {
  return ARTIK_SENSOR_PROXIMITY;
}

int artik::ProximitySensor::get_index(void) {
  return this->m_index;
}

void artik::ProximitySensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::ProximitySensor::get_presence(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_presence(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::FlameSensor::FlameSensor(artik_sensor_flame*sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::FlameSensor::FlameSensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::FlameSensor::FlameSensor(artik::FlameSensor const &val)
  : artik::SensorDevice() {
  *this = val;
}

artik::FlameSensor::~FlameSensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::FlameSensor &artik::FlameSensor::operator=(
    artik::FlameSensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::FlameSensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::FlameSensor::get_type(void) {
  return ARTIK_SENSOR_FLAME;
}

int artik::FlameSensor::get_index(void) {
  return this->m_index;
}

void artik::FlameSensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::FlameSensor::get_signals(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_signals(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::PressureSensor::PressureSensor(artik_sensor_pressure* sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::PressureSensor::PressureSensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::PressureSensor::PressureSensor(artik::PressureSensor const &val)
  : artik::SensorDevice() {
  *this = val;
}

artik::PressureSensor::~PressureSensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::PressureSensor &artik::PressureSensor::operator=(
    artik::PressureSensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::PressureSensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::PressureSensor::get_type(void) {
  return ARTIK_SENSOR_BAROMETER;
}

int artik::PressureSensor::get_index(void) {
  return this->m_index;
}

void artik::PressureSensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::PressureSensor::get_pressure(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_pressure(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::HallSensor::HallSensor(artik_sensor_hall* sensor,
    artik_sensor_config *config, artik_sensor_handle handle, int index)
  : artik::SensorDevice(),
    m_sensor(sensor),
    m_config(config),
    m_handle(handle),
    m_index(index) {
}

artik::HallSensor::HallSensor()
  : artik::SensorDevice(),
    m_sensor(NULL),
    m_config(NULL),
    m_handle(NULL),
    m_index(-1) {
}

artik::HallSensor::HallSensor(artik::HallSensor const &val)
  : artik::SensorDevice() {
  *this = val;
}

artik::HallSensor::~HallSensor() {
  if (this->m_sensor)
    this->m_sensor->release(this->m_handle);
}

artik::HallSensor &artik::HallSensor::operator=(artik::HallSensor const &val) {
  this->m_sensor = val.m_sensor;
  this->m_config = val.m_config;
  this->m_handle = val.m_handle;
  this->m_index = val.m_index;
  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  return *this;
}

char *artik::HallSensor::get_name(void) const {
  return this->m_config->name;
}

artik_sensor_device_t artik::HallSensor::get_type(void) {
  return ARTIK_SENSOR_HALL;
}

int artik::HallSensor::get_index(void) {
  return this->m_index;
}

void artik::HallSensor::release(void) {
  if (this->m_sensor && this->m_handle)
    this->m_sensor->release(this->m_handle);
}

int artik::HallSensor::get_detection(void) const {
  int data = 0;

  if (!this->m_sensor || !this->m_config || !this->m_handle)
    artik_throw(artik::ArtikInitException());
  if (this->m_sensor->get_detection(this->m_handle, &data) != S_OK)
    artik_throw(artik::ArtikBadValException());
  return data;
}

artik::Sensor::Sensor(artik::Sensor const &val) {
  (*this) = val;
}

artik::Sensor::Sensor() {
  m_module = reinterpret_cast<artik_sensor_module*>(
      artik_request_api_module("sensor"));

  if (!this->m_module)
    artik_throw(artik::ArtikInitException());
}

artik::Sensor::~Sensor() {
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik::Sensor &artik::Sensor::operator=(Sensor const &val) {
  this->m_module = val.m_module;
  if (!this->m_module)
    artik_throw(artik::ArtikInitException());
  return *this;
}

std::vector<artik::SensorDevice*> artik::Sensor::list(void) {
  artik_sensor_config *array_conf = this->m_module->list();
  std::vector<artik::SensorDevice*> devices;
  int len = 0;

  for (; array_conf && array_conf[len].type; len++) {}

  if (!len)
    artik_throw(artik::ArtikInitException());

  for (int ref = 0, i = 0; i < len; i++) {
    switch (array_conf[i].type) {
    case ARTIK_SENSOR_ACCELEROMETER :
      devices.push_back(new artik::AccelerometerSensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_HUMIDITY :
      devices.push_back(new artik::HumiditySensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_LIGHT :
      devices.push_back(new artik::LightSensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_TEMPERATURE:
      devices.push_back(new artik::TemperatureSensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_PROXIMITY:
      devices.push_back(new artik::ProximitySensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_FLAME:
      devices.push_back(new artik::FlameSensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_BAROMETER:
      devices.push_back(new artik::PressureSensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_GYRO:
      devices.push_back(new artik::GyroSensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_HALL:
      devices.push_back(new artik::HallSensor(NULL, &array_conf[i],
            NULL, ref));
      break;
    case ARTIK_SENSOR_NONE:
      break;
    }
    ref = (array_conf[i].type != array_conf[i+1].type ? 0 : ref+1);
  }
  return devices;
}

artik::SensorDevice *artik::Sensor::get_sensor(int index,
    artik_sensor_device_t type) {

  switch (type) {
  case ARTIK_SENSOR_ACCELEROMETER :
    return this->get_accelerometer_sensor(index);
  case ARTIK_SENSOR_HUMIDITY :
    return this->get_humidity_sensor(index);
  case ARTIK_SENSOR_LIGHT :
    return this->get_light_sensor(index);
  case ARTIK_SENSOR_TEMPERATURE:
    return this->get_temperature_sensor(index);
  case ARTIK_SENSOR_PROXIMITY:
    return this->get_proximity_sensor(index);
  case ARTIK_SENSOR_FLAME:
    return this->get_flame_sensor(index);
  case ARTIK_SENSOR_BAROMETER:
    return this->get_pressure_sensor(index);
  case ARTIK_SENSOR_GYRO:
    return this->get_gyro_sensor(index);
  case ARTIK_SENSOR_HALL:
    return this->get_hall_sensor(index);
  case ARTIK_SENSOR_NONE:
    break;
  }
  artik_throw(artik::ArtikBadArgsException());
}

artik::AccelerometerSensor *artik::Sensor::get_accelerometer_sensor(int index) {
  artik_sensor_config *accelerometer_conf =
    this->m_module->get_accelerometer_sensor(index);
  artik_sensor_accelerometer *accelerometer_sensor = NULL;
  artik_sensor_handle accelerometer_handle = NULL;
  artik_error res = S_OK;

  if (!accelerometer_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));

  if ((res = this->m_module->request(accelerometer_conf, &accelerometer_handle,
      reinterpret_cast<artik_sensor_ops*>(&accelerometer_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::AccelerometerSensor(accelerometer_sensor,
        accelerometer_conf, accelerometer_handle, index));
}

artik::HumiditySensor *artik::Sensor::get_humidity_sensor(int index) {
  artik_sensor_config *humidity_conf =
    this->m_module->get_humidity_sensor(index);
  artik_sensor_humidity *humidity_sensor = NULL;
  artik_sensor_handle humidity_handle = NULL;
  artik_error   res = S_OK;

  if (!humidity_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(humidity_conf, &humidity_handle,
      reinterpret_cast<artik_sensor_ops*>(&humidity_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::HumiditySensor(humidity_sensor, humidity_conf,
        humidity_handle, index));
}

artik::LightSensor *artik::Sensor::get_light_sensor(int index) {
  artik_sensor_config *light_conf = this->m_module->get_light_sensor(index);
  artik_sensor_light  *light_sensor = NULL;
  artik_sensor_handle light_handle = NULL;
  artik_error res = S_OK;

  if (!light_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(light_conf, &light_handle,
      reinterpret_cast<artik_sensor_ops*>(&light_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::LightSensor(light_sensor, light_conf, light_handle,
        index));
}

artik::TemperatureSensor  *artik::Sensor::get_temperature_sensor(int index) {
  artik_sensor_config *temperature_conf =
    this->m_module->get_temperature_sensor(index);
  artik_sensor_temperature *temperature_sensor = NULL;
  artik_sensor_handle temperature_handle = NULL;
  artik_error res = S_OK;

  if (!temperature_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(temperature_conf, &temperature_handle,
      reinterpret_cast<artik_sensor_ops*>(&temperature_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::TemperatureSensor(temperature_sensor, temperature_conf,
        temperature_handle, index));
}

artik::ProximitySensor *artik::Sensor::get_proximity_sensor(int index) {
  artik_sensor_config *proximity_conf =
    this->m_module->get_proximity_sensor(index);
  artik_sensor_proximity *proximity_sensor = NULL;
  artik_sensor_handle proximity_handle = NULL;
  artik_error res = S_OK;

  if (!proximity_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(proximity_conf, &proximity_handle,
      reinterpret_cast<artik_sensor_ops*>(&proximity_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::ProximitySensor(proximity_sensor, proximity_conf,
      proximity_handle, index));
}

artik::FlameSensor *artik::Sensor::get_flame_sensor(int index) {
  artik_sensor_config *flame_conf = this->m_module->get_flame_sensor(index);
  artik_sensor_flame *flame_sensor = NULL;
  artik_sensor_handle flame_handle = NULL;
  artik_error res = S_OK;

  if (!flame_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(flame_conf, &flame_handle,
      reinterpret_cast<artik_sensor_ops*>(&flame_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::FlameSensor(flame_sensor, flame_conf, flame_handle,
        index));
}

artik::PressureSensor *artik::Sensor::get_pressure_sensor(int index) {
  artik_sensor_config *pressure_conf =
    this->m_module->get_pressure_sensor(index);
  artik_sensor_pressure *pressure_sensor = NULL;
  artik_sensor_handle pressure_handle  = NULL;
  artik_error res = S_OK;

  if (!pressure_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(pressure_conf, &pressure_handle,
      reinterpret_cast<artik_sensor_ops*>(&pressure_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::PressureSensor(pressure_sensor, pressure_conf,
      pressure_handle, index));
}
artik::GyroSensor *artik::Sensor::get_gyro_sensor(int index) {
  artik_sensor_config *gyro_conf  = this->m_module->get_gyro_sensor(index);
  artik_sensor_gyro *gyro_sensor = NULL;
  artik_sensor_handle gyro_handle = NULL;
  artik_error   res = S_OK;

  if (!gyro_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(gyro_conf, &gyro_handle,
      reinterpret_cast<artik_sensor_ops*>(&gyro_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::GyroSensor(gyro_sensor, gyro_conf, gyro_handle, index));
}

artik::HallSensor *artik::Sensor::get_hall_sensor(int index) {
  artik_sensor_config *hall_conf = this->m_module->get_hall_sensor(index);
  artik_sensor_hall *hall_sensor = NULL;
  artik_sensor_handle hall_handle = NULL;
  artik_error res = S_OK;

  if (!hall_conf)
    artik_throw(artik::ArtikException(E_NOT_SUPPORTED));
  if ((res = this->m_module->request(hall_conf, &hall_handle,
      reinterpret_cast<artik_sensor_ops*>(&hall_sensor))) != S_OK)
    artik_throw(artik::ArtikException(res));

  return (new artik::HallSensor(hall_sensor, hall_conf, hall_handle, index));
}
