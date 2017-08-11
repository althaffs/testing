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


#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <artik_sensor.hh>
#include <artik_module.h>
#include <artik_platform.h>

#include <vector>

static int end = 0;

static void signal_handler(int signum) {
  end = 1;
}

int main(int argc, char *argv[]) {
  artik::Sensor sensor_module;
  std::vector<artik::SensorDevice*> tab;
  artik::AccelerometerSensor *sensor_acce = NULL;
  artik::HumiditySensor *sensor_humidity = NULL;
  artik::LightSensor *sensor_photolight = NULL;
  artik::TemperatureSensor *sensor_envtemp = NULL;
  artik::ProximitySensor *sensor_proximity = NULL;
  artik::FlameSensor *sensor_flame = NULL;
  artik::PressureSensor *sensor_pressure = NULL;
  artik::GyroSensor *sensor_gyro = NULL;
  artik::HallSensor *sensor_hall = NULL;

  signal(SIGINT, signal_handler);
  try {
    try {
      tab = sensor_module.list();
    }
    catch (artik::ArtikException error) {
      printf("list ERROR[%s]\n", error.what());
    }
    for (unsigned int i = 0; i < tab.size(); ++i) {
      printf("CONFIG[%s] : [%d]\n", tab[i]->get_name(), tab[i]->get_type());
      delete tab[i];
    }
    tab.clear();
    try {
      sensor_acce = sensor_module.get_accelerometer_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("accelerometer ERROR[%s]\n", error.what());
    }
    try {
      sensor_humidity = dynamic_cast<artik::HumiditySensor*>(
          sensor_module.get_sensor(0, ARTIK_SENSOR_HUMIDITY));
    }
    catch (artik::ArtikException error) {
      printf("humidity ERROR[%s]\n", error.what());
    }
    try {
      sensor_photolight = sensor_module.get_light_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("photo light ERROR[%s]\n", error.what());
    }
    try {
      sensor_envtemp = sensor_module.get_temperature_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("env temp ERROR[%s]\n", error.what());
    }
    try {
      sensor_proximity = sensor_module.get_proximity_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("proximity ERROR[%s]\n", error.what());
    }
    try {
      sensor_flame = sensor_module.get_flame_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("flame ERROR[%s]\n", error.what());
    }
    try {
      sensor_pressure = sensor_module.get_pressure_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("Pressure ERROR[%s]\n", error.what());
    }
    try {
      sensor_gyro = sensor_module.get_gyro_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("Gyro ERROR[%s]\n", error.what());
    }
    try {
      sensor_hall = sensor_module.get_hall_sensor(0);
    }
    catch (artik::ArtikException error) {
      printf("Hall ERROR[%s]\n", error.what());
    }

    for (int i = 0; i < 5 && !end; ++i) {
      printf("================================================"
             "===========================\n");
      try {
        if (sensor_acce)
          printf("Accelerometer type{%d} index{%d} x{%d} y{%d} z{%d}\n",
              sensor_acce->get_type(), sensor_acce->get_index(),
              sensor_acce->get_speed_x(), sensor_acce->get_speed_y(),
              sensor_acce->get_speed_z());
      } catch (artik::ArtikException error) {
        printf("AccelerometerSensor get_speed_error[%s]\n", error.what());
      }
      try {
        if (sensor_humidity)
          printf("HumiditySensor type{%d} index{%d} humidity{%d%%}\n",
              sensor_humidity->get_type(), sensor_humidity->get_index(),
              sensor_humidity->get_humidity());
      } catch (artik::ArtikException error) {
        printf("HumiditySensor get_humidity_error[%s]\n", error.what());
      }
      try {
        if (sensor_photolight)
          printf("PhotoLightSensor type{%d} index{%d} intensity{%d%%}\n",
              sensor_photolight->get_type(), sensor_photolight->get_index(),
              sensor_photolight->get_intensity());
      } catch (artik::ArtikException error) {
        printf("PhotoLightSensor get_intensity_error[%s]\n", error.what());
      }
      try {
        if (sensor_envtemp)
          printf("TempSensor type{%d} index{%d} celsius{%d°C} fahrenheit{%d°F}",
              sensor_envtemp->get_type(), sensor_envtemp->get_index(),
              sensor_envtemp->get_celsius(), sensor_envtemp->get_fahrenheit());
      } catch (artik::ArtikException error) {
        printf("TempSensor get_degree_error[%s]\n", error.what());
      }
      try {
        if (sensor_proximity)
          printf("ProxySensor type{%d} index{%d}  presence{%d}\n",
              sensor_proximity->get_type(), sensor_proximity->get_index(),
              sensor_proximity->get_presence());
      } catch (artik::ArtikException error) {
        printf("ProxySensor get_presence_error[%s]\n", error.what());
      }
      try {
        if (sensor_flame)
          printf("FlameSensor type{%d} index{%d} signals{%d}\n",
              sensor_flame->get_type(), sensor_flame->get_index(),
              sensor_flame->get_signals());
      } catch (artik::ArtikException error) {
        printf("FlameSensor get_signals_error[%s]\n", error.what());
      }
      try {
        if (sensor_pressure)
          printf("PressureSensor type{%d} index{%d} pressure{%d}\n",
              sensor_pressure->get_type(), sensor_pressure->get_index(),
              sensor_pressure->get_pressure());
      } catch (artik::ArtikException error) {
        printf("PressureSensor get_pressure error[%s]\n", error.what());
      }
      try {
        if (sensor_gyro)
          printf("GyroSensor type{%d} index{%d} yaw{%d} roll{%d} pitch{%d}\n",
              sensor_gyro->get_type(), sensor_gyro->get_index(),
              sensor_gyro->get_yaw(), sensor_gyro->get_roll(),
              sensor_gyro->get_pitch());
      } catch (artik::ArtikException error) {
        printf("FlameSensor get error[%s]\n", error.what());
      }
      try {
        if (sensor_hall)
          printf("HallSensor type{%d} index{%d}  Magnetic Detection{%d}\n",
              sensor_hall->get_type(), sensor_hall->get_index(),
              sensor_hall->get_detection());
      } catch (artik::ArtikException error) {
        printf("HallSensor get_detection error[%s]\n", error.what());
      }
      printf("================================================"
             "===========================\n");
      sleep(2);
    }
    if (sensor_acce)
      delete sensor_acce;
    if (sensor_humidity)
      delete sensor_humidity;
    if (sensor_photolight)
      delete sensor_photolight;
    if (sensor_envtemp)
      delete sensor_envtemp;
    if (sensor_proximity)
      delete sensor_proximity;
    if (sensor_flame)
      delete sensor_flame;
    if (sensor_pressure)
      delete sensor_pressure;
    if (sensor_gyro)
      delete sensor_gyro;
    if (sensor_hall)
      delete sensor_hall;
  }catch (artik::ArtikException error) {
    printf("ERROR[%s]\n", error.what());
  }
  return 0;
}
