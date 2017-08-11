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

#ifndef	__ARTIK_ZIGBEE_H__
#define	__ARTIK_ZIGBEE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "artik_error.h"
#include "artik_types.h"
#include "artik_zigbee_clusters.h"

/*! \file artik_zigbee.h
 *
 *	\brief ZigBee module definition
 *
 *	Definitions and functions for accessing
 *	the ZigBee module and performing operations
 *
 *  \example zigbee_test/artik_zigbee_test.c
 *  \example zigbee_test/artik_zigbee_cli.c
 *  \example zigbee_test/artik_zigbee_test_common.c
 *  \example zigbee_test/artik_zigbee_test_common.h
 */

typedef unsigned short ARTIK_ZIGBEE_PROFILE;
#define ARTIK_ZIGBEE_PROFILE_HA		0x0104
#define ARTIK_ZIGBEE_PROFILE_ZLL	0xC05E
#define ARTIK_ZIGBEE_PROFILE_GP		0xA1E0

typedef unsigned short ARTIK_ZIGBEE_DEVICEID;
#define ARTIK_ZIGBEE_DEVICE_ON_OFF_SWITCH		0x0000
#define ARTIK_ZIGBEE_DEVICE_LEVEL_CONTROL_SWITCH	0x0001
#define ARTIK_ZIGBEE_DEVICE_REMOTE_CONTROL		0x0006
#define ARTIK_ZIGBEE_DEVICE_ON_OFF_LIGHT		0x0100
#define ARTIK_ZIGBEE_DEVICE_DIMMABLE_LIGHT		0x0101
#define ARTIK_ZIGBEE_DEVICE_COLOR_DIMMABLE_LIGHT	0x0102
#define ARTIK_ZIGBEE_DEVICE_ON_OFF_LIGHT_SWITCH		0x0103
#define ARTIK_ZIGBEE_DEVICE_DIMMER_SWITCH		0x0104
#define ARTIK_ZIGBEE_DEVICE_COLOR_DIMMER_SWITCH		0x0105
#define ARTIK_ZIGBEE_DEVICE_LIGHT_SENSOR		0x0106
#define ARTIK_ZIGBEE_DEVICE_OCCUPANCY_SENSOR		0x0107
#define ARTIK_ZIGBEE_DEVICE_HEATING_COOLING_UNIT	0x0300
#define ARTIK_ZIGBEE_DEVICE_THERMOSTAT			0x0301
#define ARTIK_ZIGBEE_DEVICE_TEMPERATURE_SENSOR		0x0302
#define ARTIK_ZIGBEE_DEVICE_INVALIDATE			0x7FFF

#define ARTIK_ZCL_BASIC_CLUSTER_ID                         0x0000
#define ARTIK_ZCL_POWER_CONFIG_CLUSTER_ID                  0x0001
#define ARTIK_ZCL_DEVICE_TEMP_CLUSTER_ID                   0x0002
#define ARTIK_ZCL_IDENTIFY_CLUSTER_ID                      0x0003
#define ARTIK_ZCL_GROUPS_CLUSTER_ID                        0x0004
#define ARTIK_ZCL_SCENES_CLUSTER_ID                        0x0005
#define ARTIK_ZCL_ON_OFF_CLUSTER_ID                        0x0006
#define ARTIK_ZCL_ON_OFF_SWITCH_CONFIG_CLUSTER_ID          0x0007
#define ARTIK_ZCL_LEVEL_CONTROL_CLUSTER_ID                 0x0008
#define ARTIK_ZCL_ALARM_CLUSTER_ID                         0x0009
#define ARTIK_ZCL_TIME_CLUSTER_ID                          0x000A
#define ARTIK_ZCL_RSSI_LOCATION_CLUSTER_ID                 0x000B
#define ARTIK_ZCL_BINARY_INPUT_BASIC_CLUSTER_ID            0x000F
#define ARTIK_ZCL_COMMISSIONING_CLUSTER_ID                 0x0015
#define ARTIK_ZCL_PARTITION_CLUSTER_ID                     0x0016
#define ARTIK_ZCL_OTA_BOOTLOAD_CLUSTER_ID                  0x0019
#define ARTIK_ZCL_POWER_PROFILE_CLUSTER_ID                 0x001A
#define ARTIK_ZCL_APPLIANCE_CONTROL_CLUSTER_ID             0x001B
#define ARTIK_ZCL_POLL_CONTROL_CLUSTER_ID                  0x0020
#define ARTIK_ZCL_SHADE_CONFIG_CLUSTER_ID                  0x0100
#define ARTIK_ZCL_DOOR_LOCK_CLUSTER_ID                     0x0101
#define ARTIK_ZCL_WINDOW_COVERING_CLUSTER_ID               0x0102
#define ARTIK_ZCL_PUMP_CONFIG_CONTROL_CLUSTER_ID           0x0200
#define ARTIK_ZCL_THERMOSTAT_CLUSTER_ID                    0x0201
#define ARTIK_ZCL_FAN_CONTROL_CLUSTER_ID                   0x0202
#define ARTIK_ZCL_DEHUMID_CONTROL_CLUSTER_ID               0x0203
#define ARTIK_ZCL_THERMOSTAT_UI_CONFIG_CLUSTER_ID          0x0204
#define ARTIK_ZCL_COLOR_CONTROL_CLUSTER_ID                 0x0300
#define ARTIK_ZCL_BALLAST_CONFIGURATION_CLUSTER_ID         0x0301
#define ARTIK_ZCL_ILLUM_MEASUREMENT_CLUSTER_ID             0x0400
#define ARTIK_ZCL_ILLUM_LEVEL_SENSING_CLUSTER_ID           0x0401
#define ARTIK_ZCL_TEMP_MEASUREMENT_CLUSTER_ID              0x0402
#define ARTIK_ZCL_PRESSURE_MEASUREMENT_CLUSTER_ID          0x0403
#define ARTIK_ZCL_FLOW_MEASUREMENT_CLUSTER_ID              0x0404
#define ARTIK_ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID 0x0405
#define ARTIK_ZCL_OCCUPANCY_SENSING_CLUSTER_ID             0x0406
#define ARTIK_ZCL_IAS_ZONE_CLUSTER_ID                      0x0500
#define ARTIK_ZCL_IAS_ACE_CLUSTER_ID                       0x0501
#define ARTIK_ZCL_IAS_WD_CLUSTER_ID                        0x0502
#define ARTIK_ZCL_GENERIC_TUNNEL_CLUSTER_ID                0x0600
#define ARTIK_ZCL_BACNET_PROTOCOL_TUNNEL_CLUSTER_ID        0x0601
#define ARTIK_ZCL_11073_PROTOCOL_TUNNEL_CLUSTER_ID         0x0614
#define ARTIK_ZCL_ISO7816_PROTOCOL_TUNNEL_CLUSTER_ID       0x0615
#define ARTIK_ZCL_PRICE_CLUSTER_ID                         0x0700
#define ARTIK_ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID  0x0701
#define ARTIK_ZCL_SIMPLE_METERING_CLUSTER_ID               0x0702
#define ARTIK_ZCL_MESSAGING_CLUSTER_ID                     0x0703
#define ARTIK_ZCL_TUNNELING_CLUSTER_ID                     0x0704
#define ARTIK_ZCL_PREPAYMENT_CLUSTER_ID                    0x0705
#define ARTIK_ZCL_ENERGY_MANAGEMENT_CLUSTER_ID             0x0706
#define ARTIK_ZCL_CALENDAR_CLUSTER_ID                      0x0707
#define ARTIK_ZCL_DEVICE_MANAGEMENT_CLUSTER_ID             0x0708
#define ARTIK_ZCL_EVENTS_CLUSTER_ID                        0x0709
#define ARTIK_ZCL_MDU_PAIRING_CLUSTER_ID                   0x070A
#define ARTIK_ZCL_KEY_ESTABLISHMENT_CLUSTER_ID             0x0800
#define ARTIK_ZCL_INFORMATION_CLUSTER_ID                   0x0900
#define ARTIK_ZCL_DATA_SHARING_CLUSTER_ID                  0x0901
#define ARTIK_ZCL_GAMING_CLUSTER_ID                        0x0902
#define ARTIK_ZCL_DATA_RATE_CONTROL_CLUSTER_ID             0x0903
#define ARTIK_ZCL_VOICE_OVER_ZIGBEE_CLUSTER_ID             0x0904
#define ARTIK_ZCL_CHATTING_CLUSTER_ID                      0x0905
#define ARTIK_ZCL_PAYMENT_CLUSTER_ID                       0x0A01
#define ARTIK_ZCL_BILLING_CLUSTER_ID                       0x0A02
#define ARTIK_ZCL_APPLIANCE_IDENTIFICATION_CLUSTER_ID      0x0B00
#define ARTIK_ZCL_METER_IDENTIFICATION_CLUSTER_ID          0x0B01
#define ARTIK_ZCL_APPLIANCE_EVENTS_AND_ALERT_CLUSTER_ID    0x0B02
#define ARTIK_ZCL_APPLIANCE_STATISTICS_CLUSTER_ID          0x0B03
#define ARTIK_ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID        0x0B04
#define ARTIK_ZCL_DIAGNOSTICS_CLUSTER_ID                   0x0B05
#define ARTIK_ZCL_ZLL_COMMISSIONING_CLUSTER_ID             0x1000
#define ARTIK_ZCL_SAMPLE_MFG_SPECIFIC_CLUSTER_ID           0xFC00

#define	ARTIK_ZIGBEE_MAX_COMMAND_BUFFER_SIZE			5000
#define ARTIK_ZIGBEE_MAX_RESPONSE_SIZE				10000
#define ARTIK_ZIGBEE_MAX_RECEIVED_COMMAND_PLAYLOAD_LENGTH	1024
#define ARTIK_ZIGBEE_MAX_DEVICE_INFO_SIZE			20
#define ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE				5
#define ARTIK_ZIGBEE_MAX_CLUSTER_SIZE				9
#define ARTIK_ZIGBEE_EUI64_SIZE					8
#define ARTIK_ZIGBEE_COORDINATOR_NODEID				0x0000
#define ARTIK_ZIGBEE_BROADCAST_ENDPOINT				0xFF

/*!
 * \brief Receiving command notification type from ZigBee Daemon
 *        This is one of response types
 */
typedef enum {
	ARTIK_ZIGBEE_CMD_SUCCESS = 0,
	ARTIK_ZIGBEE_CMD_ERR_PORT_PROBLEM = -3001,
	ARTIK_ZIGBEE_CMD_ERR_NO_SUCH_COMMAND = -3002,
	ARTIK_ZIGBEE_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS = -3003,
	ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_OUT_OF_RANGE = -3004,
	ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_SYNTAX_ERROR = -3005,
	ARTIK_ZIGBEE_CMD_ERR_STRING_TOO_LONG = -3006,
	ARTIK_ZIGBEE_CMD_ERR_INVALID_ARGUMENT_TYPE = -3007,
	ARTIK_ZIGBEE_CMD_ERR = -3008
} artik_zigbee_notification;
/*!
 * \brief Receiving network notification type from ZigBee Daemon
 *        This is one of response types
 */
typedef enum {
	/* It is notified in coordinator side when router joins
	 * current network
	 */
	ARTIK_ZIGBEE_NETWORK_JOIN = 3100,
	/* It is notified in coordinator side when router leaves current network
	 */
	ARTIK_ZIGBEE_NETWORK_LEAVE,
	ARTIK_ZIGBEE_NETWORK_FIND_FORM_SUCCESS,
	ARTIK_ZIGBEE_NETWORK_FIND_FORM_FAILED,
	ARTIK_ZIGBEE_NETWORK_FIND_JOIN_SUCCESS,
	ARTIK_ZIGBEE_NETWORK_FIND_JOIN_FAILED,
	/* If it is in a network currently and form/join is re-called, this type
	 * notification will be used
	 */
	ARTIK_ZIGBEE_NETWORK_EXIST,
	/* It is used when network_form_manually */
	ARTIK_ZIGBEE_NETWORK_FORM_SUCCESS,
	ARTIK_ZIGBEE_NETWORK_FORM_FAILED,
	/* It is used when network_join_manually */
	ARTIK_ZIGBEE_NETWORK_JOIN_SUCCESS,
	ARTIK_ZIGBEE_NETWORK_JOIN_FAILED
} artik_zigbee_network_notification;
/*!
 * \brief Receiving node type from ZigBee Daemon
 *        This is one of response types
 */
typedef enum {
	ARTIK_ZIGBEE_UNKNOWN_DEVICE = 3200,
	ARTIK_ZIGBEE_COORDINATOR = 3201,
	ARTIK_ZIGBEE_ROUTER = 3202,
	ARTIK_ZIGBEE_END_DEVICE = 3203,
	ARTIK_ZIGBEE_SLEEPY_END_DEVICE = 3204
} artik_zigbee_node_type;
/*!
 * \brief Receiving network state from ZigBee Daemon
 *        This is one of response types
 */
typedef enum {
	ARTIK_ZIGBEE_NO_NETWORK = 3210,
	ARTIK_ZIGBEE_JOINING_NETWORK,
	ARTIK_ZIGBEE_JOINED_NETWORK,
	ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT,
	ARTIK_ZIGBEE_LEAVING_NETWORK
} artik_zigbee_network_state;
/*!
 * \brief Device discovery result
 */
typedef enum {
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_NO_DEVICE = 3220,
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_FOUND,
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_DONE,
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_START,
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_ERROR,
	/* Cyclic discovery is in progress or last calling is not finished */
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_IN_PROGRESS,
	/* In cycle discovery, device is changed */
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_CHANGED,
	/* In cycle discovery, device is lost */
	ARTIK_ZIGBEE_DEVICE_DISCOVERY_LOST
} artik_zigbee_device_discovery_status;
/*!
 * \brief Network find status
 */
typedef enum {
	ARTIK_ZIGBEE_NETWORK_FOUND = 3230,
	ARTIK_ZIGBEE_NETWORK_FIND_FINISHED,
	ARTIK_ZIGBEE_NETWORK_FIND_ERR
} artik_zigbee_network_find_status;

typedef enum {
	ARTIK_ZIGBEE_SERVICE_DISCOVERY_RECEIVED = 3240,
	ARTIK_ZIGBEE_SERVICE_DISCOVERY_DONE,
	ARTIK_ZIGBEE_SERVICE_DISCOVERY_ERROR
} artik_zigbee_service_discovery_result;
/*!
 * \brief Receiving response type from ZigBee Daemon
 */
typedef enum {
	/* Common response */
	ARTIK_ZIGBEE_RESPONSE_NOTIFICATION = 3300,
	ARTIK_ZIGBEE_RESPONSE_CLIENT_TO_SERVER_COMMAND_RECEIVED,
	ARTIK_ZIGBEE_RESPONSE_ATTRIBUTE_CHANGE,
	ARTIK_ZIGBEE_RESPONSE_REPORTING_CONFIGURE,
	ARTIK_ZIGBEE_RESPONSE_REPORT_ATTRIBUTE,
	ARTIK_ZIGBEE_RESPONSE_IDENTIFY_FEEDBACK_START,
	ARTIK_ZIGBEE_RESPONSE_IDENTIFY_FEEDBACK_STOP,
	/* Network response */
	ARTIK_ZIGBEE_RESPONSE_NETWORK_NOTIFICATION,
	ARTIK_ZIGBEE_RESPONSE_NETWORK_FIND,
	/* Device response */
	ARTIK_ZIGBEE_RESPONSE_DEVICE_DISCOVER,
	/* Cluster response */
	ARTIK_ZIGBEE_RESPONSE_BROADCAST_IDENTIFY_QUERY,
	ARTIK_ZIGBEE_RESPONSE_GROUPS_INFO,
	ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS,
	ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_TARGET_INFO,
	ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_BOUND_INFO,
	ARTIK_ZIGBEE_RESPONSE_IEEE_ADDR_RESP,
	ARTIK_ZIGBEE_RESPONSE_SIMPLE_DESC_RESP,
	ARTIK_ZIGBEE_RESPONSE_MATCH_DESC_RESP,
	ARTIK_ZIGBEE_RESPONSE_END_DEVICE_BIND,
	/* < Received basic reset to factory defaults requesting */
	ARTIK_ZIGBEE_RESPONSE_BASIC_RESET_TO_FACTORY,
	ARTIK_ZIGBEE_RESPONSE_LEVEL_CONTROL,
	/* Common response none */
	ARTIK_ZIGBEE_RESPONSE_NONE = 3399
} artik_zigbee_response_type;
/*!
 * \brief Attribute change type
 */
typedef enum {
	ARTIK_ZIGBEE_ATTR_ONOFF_STATUS,
	ARTIK_ZIGBEE_ATTR_LEVELCONTROL_LEVEL,
	ARTIK_ZIGBEE_ATTR_COLOR_HUE,
	ARTIK_ZIGBEE_ATTR_COLOR_SATURATION,
	ARTIK_ZIGBEE_ATTR_COLOR_CURRENT_X,
	ARTIK_ZIGBEE_ATTR_COLOR_CURRENT_Y,
	ARTIK_ZIGBEE_ATTR_COLOR_TEMP,
	ARTIK_ZIGBEE_ATTR_FAN_MODE,
	ARTIK_ZIGBEE_ATTR_FAN_MODE_SEQUENCE,
	ARTIK_ZIGBEE_ATTR_OCCUPIED_HEATING_SETPOINT,
	ARTIK_ZIGBEE_ATTR_OCCUPIED_COOLING_SETPOINT,
	ARTIK_ZIGBEE_ATTR_SYSTEM_MODE,
	ARTIK_ZIGBEE_ATTR_CONTROL_SEQUENCE,
	/* cls:0x0400, attr:0x0000 */
	ARTIK_ZIGBEE_ATTR_ILLUMINANCE,
	/* cls:0x0402, attr:0x0000 */
	ARTIK_ZIGBEE_ATTR_TEMPERATURE,
	/* cls:0x0406, attr:0x0000 */
	ARTIK_ZIGBEE_ATTR_OCCUPANCY,
	/* cls:0x0201, attr:0x0000 */
	ARTIK_ZIGBEE_ATTR_THERMOSTAT_TEMPERATURE,
	ARTIK_ZIGBEE_ATTR_NONE
} artik_zigbee_attribute_type;
/*!
 * \brief Selectable tx power level
 */
typedef enum {
	ARTIK_ZIGBEE_TX_POWER_8 = 8,
	ARTIK_ZIGBEE_TX_POWER_7 = 7,
	ARTIK_ZIGBEE_TX_POWER_6 = 6,
	ARTIK_ZIGBEE_TX_POWER_5 = 5,
	ARTIK_ZIGBEE_TX_POWER_4 = 4,
	ARTIK_ZIGBEE_TX_POWER_3 = 3,
	ARTIK_ZIGBEE_TX_POWER_2 = 2,
	ARTIK_ZIGBEE_TX_POWER_1 = 1,
	ARTIK_ZIGBEE_TX_POWER_0 = 0,
	ARTIK_ZIGBEE_TX_POWER_MINUS1 = -1,
	ARTIK_ZIGBEE_TX_POWER_MINUS2 = -2,
	ARTIK_ZIGBEE_TX_POWER_MINUS3 = -3,
	ARTIK_ZIGBEE_TX_POWER_MINUS4 = -4,
	ARTIK_ZIGBEE_TX_POWER_MINUS5 = -5,
	ARTIK_ZIGBEE_TX_POWER_MINUS6 = -6,
	ARTIK_ZIGBEE_TX_POWER_MINUS7 = -7,
	ARTIK_ZIGBEE_TX_POWER_MINUS8 = -8,
	ARTIK_ZIGBEE_TX_POWER_MINUS9 = -9,
	ARTIK_ZIGBEE_TX_POWER_MINUS11 = -11,
	ARTIK_ZIGBEE_TX_POWER_MINUS12 = -12,
	ARTIK_ZIGBEE_TX_POWER_MINUS14 = -14,
	ARTIK_ZIGBEE_TX_POWER_MINUS17 = -17,
	ARTIK_ZIGBEE_TX_POWER_MINUS20 = -20,
	ARTIK_ZIGBEE_TX_POWER_MINUS26 = -26,
	ARTIK_ZIGBEE_TX_POWER_MINUS43 = -43
} artik_zigbee_tx_power;
/*!
 * \brief Request for device attribute information reporting
 */
typedef enum {
	/*
	 * The change_threshold should be in the range 0 ~ 60082
	 *
	 */
	ARTIK_ZIGBEE_REPORTING_THERMOSTAT_TEMPERATURE = 0,
	/*
	 * ARTIK_ZIGBEE_REPORTING_OCCUPANCY_SENSING for change_threshold
	 * parameter has no meaning, set change_threshold value to 0
	 *
	 */
	ARTIK_ZIGBEE_REPORTING_OCCUPANCY_SENSING,
	/*
	 * The change_threshold should be in the range 0 ~ 65533
	 *
	 */
	ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE,
	/*
	 * The change_threshold should be in the range 0 ~ 60082
	 *
	 */
	ARTIK_ZIGBEE_REPORTING_MEASURED_TEMPERATURE
} artik_zigbee_reporting_type;
/*!
 * \brief Local endpoint
 */
typedef struct {
	ARTIK_ZIGBEE_PROFILE profile;
	int endpoint_id;
	ARTIK_ZIGBEE_DEVICEID device_id;
} artik_zigbee_local_endpoint;
/*!
 * \brief Local endpoint information
 */
typedef struct {
	int count;
	artik_zigbee_local_endpoint endpoints[ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE];
} artik_zigbee_local_endpoint_info;
/*!
 * \brief Structure for endpoint information
 */
typedef struct {
	int endpoint_id;
	int node_id;
	ARTIK_ZIGBEE_DEVICEID device_id;
	int server_cluster[ARTIK_ZIGBEE_MAX_CLUSTER_SIZE];
	int client_cluster[ARTIK_ZIGBEE_MAX_CLUSTER_SIZE];
} artik_zigbee_endpoint;
/*!
 * \brief Structure for endpoint list to find endpoints by cluster ID
 */
typedef struct {
	int num;
	artik_zigbee_endpoint endpoint[ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE *
					ARTIK_ZIGBEE_MAX_DEVICE_INFO_SIZE];
} artik_zigbee_endpoint_list;
/*!
 * \brief Structure for device information
 *        This is used when sending cluster commands to select device
 */
typedef struct {
	char eui64[ARTIK_ZIGBEE_EUI64_SIZE];
	int node_id;
	int endpoint_count;
	artik_zigbee_endpoint endpoint[ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE];
} artik_zigbee_device;
/*!
 * \brief List of device information
 */
typedef struct {
	int num;
	artik_zigbee_device device[ARTIK_ZIGBEE_MAX_DEVICE_INFO_SIZE];
} artik_zigbee_device_info;
/*!
 * \brief Device discovery response
 */
typedef struct {
	artik_zigbee_device_discovery_status status;
	artik_zigbee_device device;
} artik_zigbee_device_discovery;
/*!
 * \brief	Structure for network information
 */
typedef struct {
	int channel;
	artik_zigbee_tx_power tx_power;
	int pan_id;
} artik_zigbee_network_info;
/*!
 * \brief Network find result
 */
typedef struct {
	artik_zigbee_network_find_status find_status;
	artik_zigbee_network_info network_info;
} artik_zigbee_network_find_result;
/*!
 * \brief Received command from remote device
 */
typedef struct {
	bool is_global_command;
	int dest_endpoint_id;
	int cluster_id;
	int command_id;
	char payload[ARTIK_ZIGBEE_MAX_RECEIVED_COMMAND_PLAYLOAD_LENGTH];
	/* -1 if buffer size isn't enough */
	int payload_length;
	int source_node_id;
	int source_endpoint_id;
} artik_zigbee_received_command;
/*!
 * \brief Attribute changed response from zigbeed
 */
typedef struct {
	artik_zigbee_attribute_type type;
	int endpoint_id;
} artik_zigbee_attribute_changed_response;

/*!
 * \brief A structure used to store reporting configurations. If endpoint
 *        field is ::EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID, this
 *        configure is unused.
 */
typedef struct {
	bool used;
	int endpoint_id;
	int cluster_id;
	int attribute_id;
	bool is_server;

	struct {
		/** The minimum reporting interval, measured in seconds. */
		uint16_t min_interval;
		/** The maximum reporting interval, measured in seconds. */
		uint16_t max_interval;
		/** The minimum change to the attribute that will result in
		 *   a report being sent.
		 */
		uint32_t reportable_change;
	} reported;
} artik_zigbee_reporting_info;

/*!
 * \brief Structure for report attribute from remote server
 */
typedef struct {
	artik_zigbee_attribute_type attribute_type;
	union {
		int value;
		artik_zigbee_occupancy_status occupancy;
	} data;
} artik_zigbee_report_attribute_info;

/*!
 * \brief A structure for notifying identify feedback.
 */
typedef struct {
	int endpoint_id;
	int duration;	/* in seconds */
} artik_zigbee_identify_feedback_info;

/*!
 * \brief Structure of target that ezmode commissioning find
 *        Used in callback, to notify user
 */
typedef struct {
	int node_id;
	int endpoint_id;
} artik_zigbee_commissioning_target_info;

/*!
 * \brief Structure of clusters that bound by ezmode commissioning
 *        Used in callback, to notify user
 */
typedef struct {
	int node_id;
	int cluster_id;
	int endpoint_id;
} artik_zigbee_commissioning_bound_info;

/*!
 * \brief Commissioning state
 *        Used in callback, to notify user
 */
typedef enum {
	/* < Commissioning error */
	ARTIK_ZIGBEE_COMMISSIONING_ERROR = 0x00,
	/* < Commissioning in progress error */
	ARTIK_ZIGBEE_COMMISSIONING_ERR_IN_PROGRESS,
	/* < Try to form a new network if network join failed */
	ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_FORM,
	/* < Network steering successfully */
	ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_SUCCESS,
	/* < Network steering failed */
	ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_FAILED,
	/* < Network steering is in progress, wait */
	ARTIK_ZIGBEE_COMMISSIONING_WAIT_NETWORK_STEERING,
	/* < Commissioning target successfully */
	ARTIK_ZIGBEE_COMMISSIONING_TARGET_SUCCESS,
	/* < Commissioning target stopped */
	ARTIK_ZIGBEE_COMMISSIONING_TARGET_STOP,
	/* < Commissioning target failed */
	ARTIK_ZIGBEE_COMMISSIONING_TARGET_FAILED,
	/* < Commissioning initiator successfully */
	ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_SUCCESS,
	/* < Commissioning initiator stopped */
	ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_STOP,
	/* < Commissioning initiator failed */
	ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_FAILED
} artik_zigbee_commissioning_state;

/*!
 * \brief Simple descriptor response
 *        Used in callback, to notify user
 */
typedef struct {
	artik_zigbee_service_discovery_result result;
	int target_node_id;
	int target_endpoint;
	int server_cluster_count;
	int server_cluster[ARTIK_ZIGBEE_MAX_CLUSTER_SIZE];
	int client_cluster_count;
	int client_cluster[ARTIK_ZIGBEE_MAX_CLUSTER_SIZE];
} artik_zigbee_simple_descriptor_response;

/*!
 * \brief Broadcast identify query response
 *        Used in callback, to notify user
 */
typedef struct {
	int node_id;
	int endpoint_id;
	int timeout;
} artik_zigbee_broadcast_identify_query_response;

/*!
 * \brief Ieee addr response
 *        Used in callback, to notify user
 */
typedef struct {
	artik_zigbee_service_discovery_result result;
	int node_id;
	char eui64[ARTIK_ZIGBEE_EUI64_SIZE];
} artik_zigbee_ieee_addr_response;

/*!
 * \brief Match descriptor response
 *        Used in callback, to notify user
 */
typedef struct {
	artik_zigbee_service_discovery_result result;
	int node_id;
	int endpoint_list[ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE];
	int count;
} artik_zigbee_match_desc_response;

/*!
 * \brief Zigbee binding type
 */
typedef enum {
	/* A binding that is currently not in use. */
	ARTIK_UNUSED_BINDING = 0,
	/* A unicast binding whose 64-bit identifier is
	 * the destination EUI64.
	 */
	ARTIK_UNICAST_BINDING = 1,
	/* A unicast binding whose 64-bit identifier is the many-to-one
	 * destination EUI64.  Route discovery should be disabled when sending
	 * unicasts via many-to-one bindings.
	 */
	ARTIK_MANY_TO_ONE_BINDING = 2,
	/* A multicast binding whose 64-bit identifier is the group address. A
	 * multicast binding can be used to send messages to the group and to
	 * receive messages sent to the group.
	 */
	ARTIK_MULTICAST_BINDING	= 3
} artik_zigbee_binding_type;

/*!
 * \brief Zigbee binding table entry
 */
typedef struct {
	/* The type of binding. */
	artik_zigbee_binding_type type;
	/*The endpoint on the local node.*/
	int local;
	/* A cluster ID that matches one from the local endpoint's
	 * simple descriptor. This cluster ID is set by the provisioning
	 * application to indicate which part an endpoint's functionality is
	 * bound to this particular remote node and is used to distinguish
	 * between unicast and multicast bindings. Note that a binding can be
	 * used to to send messages with any cluster ID, not
	 * just that listed in the binding.
	 */
	int cluster_id;
	/* The endpoint on the remote node (specified by \c identifier).*/
	int remote;
	/* A 64-bit identifier.  This is either:
	 * - The destination EUI64, for unicasts
	 * - A 16-bit multicast group address, for multicasts
	 */
	char identifier[ARTIK_ZIGBEE_EUI64_SIZE];
	/* The index of the network the binding belongs to.*/
	int network_index;
} artik_zigbee_binding_table_entry;

/*!
 * \brief endpoint instance
 */
typedef void *artik_zigbee_endpoint_handle;

/*!
 * \brief Structure for device on/off switch, to storage the local endpoint
 *        information and functions can be invoked.
 */
typedef struct {
	/* identify */
	/*!
	 * \brief Send command "Identify", to request identify.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint Remote endpoint instance pointer.
	 * \param [in] duration Identify duration in seconds.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_request)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int duration);
	/*!
	 * \brief Send command "Identify Query", to get remote identify time.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [out] time Remaining seconds of identifying.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_get_remaining_time)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int *time);
	/* onoff client */
	/*!
	 * \brief Send command to control remote on/off.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] target_status On/off command, please refer to
	 *             "enum zigbee_onoff_status".
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*onoff_command)(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint * endpoint,
				artik_zigbee_onoff_status target_status);
	/* ezmode commissioning */
	/*!
	 * \brief Start ezmode commissioning on some endpoint as initiator
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *        it includes following step: network steering,
	 *        finding-and-binding this API will do everything inside
	 *        automatically.
	 *
	 * \param [in] handle The handle of endpoint that start do ezmode
	 *	       commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_NETWORK_STEERING_FORM
	 *                     COMMISSIONING_NETWORK_STEERING_SUCCESS
	 *                     COMMISSIONING_INITIATOR_SUCCESS
	 *                     COMMISSIONING_INITIATOR_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_initiator_start)
		(
			artik_zigbee_endpoint_handle handle
		);

	/*!
	 * \brief Stop ezmode commissioning on some endpoint as initiator
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *        it stop the commissioning procedure.
	 *
	 * \param [in] handle The handle of endpoint that stop do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_INITIATOR_STOP
	 *                     COMMISSIONING_INITIATOR_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_initiator_stop)
		(
			artik_zigbee_endpoint_handle handle
		);
} artik_zigbee_device_on_off_switch;

extern const artik_zigbee_device_on_off_switch device_on_off_switch_func;

/*!
 * \brief Structure for device level control switch, to storage the local
 *        endpoint information and functions can be invoked.
 */
typedef struct {
	/* identify */
	/*!
	 * \brief Send command "Identify".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] duration	Identify duration in seconds.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_request)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int duration);
	/*!
	 * \brief Send command "Identify Query", to get remote identify time.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [out] time Remaining seconds of identifying.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_get_remaining_time)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int *time);
	/* onoff client */
	/*!
	 * \brief Send command to control remote on/off.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] target_status On/off command, please refer to
	 *             "enum zigbee_onoff_status".
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*onoff_command)(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint * endpoint,
				artik_zigbee_onoff_status target_status);
	/* level control client */
	/*!
	 * \brief Send commands about level control, to control remote level.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] command Level control command, please refer
	 *             to "struct zigbee_level_control_command".
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*level_control_request)(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint * endpoint,
			const artik_zigbee_level_control_command * command);
	/* ezmode commissioning */
	/*!
	 * \brief Start ezmode commissioning on some endpoint as initiator
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *        it includes following step: network steering,
	 *        finding-and-binding this API will do everything inside
	 *        automatically.
	 *
	 * \param [in] handle The handle of endpoint that start do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_NETWORK_STEERING_FORM
	 *                     COMMISSIONING_NETWORK_STEERING_SUCCESS
	 *                     COMMISSIONING_INITIATOR_SUCCESS
	 *                     COMMISSIONING_INITIATOR_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_initiator_start)
				(
					artik_zigbee_endpoint_handle handle
				);

	/*!
	 * \brief Stop ezmode commissioning on some endpoint as initiator
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *        it stop commissioning procedure
	 *
	 * \param [in] handle The handle of endpoint that stop do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_INITIATOR_STOP
	 *                     COMMISSIONING_INITIATOR_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_initiator_stop)
			(
				artik_zigbee_endpoint_handle handle
			);
} artik_zigbee_device_level_control_switch;

extern const artik_zigbee_device_level_control_switch
					device_level_control_switch_func;

/*!
 * \brief Structure for device on/off light, to storage the local endpoint
 *        information and functions can be invoked.
 */
typedef struct {
	/* group server */
	/*!
	 * \brief Get attribute of "name support" of cluster "Groups".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Local endpoint id.
	 * \return S_OK	group names is supported.
	 *         E_NOT_SUPPORTED group names isn't supported.
	 *         otherwise is others error code.
	 */
	artik_error(*groups_get_local_name_support)(
					artik_zigbee_endpoint_handle handle,
					int endpoint);
	/*!
	 * \brief Set local attribute of "name support" of cluster "Groups".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] support Set name support attribute.
	 * \return Result of operation, EZ_OK on success, error code otherwise.
	 */
	artik_error(*groups_set_local_name_support)(
					artik_zigbee_endpoint_handle handle,
					bool support);
	/* onoff server */
	/*!
	 * \brief Get attribute of "on/off" of cluster "On/off".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [out]	status On/off status.
	 *	  (refer to enum zigbee_onoff_status).
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*onoff_get_value)(artik_zigbee_endpoint_handle handle,
					artik_zigbee_onoff_status * status);
	/* ezmode commissioning */
	/*!
	 * \brief Start ezmode commissioning on some endpoint as target
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *
	 * \param [in] handle The handle of endpoint that start do ezmode
	 *	       commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_NETWORK_STEERING_FORM
	 *                     COMMISSIONING_NETWORK_STEERING_SUCCESS
	 *                     COMMISSIONING_TARGET_SUCCESS
	 *                     COMMISSIONING_TARGET_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_target_start)
				(
					artik_zigbee_endpoint_handle handle
				);

	/*!
	 * \brief Stop ezmode commissioning on some endpoint as target
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *
	 * \param [in] handle The handle of endpoint that stop do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_TARGET_STOP
	 *                     COMMISSIONING_TARGET_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	 artik_error(*ezmode_commissioning_target_stop)
				(
					artik_zigbee_endpoint_handle handle
				);
} artik_zigbee_device_on_off_light;

extern const artik_zigbee_device_on_off_light device_on_off_light_func;

/*!
 * \brief Structure for device dimmable light, to storage the local endpoint
 *	  information and functions can be invoked.
 */
typedef struct {
	/* group server */
	/*!
	 * \brief Get attribute of "name support" of cluster "Groups".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Local endpoint id.
	 * \return S_OK group names is supported.
	 *         E_NOT_SUPPORTED group names isn't supported.
	 *         otherwise is others error code.
	 */
	artik_error(*groups_get_local_name_support)(
					artik_zigbee_endpoint_handle handle,
					int endpoint);
	/*!
	 * \brief Set local attribute of "name support" of cluster "Groups".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] support Set name support attribute.
	 * \return Result of operation, EZ_OK on success, error code otherwise.
	 */
	artik_error(*groups_set_local_name_support)(
			artik_zigbee_endpoint_handle handle, bool support);
	/* onoff server */
	/*!
	 * \brief Get attribute of "on/off" of cluster "On/off".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [out]	status On/off status.
	 *	        (refer to enum zigbee_onoff_status).
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*onoff_get_value)(artik_zigbee_endpoint_handle handle,
					artik_zigbee_onoff_status * status);
	/* level control server */
	/*!
	 * \brief Get attribute of "current level" of cluster "Levle Control".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [out] value Current level of this device. The meaning of
	 *              'level' is device dependent.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*level_control_get_value)(
				artik_zigbee_endpoint_handle handle,
				int *value);
	/* ezmode commissioning */
	/*!
	 * \brief Start ezmode commissioning on some endpoint as target
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *
	 * \param [in] handle The handle of endpoint that start do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_NETWORK_STEERING_FORM
	 *                     COMMISSIONING_NETWORK_STEERING_SUCCESS
	 *                     COMMISSIONING_TARGET_SUCCESS
	 *                     COMMISSIONING_TARGET_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_target_start)
				(
					artik_zigbee_endpoint_handle handle
				);

	/*!
	 * \brief Stop ezmode commissioning on some endpoint as target
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *
	 * \param [in] handle The handle of endpoint that stop do ezmode
	 *	       commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 * -Response payload : pointer of enum zigbee_commissioning_state,
	 *                     which including:
	 *                     COMMISSIONING_TARGET_STOP
	 *                     COMMISSIONING_TARGET_FAILED
	 *                     COMMISSIONING_ERROR
	 *                     COMMISSIONING_ERR_IN_PROGRESS
	 */
	 artik_error(*ezmode_commissioning_target_stop)
				(
					artik_zigbee_endpoint_handle handle
				);
} artik_zigbee_device_dimmable_light;

extern const artik_zigbee_device_dimmable_light device_dimmable_light_func;

/*!
 * \brief Structure for device light sensor, to storage the local endpoint
 *        information and functions can be invoked.
 */
typedef struct {
	/* identify */
	/*!
	 * \brief Send command "Identify".
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] duration	Identify duration.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_request)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int duration);
	/*!
	 * \brief Send command "Identify Query", to get remote identify time.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [out] time Remaining seconds of identifying.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_get_remaining_time)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int *time);
	/* Illuminance measurement server */
	/*!
	 * \brief Set the range of illuminance in measured
	 *        value(1 to 65534(0xFFFE))
	 *        Max value shall be greater than min value.
	 *        MeasuredValue represents the Illuminance in Lux (symbol lx)
	 *        as follows:
	 *	  MeasuredValue = 10,000 x log10 Illuminance + 1
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] min Min measured value.
	 * \param [in] max Max measured value.
	 *
	 * \return Result of operation, S_OK on success, error code otherwise..
	 */
	artik_error(*illum_set_measured_value_range)(
					artik_zigbee_endpoint_handle handle,
					int min, int max);
	/*!
	 * \brief Set the measured illuminance value.
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] value Measured value, in the range 1 to 65534(0xfffe),
	 *             which corresponding to illuminance of 1 lx to
	 *	       3.576 x 10^6 lx.
	 *
	 *             MeasuredValue represents the Illuminance in
	 *             Lux (symbol lx) as follows:
	 *             MeasuredValue = 10,000 x log10 Illuminance + 1
	 *
	 *             e.g. To set the sensor to 10 lx, use calculated
	 *                  value 10001.
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*illum_set_measured_value)(
					artik_zigbee_endpoint_handle handle,
					int value);

	/*!
	 * \brief Get the Illuminance value
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [out]	value Measured value, in the range 1 to 65534(0xfffe).
	 *
	 *              MeasuredValue represents the Illuminance in
	 *              Lux (symbol lx) as follows:
	 *              MeasuredValue = 10,000 x log10 Illuminance + 1
	 *
	 *		e.g. The returned value 10001 means 10 lx.
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*illum_get_measured_value)(
					artik_zigbee_endpoint_handle handle,
					int *value);

	/* ezmode commissioning */
	/*!
	 * \brief Start ezmode commissioning on some endpoint as initiator
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *        it includes following step: network steering,
	 *        finding-and-binding this API will do everything inside
	 *        automatically.
	 *
	 * \param [in] handle The handle of endpoint that start do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 *         The result will be delivered through callback function.
	 *         Callback function is called for notifying the result.
	 *         -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 *         -Response payload : pointer of
	 *                             enum zigbee_commissioning_state,
	 *                             which including:
	 *                             COMMISSIONING_NETWORK_STEERING_FORM
	 *                             COMMISSIONING_NETWORK_STEERING_SUCCESS
	 *                             COMMISSIONING_INITIATOR_SUCCESS
	 *                             COMMISSIONING_INITIATOR_FAILED
	 *                             COMMISSIONING_ERROR
	 *                             COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_initiator_start)
				(
					artik_zigbee_endpoint_handle handle
				);

	/*!
	 * \brief Stop ezmode commissioning on some endpoint as initiator
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *        it stop commissioning procedure
	 *
	 * \param [in] handle The handle of endpoint that stop do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 *         The result will be delivered through callback function.
	 *         Callback function is called for notifying the result.
	 *         -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 *         -Response payload : pointer of enum
	 *                             zigbee_commissioning_state,
	 *                             which including:
	 *                             COMMISSIONING_INITIATOR_STOP
	 *                             COMMISSIONING_INITIATOR_FAILED
	 *                             COMMISSIONING_ERROR
	 *                             COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_initiator_stop)
				(
					artik_zigbee_endpoint_handle handle
				);
} artik_zigbee_device_light_sensor;

extern const artik_zigbee_device_light_sensor device_light_sensor_func;

/*!
 * \brief Structure for device remote control, to storage the local endpoint
 *        information and functions can be invoked.
 */
typedef struct {
	/* basic client */
	/*!
	 * \brief Resets all attribute values to factory default.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*reset_to_factory_default)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint);

	/* identify */
	/*!
	 * \brief Send command "Identify".
	 *
	 * \param [in] handle	The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] duration	Identify duration.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_request)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int duration);

	/*!
	 * \brief Send command "Identify Query", to get remote identify time.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [out] time Remaining seconds of identifying.
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*identify_get_remaining_time)(
					artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					int *time);

	/* onoff client */
	/*!
	 * \brief Send command to control remote on/off.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] target_status On/off command, please refer to
	 *             "enum zigbee_onoff_status".
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*onoff_command)(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint * endpoint,
				artik_zigbee_onoff_status target_status);

	/* level control client */
	/*!
	 * \brief Send commands about level control, to control remote level.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] command Level control command, please refer
	 *             to "struct zigbee_level_control_command".
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*level_control_request)(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint * endpoint,
			const artik_zigbee_level_control_command * command);

	/* cluster client */
	/*!
	 * \brief Notice device server to report the attribute.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint	Remote endpoint instance pointer.
	 * \param [in] report_type The reporting type to be requested.
	 *             This parameter in remote control device supports:
	 *             ARTIK_ZIGBEE_REPORTING_THERMOSTAT_TEMPERATURE,
	 *             ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE,
	 *             ARTIK_ZIGBEE_REPORTING_MEASURED_TEMPERATURE
	 *
	 * \param [in] min_interval The reporting minimum interval.
	 * \param [in] max_interval The reporting maximum interval.
	 * \param [in] change_threshold	Minimum changed value to trigger
	 *             reporting.
	 * \return Result of operation, EZ_OK on success, error code otherwise.
	 */
	artik_error(*request_reporting)(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint * endpoint,
					artik_zigbee_reporting_type report_type,
					int min_interval,
					int max_interval,
					int change_threshold);

	/* cluster client */
	/*!
	 * \brief Notice device server to stop the attribute reporting.
	 *
	 * \param [in] handle The handle of each endpoint instance.
	 * \param [in] endpoint Remote endpoint instance pointer.
	 * \param [in] report_type The reporting type to be stopped..
	 *             This parameter in remote control device supports:
	 *             ARTIK_ZIGBEE_REPORTING_THERMOSTAT_TEMPERATURE,
	 *             ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE,
	 *             ARTIK_ZIGBEE_REPORTING_MEASURED_TEMPERATURE
	 *
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	 artik_error(*stop_reporting)(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint * endpoint,
				artik_zigbee_reporting_type report_type);
	/* ezmode commissioning */
	/*!
	 * \brief Start ezmode commissioning on some endpoint as target
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *
	 * \param [in] handle The handle of endpoint that start do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 *         The result will be delivered through callback function.
	 *         Callback function is called for notifying the result.
	 *         -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 *         -Response payload : pointer of enum
	 *                             zigbee_commissioning_state,
	 *                             which including:
	 *                             COMMISSIONING_NETWORK_STEERING_FORM
	 *                             COMMISSIONING_NETWORK_STEERING_SUCCESS
	 *                             COMMISSIONING_TARGET_SUCCESS
	 *                             COMMISSIONING_TARGET_FAILED
	 *                             COMMISSIONING_ERROR
	 *                             COMMISSIONING_ERR_IN_PROGRESS
	 */
	artik_error(*ezmode_commissioning_target_start)
				(
					artik_zigbee_endpoint_handle handle
				);

	/*!
	 * \brief Stop ezmode commissioning on some endpoint as target
	 *        refer to [Home Automation Public Application Profile]
	 *        chapter 8 [Home Automation Commissioning]
	 *
	 * \param [in] handle The handle of endpoint that stop do ezmode
	 *             commissioning
	 * \return Result of operation, S_OK on success, error code otherwise.
	 *         The result will be delivered through callback function.
	 *         Callback function is called for notifying the result.
	 *         -Response type : ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS
	 *         -Response payload : pointer of enum
	 *                             zigbee_commissioning_state,
	 *                             which including:
	 *                             COMMISSIONING_TARGET_STOP
	 *                             COMMISSIONING_TARGET_FAILED
	 *                             COMMISSIONING_ERROR
	 *                             COMMISSIONING_ERR_IN_PROGRESS
	 */
	 artik_error(*ezmode_commissioning_target_stop)
				(
					artik_zigbee_endpoint_handle handle
				);
} artik_zigbee_device_remote_control;

extern const artik_zigbee_device_remote_control device_remote_control_func;

/*!
 * \brief Callback function type
 */
typedef void(*artik_zigbee_client_callback)(void *user_data,
				artik_zigbee_response_type response_type,
				void *payload);

/*! \struct artik_zigbee_module
 *
 *  \brief ZigBee module operations
 *
 *  Structure containing all the exposed operations exposed
 *  by the ZigBee module
 */
typedef struct {
	/*!
	 * \brief Reset local device attribute, binding list and initialize
	 *        network.
	 *
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*reset_local)(void);
	/*!
	 * \brief Set local endpoints, they are saved locally and used to create
	 *        device when initialize() is called.
	 *
	 * \param [in] endpoint_info The local endpoints to be saved
	 *
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*set_local_endpoint)
			(
				artik_zigbee_local_endpoint_info * endpoint_info
			);
	/*!
	 * \brief Get local endpoints which are saved locally when last function
	 *        set_local_endpoint() is called.
	 *
	 * \param [out]	endpoint_info The local endpoints to be returned
	 *
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*get_local_endpoint)
			(
				artik_zigbee_local_endpoint_info * endpoint_info
			);
	/*!
	 * \brief Initialize zigbee module
	 *        This function will not change network setting, and the
	 *        others device in current network will still hold old
	 *        endpoints information. Can let remote devices update saved
	 *        endpoints information by:
	 *        1. remote device re-discover devices and update devices info
	 *           by invoke function device_discover().
	 *        2. local device leave current network and re-join it.
	 *
	 * \param [in] callback	User callback function for receiving
	 *             such a command results
	 * \param [in] user_data Send user data
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*initialize)(artik_zigbee_client_callback callback,
							 void *user_data);
	/*!
	 * \brief Deinitialize, release the data allocated in artik-sdk
	 */
	void (*deinitialize)(void);
	/*!
	 * \brief Resume network operation after a reboot
	 *
	 * This should be called on startup whether or not
	 * the node was previously part of a network.
	 *
	 * \param [out]	state ARTIK_ZIGBEE_JOINED_NETWORK when succeeded
	 *              ARTIK_ZIGBEE_NO_NETWORK if the node is not part of
	 *		a network
	 * \return Result of operation, S_OK on success, error code otherwise.
	 */
	artik_error(*network_start)(artik_zigbee_network_state * state);
	/*!
	 * \brief Form a new network as a coordinator.
	 *
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 *         This begins a search for an unused Channel and Pan Id.
	 *         Then this will automatically form a network on the first
	 *         unused Channel and Pan Id it finds.
	 *         Callback function is called for notifying the result.
	 *         -Response type : ARTIK_RESPONSE_NETWORK_NOTIFICATION
	 *         -Response payload : ARTIK_NETWORK_FIND_FORM (succeeded)
	 */
	artik_error(*network_form)(void);
	/*!
	 * \brief Form a new network as a coordinator
	 * \param [in] network_info Network information for forming network
	 *             including channel, tx power, pan ID
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*network_form_manually)
			(
				const artik_zigbee_network_info *network_info
			);
	/*!
	 * \brief Permit joining to the formed network from other nodes
	 * \param [in] duration	Second value to permit joining, 0xff is
	 *             unlimited
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*network_permitjoin)(int duration_sec);
	/*!
	 * \brief Leave from the joined network
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*network_leave)(void);
	/*!
	 * \brief Join to the existing network by other coordinator
	 *        automatically.
	 *        If we were searching for a joinable network and have
	 *        successfully joined, we give the application some time to
	 *        determine if this is the correct network. If so, we'll
	 *        eventually time out (1 minute) and clean up the state machine.
	 *        If not, user can call network_leave to leave current network,
	 *        and we'll continue searching and joining automatically.
	 *        Besides, user can call network_stop_scan to stop auto network
	 *			searching and joining.
	 *
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 *         Callback function is called for notifying the result.
	 *         -Response type : ARTIK_RESPONSE_NETWORK_NOTIFICATION
	 * -Response payload : ARTIK_NETWORK_FIND_JOIN (succeeded)
	 *                     ARTIK_NETWORK_FIND_JOIN_FAILED (failed)
	 */
	artik_error(*network_join)(void);
	/*!
	 * \brief Stop the network scanning.
	 *        When 'network form', 'network join' and 'network find' are
	 *        called, network scanning is conducted in zigbeed side, this
	 *        api is provided to stop network scanning.
	 *
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*network_stop_scan)(void);
	/*!
	 * \brief Join to the formed network by other coordinator
	 * \param [in] network_info Network information for joining network
	 *             including channel, tx power, pan ID
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*network_join_manually)(
					const artik_zigbee_network_info *
					network_info);
	/*!
	 * /brief Scan existing networks
	 *
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 * The result will be delivered through callback function.
	 * Callback function is called for notifying the result.
	 * -Response type : ARTIK_ZIGBEE_RESPONSE_NETWORK_FIND
	 * -Response payload : the pointer of struct zigbee_network_find
	 */
	artik_error(*network_find)(void);
	/*!
	 * \brief Request my current network status
	 *
	 * \param [out]	state ARTIK_ZIGBEE_JOINED_NETWORK when succeeded
	 *		IGBEE_NO_NETWORK if the node is not part of
	 *              a network
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*network_request_my_network_status)
				(
					artik_zigbee_network_state * state
				);
	/*!
	 * \brief Request device/service discovery
	 *        Device/service discovery is a cyclic call, and the default
	 *        cyclic duration is 1 minute, for the cyclic duration setting,
	 *        please check api 'set_discover_cycle_time'.
	 *        If current api 'device_discover' is called, the discovery is
	 *        triggered immediately.
	 *
	 * \return Result of operation, S_OK when succeeded
	 *         The result will be delivered through callback function.
	 *         Callback function is called for notifying the result.
	 *         -Response type : ARTIK_ZIGBEE_RESPONSE_DEVICE_DISCOVER
	 *         -Response payload : pointer of artik_zigbee_device_discovery.
	 *		               artik_zigbee_device_discovery_status will
	 *                             be sent in different phases.
	 *
	 *                            Initialization phase:
	 *                            ARTIK_ZIGBEE_DEVICE_DISCOVERY_START,
	 *                            ARTIK_ZIGBEE_DEVICE_DISCOVERY_IN_PROGRESS.
	 *                            Discovery phase:
	 *			      ARTIK_ZIGBEE_DEVICE_DISCOVERY_FOUND,
	 *			      ARTIK_ZIGBEE_DEVICE_DISCOVERY_ERROR.
	 *                            Complete phase:
	 *                            ARTIK_ZIGBEE_DEVICE_DISCOVERY_DONE,
	 *                            ARTIK_ZIGBEE_DEVICE_DISCOVERY_NO_DEVICE.
	 */
	artik_error(*device_discover)(void);
	/*!
	 * \brief Set the cyclic duaration of device discovery cycle
	 * \param[in] time_minutes The cyclic duaration, in minutes, positive
	 *            value. The default cyclic duration is 1 minute.
	 *            If the value is equal to 0 , the loop timer is stopped.
	 *
	 * \return Result of operation, S_OK when succeeded
	 */
	artik_error(*set_discover_cycle_time)(unsigned int time_minutes);
	/*!
	 * \brief Get device info from zigbee daemon
	 *
	 * \param[out] device_info the memory pointer for return device info
	 *             list.
	 * \return Result of operation, S_OK when succeeded, E_ZIGBEE_NO_DEVICE
	 *         when no discovered device info returned, otherwise failed.
	 */
	artik_error(*get_discovered_device_list)
				(
					artik_zigbee_device_info * device_info
				);
	/*!
	 * \brief Request my current device type
	 *
	 * \param [out] type current device type
	 * \return Result of operation, S_OK when succeeded, otherwise failed.
	 */
	artik_error(*device_request_my_node_type)(artik_zigbee_node_type
									* type);
	/*!
	 * \brief Find endpoint list to filtered by cluster ID and
	 *        SERVER/CLIENT
	 * \param [out] endpoints Endpoint list that are matched by other
	 *              parameters
	 * \param [in] cluster_id Cluster ID that is defined in
	 *             artik-cluster-id.h
	 * \param [in] is_server 1 for SERVER, 0 for CLIENT
	 */
	void (*device_find_by_cluster)(artik_zigbee_endpoint_list *endpoints,
						int cluster_id,	int is_server);
	/*!
	 * \brief Send Command Line Interface(CLI) command
	 *        This is normally used for certification testing
	 * \param [in] command String of CLI command
	 */
	void (*raw_request)(const char *command);
} artik_zigbee_module;

extern const artik_zigbee_module zigbee_module;

#ifdef __cplusplus
}
#endif
#endif				/* __ARTIK_ZIGBEE_H__ */
