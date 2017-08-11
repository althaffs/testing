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

#ifndef SENSOR_CPP_ARTIK_SENSOR_HH_
#define SENSOR_CPP_ARTIK_SENSOR_HH_

#include <artik_sensor.h>

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_error.hh>

#include <vector>

/*! \file artik_sensor.hh
 *  \brief C++ Wrapper to the SENSOR module
 *
 *  This is a class encapsulation of the C
 *  Sensor module API \ref artik_sensor.h
 */
namespace artik {
/*!
 *  \brief SensorDevice class
 */
class SensorDevice {
 public:
  virtual ~SensorDevice()  {;}

  virtual char *      get_name(void) const = 0;
  virtual artik_sensor_device_t get_type(void) = 0;
  virtual int     get_index(void) = 0;
  virtual void      release(void) = 0;
};

/*!
 *  \brief AccelerometerSensor class
 */
class AccelerometerSensor : public virtual SensorDevice {
 private:
  artik_sensor_accelerometer *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  AccelerometerSensor(artik_sensor_accelerometer*, artik_sensor_config *,
      artik_sensor_handle, int);

 public:
  AccelerometerSensor();
  AccelerometerSensor(AccelerometerSensor const &);
  virtual ~AccelerometerSensor();

  AccelerometerSensor &operator=(AccelerometerSensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_speed_x(void) const;
  int get_speed_y(void) const;
  int get_speed_z(void) const;

  friend class Sensor;
};

/*!
 *  \brief HumiditySensor class
 */
class HumiditySensor : public virtual SensorDevice {
 private:
  artik_sensor_humidity *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  HumiditySensor(artik_sensor_humidity*, artik_sensor_config *,
      artik_sensor_handle, int);

 public:
  HumiditySensor();
  HumiditySensor(HumiditySensor const &);
  virtual ~HumiditySensor();

  HumiditySensor &operator=(HumiditySensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_humidity(void) const;

  friend class Sensor;
};

/*!
 *  \brief LightSensor class
 */
class LightSensor : public virtual SensorDevice {
 private:
  artik_sensor_light *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  LightSensor(artik_sensor_light*, artik_sensor_config *, artik_sensor_handle,
      int);

 public:
  LightSensor();
  LightSensor(LightSensor const &);
  virtual ~LightSensor();

  LightSensor &operator=(LightSensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_intensity(void) const;

  friend class Sensor;
};

/*!
 *  \brief TemperatureSensor class
 */
class TemperatureSensor : public virtual SensorDevice {
 private:
  artik_sensor_temperature *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  TemperatureSensor(artik_sensor_temperature*, artik_sensor_config *,
      artik_sensor_handle, int);

 public:
  TemperatureSensor();
  TemperatureSensor(TemperatureSensor const &);
  virtual ~TemperatureSensor();

  TemperatureSensor &operator=(TemperatureSensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_celsius(void) const;
  int get_fahrenheit(void) const;

  friend class Sensor;
};

/*!
 *  \brief ProximitySensor class
 */
class ProximitySensor : public virtual SensorDevice {
 private:
  artik_sensor_proximity *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  ProximitySensor(artik_sensor_proximity*, artik_sensor_config *,
      artik_sensor_handle, int);

 public:
  ProximitySensor();
  ProximitySensor(ProximitySensor const &);
  virtual ~ProximitySensor();

  ProximitySensor &operator=(ProximitySensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_presence(void) const;

  friend class Sensor;
};

/*!
 *  \brief FlameSensor class
 */
class FlameSensor : public virtual SensorDevice {
 private:
  artik_sensor_flame *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  FlameSensor(artik_sensor_flame*, artik_sensor_config *, artik_sensor_handle,
      int);

 public:
  FlameSensor();
  FlameSensor(FlameSensor const &);
  virtual ~FlameSensor();

  FlameSensor &operator=(FlameSensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_signals(void) const;

  friend class Sensor;
};

/*!
 *  \brief PressureSensor class
 */
class PressureSensor : public virtual SensorDevice {
 private:
  artik_sensor_pressure *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  PressureSensor(artik_sensor_pressure*, artik_sensor_config *,
      artik_sensor_handle, int);

 public:
  PressureSensor();
  PressureSensor(PressureSensor const &);
  virtual ~PressureSensor();

  PressureSensor &operator=(PressureSensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_pressure(void) const;

  friend class Sensor;
};

/*!
 *  \brief GyroSensor class
 */
class GyroSensor : public virtual SensorDevice {
 private:
  artik_sensor_gyro *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  GyroSensor(artik_sensor_gyro*, artik_sensor_config *, artik_sensor_handle,
      int);

 public:
  GyroSensor();
  GyroSensor(GyroSensor const &);
  virtual ~GyroSensor();

  GyroSensor &operator=(GyroSensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_yaw(void) const;
  int get_roll(void) const;
  int get_pitch(void) const;

  friend class Sensor;
};

/*!
 *  \brief HallSensor class
 */
class HallSensor : public virtual SensorDevice {
 private:
  artik_sensor_hall *m_sensor;
  artik_sensor_config *m_config;
  artik_sensor_handle m_handle;
  int m_index;

 protected:
  HallSensor(artik_sensor_hall*, artik_sensor_config *, artik_sensor_handle,
      int);

 public:
  HallSensor();
  HallSensor(HallSensor const &);
  virtual ~HallSensor();

  HallSensor &operator=(HallSensor const&);

  virtual char *get_name(void) const;
  virtual artik_sensor_device_t get_type(void);
  virtual int get_index(void);
  virtual void release(void);

  int get_detection(void) const;

  friend class Sensor;
};

/*!
 *  \brief Sensor module C++ Class
 */
class Sensor {
 private:
  artik_sensor_module* m_module;

 public:
  Sensor(Sensor const &);
  Sensor();
  ~Sensor();

  Sensor &operator=(Sensor const &);

 public:
  std::vector<SensorDevice*> list(void);
  SensorDevice *get_sensor(int index, artik_sensor_device_t type);
  AccelerometerSensor *get_accelerometer_sensor(int index);
  HumiditySensor *get_humidity_sensor(int index);
  LightSensor *get_light_sensor(int index);
  TemperatureSensor *get_temperature_sensor(int index);
  ProximitySensor *get_proximity_sensor(int index);
  FlameSensor *get_flame_sensor(int index);
  PressureSensor *get_pressure_sensor(int index);
  GyroSensor *get_gyro_sensor(int index);
  HallSensor *get_hall_sensor(int index);
};

}  // namespace artik

#endif  // SENSOR_CPP_ARTIK_SENSOR_HH_
