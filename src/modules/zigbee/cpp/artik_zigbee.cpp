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

#include "artik_zigbee.hh"

#include <list>

int artik::ZigbeeDevice::get_profile_id() {
  return this->m_profile_id;
}

int artik::ZigbeeDevice::get_endpoint_id() {
  return this->m_endpoint_id;
}

int artik::ZigbeeDevice::get_device_id() {
  return this->m_device_id;
}

artik_zigbee_endpoint_handle artik::ZigbeeDevice::get_handle() {
  return this->m_handle;
}

void artik::ZigbeeDevice::set_handle(artik_zigbee_endpoint_handle handle) {
  this->m_handle = handle;
}

artik::Zigbee::Zigbee() {
  m_module = reinterpret_cast<artik_zigbee_module*>(
      artik_request_api_module("zigbee"));
}

artik::Zigbee::~Zigbee() {
  this->m_module->deinitialize();

  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

void artik::Zigbee::initialize(artik_zigbee_client_callback callback,
    void *user_data) {
  artik_zigbee_local_endpoint_info endpoint_info;
  int index = 0;
  std::list<artik::ZigbeeDevice*>::iterator it;

  for (it = device_list.begin();
      it != device_list.end() && index < ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE;
      it++, index++) {
    endpoint_info.endpoints[index].profile = (*it)->get_profile_id();
    endpoint_info.endpoints[index].endpoint_id = (*it)->get_endpoint_id();
    endpoint_info.endpoints[index].device_id = (*it)->get_device_id();
  }

  endpoint_info.count = index;
  m_module->set_local_endpoint(&endpoint_info);
  m_module->initialize(callback, user_data);
}

artik_error artik::Zigbee::network_start(artik_zigbee_network_state *state) {
  return m_module->network_start(state);
}

void artik::Zigbee::network_form() {
  m_module->network_form();
}

artik_error artik::Zigbee::network_form_manually(
    const artik_zigbee_network_info *network_info) {
  return m_module->network_form_manually(network_info);
}

artik_error artik::Zigbee::network_permitjoin(int duration_sec) {
  return m_module->network_permitjoin(duration_sec);
}

artik_error artik::Zigbee::network_leave() {
  return m_module->network_leave();
}

void artik::Zigbee::network_join() {
  m_module->network_join();
}

void artik::Zigbee::network_find() {
  m_module->network_find();
}

artik_error artik::Zigbee::network_request_my_network_status(
    artik_zigbee_network_state *state) {
  return m_module->network_request_my_network_status(state);
}

artik_error artik::Zigbee::device_request_my_node_type(
    artik_zigbee_node_type *type) {
  return m_module->device_request_my_node_type(type);
}

void artik::Zigbee::device_find_by_cluster(
    artik_zigbee_endpoint_list *endpoints, int cluster_id, int is_server) {
  m_module->device_find_by_cluster(endpoints, cluster_id, is_server);
}

artik_error artik::Zigbee::reset_local() {
  return m_module->reset_local();
}

artik_error artik::Zigbee::network_stop_scan() {
  return m_module->network_stop_scan();
}

artik_error artik::Zigbee::network_join_manually(
    const artik_zigbee_network_info *network_info) {
  return m_module->network_join_manually(network_info);
}

artik_error artik::Zigbee::device_discover() {
  return m_module->device_discover();
}

artik_error artik::Zigbee::set_discover_cycle_time(unsigned int time_minutes) {
  return m_module->set_discover_cycle_time(time_minutes);
}

artik_error artik::Zigbee::get_discovered_device_list(
    artik_zigbee_device_info *device_info) {
  return m_module->get_discovered_device_list(device_info);
}

void artik::Zigbee::raw_request(const char *command) {
  m_module->raw_request(command);
}

std::list<artik::ZigbeeDevice*> artik::Zigbee::get_local_device_list() {
  return device_list;
}

artik::OnOffLightDevice* artik::Zigbee::get_onofflight_device(
    int endpoint_id) {
  artik::OnOffLightDevice* onofflight =
      new artik::OnOffLightDevice(endpoint_id);
  onofflight->set_handle((artik_zigbee_endpoint_handle)(intptr_t) endpoint_id);
  device_list.push_back(onofflight);
  return onofflight;
}

artik::OnOffSwitchDevice* artik::Zigbee::get_onoffswitch_device(
    int endpoint_id) {
  artik::OnOffSwitchDevice* onoffswitch =
      new artik::OnOffSwitchDevice(endpoint_id);
  onoffswitch->set_handle((artik_zigbee_endpoint_handle)(intptr_t) endpoint_id);
  device_list.push_back(onoffswitch);
  return onoffswitch;
}

artik::LevelControlSwitchDevice* artik::Zigbee::get_levelcontrolswitch_device(
    int endpoint_id) {
  artik::LevelControlSwitchDevice* levelcontrolswitch =
      new artik::LevelControlSwitchDevice(endpoint_id);
  levelcontrolswitch->set_handle((artik_zigbee_endpoint_handle)
                                 (intptr_t) endpoint_id);
  device_list.push_back(levelcontrolswitch);
  return levelcontrolswitch;
}

artik::DimmableLightDevice* artik::Zigbee::get_dimmablelight_device(
    int endpoint_id) {
  artik::DimmableLightDevice* dimmablelight =
      new artik::DimmableLightDevice(endpoint_id);
  dimmablelight->set_handle((artik_zigbee_endpoint_handle)
                            (intptr_t) endpoint_id);
  device_list.push_back(dimmablelight);
  return dimmablelight;
}

artik::LightSensorDevice* artik::Zigbee::get_lightsensor_device(
    int endpoint_id) {
  artik::LightSensorDevice* lightsensor =
      new artik::LightSensorDevice(endpoint_id);
  lightsensor->set_handle((artik_zigbee_endpoint_handle)(intptr_t) endpoint_id);
  device_list.push_back(lightsensor);
  return lightsensor;
}

artik::RemoteControlDevice* artik::Zigbee::get_remotecontrol_device(
    int endpoint_id) {
  artik::RemoteControlDevice* remotecontrol =
      new artik::RemoteControlDevice(endpoint_id);
  remotecontrol->set_handle((artik_zigbee_endpoint_handle)
                            (intptr_t) endpoint_id);
  device_list.push_back(remotecontrol);
  return remotecontrol;
}

artik::OnOffLightDevice::OnOffLightDevice(int endpoint_id) {
  this->m_device = device_on_off_light_func;
  this->m_endpoint_id = endpoint_id;
  this->m_profile_id = ARTIK_ZIGBEE_PROFILE_HA;
  this->m_device_id = ARTIK_ZIGBEE_DEVICE_ON_OFF_LIGHT;
}

artik_error artik::OnOffLightDevice::groups_get_local_name_support(
    int endpoint) {
  return this->m_device.groups_get_local_name_support(this->m_handle, endpoint);
}

artik_error artik::OnOffLightDevice::groups_set_local_name_support(
    bool support) {
  return this->m_device.groups_set_local_name_support(this->m_handle, support);
}

artik_error artik::OnOffLightDevice::onoff_get_value(
    artik_zigbee_onoff_status *status) {
  return this->m_device.onoff_get_value(this->m_handle, status);
}

artik_error artik::OnOffLightDevice::ezmode_commissioning_target_start() {
  return this->m_device.ezmode_commissioning_target_start(this->m_handle);
}

artik_error artik::OnOffLightDevice::ezmode_commissioning_target_stop() {
  return this->m_device.ezmode_commissioning_target_stop(this->m_handle);
}

artik::OnOffSwitchDevice::OnOffSwitchDevice(int endpoint_id) {
  this->m_device = device_on_off_switch_func;
  this->m_endpoint_id = endpoint_id;
  this->m_profile_id = ARTIK_ZIGBEE_PROFILE_HA;
  this->m_device_id = ARTIK_ZIGBEE_DEVICE_ON_OFF_SWITCH;
}

artik_error artik::OnOffSwitchDevice::identify_request(
    const artik_zigbee_endpoint *endpoint, int duration) {
  return this->m_device.identify_request(this->m_handle, endpoint, duration);
}

artik_error artik::OnOffSwitchDevice::identify_get_remaining_time(
    const artik_zigbee_endpoint *endpoint, int *time) {
  return this->m_device.identify_get_remaining_time(this->m_handle, endpoint,
      time);
}

artik_error artik::OnOffSwitchDevice::onoff_command(
    const artik_zigbee_endpoint *endpoint,
    artik_zigbee_onoff_status target_status) {
  return this->m_device.onoff_command(this->m_handle, endpoint, target_status);
}

artik_error artik::OnOffSwitchDevice::ezmode_commissioning_initiator_start() {
  return this->m_device.ezmode_commissioning_initiator_start(this->m_handle);
}

artik_error artik::OnOffSwitchDevice::ezmode_commissioning_initiator_stop() {
  return this->m_device.ezmode_commissioning_initiator_stop(this->m_handle);
}

artik::LevelControlSwitchDevice::LevelControlSwitchDevice(int endpoint_id) {
  this->m_device = device_level_control_switch_func;
  this->m_endpoint_id = endpoint_id;
  this->m_profile_id = ARTIK_ZIGBEE_PROFILE_HA;
  this->m_device_id = ARTIK_ZIGBEE_DEVICE_LEVEL_CONTROL_SWITCH;
}

artik_error artik::LevelControlSwitchDevice::identify_request(
    const artik_zigbee_endpoint *endpoint, int duration) {
  return this->m_device.identify_request(this->m_handle, endpoint, duration);
}

artik_error artik::LevelControlSwitchDevice::identify_get_remaining_time(
    const artik_zigbee_endpoint *endpoint, int *time) {
  return this->m_device.identify_get_remaining_time(this->m_handle,
      endpoint, time);
}

artik_error artik::LevelControlSwitchDevice::onoff_command(
    const artik_zigbee_endpoint *endpoint,
    artik_zigbee_onoff_status target_status) {
  return this->m_device.onoff_command(this->m_handle, endpoint, target_status);
}

artik_error artik::LevelControlSwitchDevice::level_control_request(
    const artik_zigbee_endpoint *endpoint,
    const artik_zigbee_level_control_command *command) {
  return this->m_device.level_control_request(this->m_handle, endpoint,
      command);
}

artik_error
artik::LevelControlSwitchDevice::ezmode_commissioning_initiator_start() {
  return this->m_device.ezmode_commissioning_initiator_start(this->m_handle);
}

artik_error
artik::LevelControlSwitchDevice::ezmode_commissioning_initiator_stop() {
  return this->m_device.ezmode_commissioning_initiator_stop(this->m_handle);
}

artik::DimmableLightDevice::DimmableLightDevice(int endpoint_id) {
  this->m_device = device_dimmable_light_func;
  this->m_endpoint_id = endpoint_id;
  this->m_profile_id = ARTIK_ZIGBEE_PROFILE_HA;
  this->m_device_id = ARTIK_ZIGBEE_DEVICE_DIMMABLE_LIGHT;
}

artik_error artik::DimmableLightDevice::groups_get_local_name_support(
    int endpoint) {
  return this->m_device.groups_get_local_name_support(this->m_handle, endpoint);
}

artik_error artik::DimmableLightDevice::groups_set_local_name_support(
    bool support) {
  return this->m_device.groups_set_local_name_support(this->m_handle, support);
}

artik_error artik::DimmableLightDevice::onoff_get_value(
    artik_zigbee_onoff_status *status) {
  return this->m_device.onoff_get_value(this->m_handle, status);
}

artik_error artik::DimmableLightDevice::level_control_get_value(int *value) {
  return this->m_device.level_control_get_value(this->m_handle, value);
}

artik_error artik::DimmableLightDevice::ezmode_commissioning_target_start() {
  return this->m_device.ezmode_commissioning_target_start(this->m_handle);
}

artik_error artik::DimmableLightDevice::ezmode_commissioning_target_stop() {
  return this->m_device.ezmode_commissioning_target_stop(this->m_handle);
}

artik::LightSensorDevice::LightSensorDevice(int endpoint_id) {
  this->m_device = device_light_sensor_func;
  this->m_endpoint_id = endpoint_id;
  this->m_profile_id = ARTIK_ZIGBEE_PROFILE_HA;
  this->m_device_id = ARTIK_ZIGBEE_DEVICE_LIGHT_SENSOR;
}

artik_error artik::LightSensorDevice::identify_request(
    const artik_zigbee_endpoint *endpoint, int duration) {
  return this->m_device.identify_request(this->m_handle, endpoint, duration);
}

artik_error artik::LightSensorDevice::identify_get_remaining_time(
    const artik_zigbee_endpoint *endpoint, int *time) {
  return this->m_device.identify_get_remaining_time(this->m_handle, endpoint,
      time);
}

artik_error artik::LightSensorDevice::illum_set_measured_value_range(int min,
    int max) {
  return this->m_device.illum_set_measured_value_range(this->m_handle, min,
      max);
}

artik_error artik::LightSensorDevice::illum_set_measured_value(int value) {
  return this->m_device.illum_set_measured_value(this->m_handle, value);
}

artik_error artik::LightSensorDevice::illum_get_measured_value(int* value) {
  return this->m_device.illum_get_measured_value(this->m_handle, value);
}

artik_error artik::LightSensorDevice::ezmode_commissioning_initiator_start() {
  return this->m_device.ezmode_commissioning_initiator_start(this->m_handle);
}

artik_error artik::LightSensorDevice::ezmode_commissioning_initiator_stop() {
  return this->m_device.ezmode_commissioning_initiator_stop(this->m_handle);
}

artik::RemoteControlDevice::RemoteControlDevice(int endpoint_id) {
  this->m_device = device_remote_control_func;
  this->m_endpoint_id = endpoint_id;
  this->m_profile_id = ARTIK_ZIGBEE_PROFILE_HA;
  this->m_device_id = ARTIK_ZIGBEE_DEVICE_REMOTE_CONTROL;
}

artik_error artik::RemoteControlDevice::reset_to_factory_default(
    const artik_zigbee_endpoint *endpoint) {
  return this->m_device.reset_to_factory_default(this->m_handle, endpoint);
}

artik_error artik::RemoteControlDevice::identify_request(
    const artik_zigbee_endpoint *endpoint, int duration) {
  return this->m_device.identify_request(this->m_handle, endpoint, duration);
}

artik_error artik::RemoteControlDevice::identify_get_remaining_time(
    const artik_zigbee_endpoint *endpoint, int *time) {
  return this->m_device.identify_get_remaining_time(this->m_handle, endpoint,
      time);
}

artik_error artik::RemoteControlDevice::onoff_command(
    const artik_zigbee_endpoint *endpoint,
    artik_zigbee_onoff_status target_status) {
  return this->m_device.onoff_command(this->m_handle, endpoint, target_status);
}

artik_error artik::RemoteControlDevice::level_control_request(
    const artik_zigbee_endpoint *endpoint,
    const artik_zigbee_level_control_command *command) {
  return this->m_device.level_control_request(this->m_handle, endpoint,
      command);
}

artik_error artik::RemoteControlDevice::request_reporting(
    const artik_zigbee_endpoint *endpoint,
    artik_zigbee_reporting_type report_type,
    int min_interval, int max_interval, float change_threshold) {
  return this->m_device.request_reporting(this->m_handle, endpoint,
      report_type, min_interval, max_interval, change_threshold);
}

artik_error artik::RemoteControlDevice::stop_reporting(
    const artik_zigbee_endpoint *endpoint,
    artik_zigbee_reporting_type report_type) {
  return this->m_device.stop_reporting(this->m_handle, endpoint, report_type);
}

artik_error artik::RemoteControlDevice::ezmode_commissioning_target_start() {
  return this->m_device.ezmode_commissioning_target_start(this->m_handle);
}

artik_error artik::RemoteControlDevice::ezmode_commissioning_target_stop() {
  return this->m_device.ezmode_commissioning_target_stop(this->m_handle);
}

