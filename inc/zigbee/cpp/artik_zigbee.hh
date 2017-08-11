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

#ifndef ZIGBEE_CPP_ARTIK_ZIGBEE_HH_
#define ZIGBEE_CPP_ARTIK_ZIGBEE_HH_

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_zigbee.h>

#include <list>

/*! \file artik_zigbee.hh
 *
 *  \brief C++ Wrapper to the ZigBee module
 *
 *  This is a class encapsulation of the C
 *  ZigBee module API \ref artik_zigbee.h
 */

namespace artik {
/*!
 *  \brief ZigbeeDevice C++ Class
 */
class ZigbeeDevice {
 protected:
  artik_zigbee_endpoint_handle m_handle;
  int m_endpoint_id;
  int m_profile_id;
  int m_device_id;

 public:
  artik_zigbee_endpoint_handle get_handle();
  void set_handle(artik_zigbee_endpoint_handle handle);
  int get_profile_id();
  int get_endpoint_id();
  int get_device_id();
};

/*!
 *  \brief OnOffLightDevice C++ Class
 */
class OnOffLightDevice : public ZigbeeDevice {
 private:
  artik_zigbee_device_on_off_light m_device;

 public:
  explicit OnOffLightDevice(int endpoint_id);

  artik_error groups_get_local_name_support(int endpoint);
  artik_error groups_set_local_name_support(bool support);
  artik_error onoff_get_value(artik_zigbee_onoff_status *status);
  artik_error ezmode_commissioning_target_start();
  artik_error ezmode_commissioning_target_stop();
};

/*!
 *  \brief OnOffSwitchDevice C++ Class
 */
class OnOffSwitchDevice : public ZigbeeDevice {
 private:
  artik_zigbee_device_on_off_switch m_device;

 public:
  explicit OnOffSwitchDevice(int endpoint_id);

  artik_error identify_request(const artik_zigbee_endpoint *endpoint,
      int duration);
  artik_error identify_get_remaining_time(const artik_zigbee_endpoint *endpoint,
      int *time);
  artik_error onoff_command(const artik_zigbee_endpoint *endpoint,
      artik_zigbee_onoff_status target_status);
  artik_error ezmode_commissioning_initiator_start();
  artik_error ezmode_commissioning_initiator_stop();
};

/*!
 *  \brief LevelControlSwitchDevice C++ Class
 */
class LevelControlSwitchDevice : public ZigbeeDevice {
 private:
  artik_zigbee_device_level_control_switch m_device;

 public:
  explicit LevelControlSwitchDevice(int endpoint_id);

  artik_error identify_request(const artik_zigbee_endpoint *endpoint,
      int duration);
  artik_error identify_get_remaining_time(const artik_zigbee_endpoint *endpoint,
      int *time);
  artik_error onoff_command(const artik_zigbee_endpoint *endpoint,
      artik_zigbee_onoff_status target_status);
  artik_error level_control_request(const artik_zigbee_endpoint *endpoint,
      const artik_zigbee_level_control_command *command);
  artik_error ezmode_commissioning_initiator_start();
  artik_error ezmode_commissioning_initiator_stop();
};

/*!
 *  \brief DimmableLightDevice C++ Class
 */
class DimmableLightDevice : public ZigbeeDevice {
 private:
  artik_zigbee_device_dimmable_light m_device;

 public:
  explicit DimmableLightDevice(int endpoint_id);

  artik_error groups_get_local_name_support(int endpoint);
  artik_error groups_set_local_name_support(bool support);
  artik_error onoff_get_value(artik_zigbee_onoff_status *status);
  artik_error level_control_get_value(int *value);
  artik_error ezmode_commissioning_target_start();
  artik_error ezmode_commissioning_target_stop();
};

/*!
 *  \brief LightSensorDevice C++ Class
 */
class LightSensorDevice : public ZigbeeDevice {
 private:
  artik_zigbee_device_light_sensor m_device;

 public:
  explicit LightSensorDevice(int endpoint_id);

  artik_error identify_request(const artik_zigbee_endpoint *endpoint,
      int duration);
  artik_error identify_get_remaining_time(const artik_zigbee_endpoint *endpoint,
      int *time);
  artik_error illum_set_measured_value_range(int min, int max);
  artik_error illum_set_measured_value(int value);
  artik_error illum_get_measured_value(int* value);
  artik_error ezmode_commissioning_initiator_start();
  artik_error ezmode_commissioning_initiator_stop();
};

/*!
 *  \brief RemoteControlDevice C++ Class
 */
class RemoteControlDevice : public ZigbeeDevice {
 private:
  artik_zigbee_device_remote_control m_device;

 public:
  explicit RemoteControlDevice(int endpoint_id);

  artik_error reset_to_factory_default(const artik_zigbee_endpoint *endpoint);
  artik_error identify_request(const artik_zigbee_endpoint *endpoint,
      int duration);
  artik_error identify_get_remaining_time(const artik_zigbee_endpoint *endpoint,
      int *time);
  artik_error onoff_command(const artik_zigbee_endpoint *endpoint,
      artik_zigbee_onoff_status target_status);
  artik_error level_control_request(const artik_zigbee_endpoint *endpoint,
      const artik_zigbee_level_control_command *command);
  artik_error request_reporting(const artik_zigbee_endpoint *endpoint,
      artik_zigbee_reporting_type report_type, int min_interval,
      int max_interval, float change_threshold);
  artik_error stop_reporting(const artik_zigbee_endpoint *endpoint,
      artik_zigbee_reporting_type report_type);
  artik_error ezmode_commissioning_target_start();
  artik_error ezmode_commissioning_target_stop();
};

/*!
 *  \brief Zigbee module C++ Class
 */
class Zigbee {
 private:
  artik_zigbee_module* m_module;
  std::list<ZigbeeDevice*> device_list;

 public:
  Zigbee();
  ~Zigbee();

  void initialize(artik_zigbee_client_callback callback, void* user_data);
  artik_error network_start(artik_zigbee_network_state *state);
  void network_form();
  artik_error network_form_manually(
      const artik_zigbee_network_info *network_info);
  artik_error network_permitjoin(int duration_sec);
  artik_error network_leave();
  void network_join();
  void network_find();
  artik_error network_request_my_network_status(
      artik_zigbee_network_state *state);
  artik_error device_request_my_node_type(artik_zigbee_node_type *type);
  void device_find_by_cluster(artik_zigbee_endpoint_list *endpoints,
      int cluster_id, int is_server);
  artik_error reset_local();
  artik_error network_stop_scan();
  artik_error network_join_manually(
      const artik_zigbee_network_info *network_info);
  artik_error device_discover();
  artik_error set_discover_cycle_time(unsigned int time_minutes);
  artik_error get_discovered_device_list(artik_zigbee_device_info *device_info);
  void raw_request(const char *command);
  OnOffLightDevice* get_onofflight_device(int endpoint_id);
  OnOffSwitchDevice* get_onoffswitch_device(int endpoint_id);
  LevelControlSwitchDevice* get_levelcontrolswitch_device(int endpoint_id);
  DimmableLightDevice* get_dimmablelight_device(int endpoint_id);
  LightSensorDevice* get_lightsensor_device(int endpoint_id);
  RemoteControlDevice* get_remotecontrol_device(int endpoint_id);

  std::list<ZigbeeDevice*> get_local_device_list();
};

}  // namespace artik

#endif  // ZIGBEE_CPP_ARTIK_ZIGBEE_HH_
