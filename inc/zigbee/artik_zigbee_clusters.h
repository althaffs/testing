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

#ifndef	__ARTIK_ZIGBEE_CLUSTERS_H__
#define	__ARTIK_ZIGBEE_CLUSTERS_H__

#ifdef __cplusplus
extern "C" {
#endif
/*! \file artik_zigbee_clusters.h
 *
 *	\brief ZIGBEE module definition
 *
 *	Definitions and functions for accessing
 *	the ZIGBEE Clusters
 *
 */

/*!
 * \brief Group command for Group cluster
 *        This is used when both sending and receiving command
 */
typedef enum {
	ARTIK_ZIGBEE_GROUPS_ADD = 3230,
	ARTIK_ZIGBEE_GROUPS_REMOVE,
	ARTIK_ZIGBEE_GROUPS_REMOVE_ALL,
	ARTIK_ZIGBEE_GROUPS_ADD_IF_IDENTIFYING
} artik_zigbee_group_command;
/*!
 * \brief Structure for receiving a notification of group command from
 *        ZigBee Daemon
 */
typedef struct {
	unsigned short group_id;
	int endpoint_id;
	artik_zigbee_group_command group_cmd;
} artik_zigbee_groups_info;
/*!
 * \brief Structure for saving Scene information
 */
typedef struct {
	unsigned char scene_count;
	unsigned char current_scene;
	unsigned short current_group;
	unsigned char scene_valid;
} artik_zigbee_scene_mgmt_info;
/*!
 * \brief On/Off status for On/Off cluster
 *        This is used when both sending and receiving command
 */
typedef enum {
	ARTIK_ZIGBEE_ONOFF_OFF = 3220,
	ARTIK_ZIGBEE_ONOFF_ON,
	ARTIK_ZIGBEE_ONOFF_TOGGLE
} artik_zigbee_onoff_status;
/*!
 * \brief Structure for receiving a command notification of On/Off cluster
 *        from ZigBee Daemon
 */
typedef struct {
	artik_zigbee_onoff_status prev_value;
	artik_zigbee_onoff_status curr_value;
	artik_zigbee_onoff_status command;
	int endpoint_id;
} artik_zigbee_onoff_info;
/*!
 * \brief Level Control cluster command type
 */
typedef enum {
	ARTIK_ZIGBEE_MOVE_TO_LEVEL = 0x00,
	ARTIK_ZIGBEE_MOVE = 0x01,
	ARTIK_ZIGBEE_STEP = 0x02,
	ARTIK_ZIGBEE_STOP = 0x03,
	ARTIK_ZIGBEE_MOVE_TO_LEVEL_ONOFF = 0x04,
	ARTIK_ZIGBEE_MOVE_ONOFF = 0x05,
	ARTIK_ZIGBEE_STEP_ONOFF = 0x06,
	ARTIK_ZIGBEE_STOP_ONOFF = 0x07
} artik_zigbee_level_control_type;
/*!
 * \brief Level Control cluster command mode
 */
typedef enum {
	ARTIK_ZIGBEE_LEVEL_CONTROL_UP = 0x00,
	ARTIK_ZIGBEE_LEVEL_CONTROL_DOWN = 0x01
} artik_zigbee_level_control_mode;
/*!
 * \brief Structure for Level Control cluster command
 */
typedef struct {
	artik_zigbee_level_control_type control_type;
	union {
		struct {
			int level;
			int transition_time; /* tenths of a second */
		} move_to_level;
		struct {
			artik_zigbee_level_control_mode control_mode;
			int rate; /* changes per second */
		} move;
		struct {
			artik_zigbee_level_control_mode control_mode;
			int step_size;
			int transition_time; /* tehnths of a second */
		} step;
	} parameters;
} artik_zigbee_level_control_command;
/*!
 * \brief Structure for receiving a update notification of Level Control
 *        cluster from ZigBee Daemon
 */
typedef struct {
	int prev_level;
	int curr_level;
	int endpoint_id;
} artik_zigbee_level_control_update;
/*!
 * \brief Color Control cluster command type
 */
typedef enum {
	ARTIK_ZIGBEE_MOVE_TO_HUE = 0x00,
	ARTIK_ZIGBEE_MOVE_HUE,
	ARTIK_ZIGBEE_STEP_HUE,
	ARTIK_ZIGBEE_MOVE_TO_SATURATION,
	ARTIK_ZIGBEE_MOVE_SATURATION,
	ARTIK_ZIGBEE_STEP_SATURATION,
	ARTIK_ZIGBEE_MOVE_TO_HUE_SATURATION,
	ARTIK_ZIGBEE_MOVE_TO_COLOR,
	ARTIK_ZIGBEE_MOVE_COLOR,
	ARTIK_ZIGBEE_STEP_COLOR,
	ARTIK_ZIGBEE_MOVE_TO_COLOR_TEMP,
	ARTIK_ZIGBEE_EMOVE_TO_HUE = 0x40,
	ARTIK_ZIGBEE_EMOVE_HUE,
	ARTIK_ZIGBEE_ESTEP_HUE,
	ARTIK_ZIGBEE_EMOVE_TO_HUE_SATURATION,
	ARTIK_ZIGBEE_COLOR_LOOP_SET,
	ARTIK_ZIGBEE_STOP_MOVE_STEP = 0x47,
	ARTIK_ZIGBEE_MOVE_COLOR_TEMP = 0x4b,
	ARTIK_ZIGBEE_STEP_COLOR_TEMP = 0x4c
} artik_zigbee_color_control_type;
/*!
 * \brief Color Control cluster command direction
 */
typedef enum {
	ARTIK_ZIGBEE_COLOR_CONTROL_DIRECTION_SHORTEST_DISTANCE = 0x00,
	ARTIK_ZIGBEE_COLOR_CONTROL_DIRECTION_LONGEST_DISTANCE = 0x01,
	ARTIK_ZIGBEE_COLOR_CONTROL_DIRECTION_UP = 0x02,
	ARTIK_ZIGBEE_COLOR_CONTROL_DIRECTION_DOWN = 0x03
} artik_zigbee_color_control_direction;
/*!
 * \brief The move mode for Color Control
 */
typedef enum {
	ARTIK_ZIGBEE_COLOR_CONTROL_MOVE_MODE_STOP = 0x00,
	ARTIK_ZIGBEE_COLOR_CONTROL_MOVE_MODE_UP = 0x01,
	ARTIK_ZIGBEE_COLOR_CONTROL_MOVE_MODE_DOWN = 0x03
} artik_zigbee_color_control_move_mode;
/*!
 * \brief The step mode for Color Control
 */
typedef enum {
	ARTIK_ZIGBEE_COLOR_CONTROL_STEP_MODE_UP = 0x01,
	ARTIK_ZIGBEE_COLOR_CONTROL_STEP_MODE_DOWN = 0x03
} artik_zigbee_color_control_step_mode;
/*!
 * \brief The loop action for Color Control
 */
typedef enum {
ARTIK_ZIGBEE_COLOR_CONTROL_LOOP_ACTION_DEACTIVATE = 0x00,
ARTIK_ZIGBEE_COLOR_CONTROL_LOOP_ACTION_ACTIVATE_FROM_START_ENHANCED_HUE	= 0x01,
ARTIK_ZIGBEE_COLOR_CONTROL_LOOP_ACTION_ACTIVATE_FROM_ENHANCED_CURRENT_HUE = 0x02
} artik_zigbee_color_control_loop_set_action;
/*!
 * \brief The command for cluster Color Control
 */
typedef struct {
	artik_zigbee_color_control_type control_type;
	union {
		struct {
			int hue;
			artik_zigbee_color_control_direction direction;
			int transition_time;
		} move_to_hue;
		struct {
			artik_zigbee_color_control_move_mode move_mode;
			int rate;
		} move_hue;
		struct {
			artik_zigbee_color_control_step_mode step_mode;
			int step_size;
			int transition_time;
		} step_hue;
		struct {
			int saturation;
			int transition_time;
		} move_to_sat;
		struct {
			artik_zigbee_color_control_move_mode move_mode;
			int rate;
		} move_sat;
		struct {
			artik_zigbee_color_control_step_mode step_mode;
			int step_size;
			int transition_time;
		} step_sat;
		struct {
			int hue;
			int saturation;
			int transition_time;
		} move_to_hue_sat;
		struct {
			int color_x;
			int color_y;
			int transition_time;
		} move_to_color;
		struct {
			int rate_x;
			int rate_y;
		} move_color;
		struct {
			int step_x;
			int step_y;
			int transition_time;
		} step_color;
		struct {
			int color_temp;
			int transition_time;
		} move_to_color_temp;
		struct {
			int update_flag;
			artik_zigbee_color_control_loop_set_action action;
			artik_zigbee_color_control_direction direction;
			int time;
			int start_hue;
		} color_loop_set;
		struct {
			artik_zigbee_color_control_move_mode move_mode;
			int rate;
			int min_color_temp;
			int max_color_temp;
		} move_color_temp;
		struct {
			artik_zigbee_color_control_step_mode step_mode;
			int step_size;
			int transition_time;
			int min_color_temp;
			int max_color_temp;
		} step_color_temp;
	} parameters;
} artik_zigbee_color_control_command;
/*!
 * \brief The Color current values
 */
typedef struct {
	int hue;
	int saturation;
	int color_x;
	int color_y;
	int color_temp;
} artik_zigbee_color_control_value;

/*!
 * \brief The status of occupancy
 */
typedef enum {
	ARTIK_ZIGBEE_OCCUPIED,
	ARTIK_ZIGBEE_UNOCCUPIED
} artik_zigbee_occupancy_status;

/*!
 * \brief The sensor type of occupancy
 */
typedef enum {
	ARTIK_ZIGBEE_OCCUPANCY_PIR,
	ARTIK_ZIGBEE_OCCUPANCY_ULTRASONIC,
	ARTIK_ZIGBEE_OCCUPANCY_PIR_ULTRASONIC
} artik_zigbee_occupancy_type;

typedef enum {
	ARTIK_ZIGBEE_FAN_MODE_OFF,
	ARTIK_ZIGBEE_FAN_MODE_LOW,
	ARTIK_ZIGBEE_FAN_MODE_MEDIUM,
	ARTIK_ZIGBEE_FAN_MODE_HIGH,
	ARTIK_ZIGBEE_FAN_MODE_ON,
	ARTIK_ZIGBEE_FAN_MODE_AUTO,
	ARTIK_ZIGBEE_FAN_MODE_SMART,
	ARTIK_ZIGBEE_FAN_MODE_RESERVED
} artik_zigbee_fan_mode;

typedef enum {
	ARTIK_ZIGBEE_FAN_MODE_SEQUENCE_LOW_MED_HIGH,
	ARTIK_ZIGBEE_FAN_MODE_SEQUENCE_LOW_HIGH,
	ARTIK_ZIGBEE_FAN_MODE_SEQUENCE_LOW_MED_HIGH_AUTO,
	ARTIK_ZIGBEE_FAN_MODE_SEQUENCE_LOW_HIGH_AUTO,
	ARTIK_ZIGBEE_FAN_MODE_SEQUENCE_ON_AUTO,
	ARTIK_ZIGBEE_FAN_MODE_SEQUENCE_RESERVED
} artik_zigbee_fan_mode_sequence;

/*!
 * \brief The Thermostat setpoint mode
 */
typedef enum {
	ARTIK_ZIGBEE_SETPOINT_MODE_HEAT = 0x00,
	ARTIK_ZIGBEE_SETPOINT_MODE_COOL,
	ARTIK_ZIGBEE_SETPOINT_MODE_BOTH,
	ARTIK_ZIGBEE_SETPOINT_MODE_RESERVED
} artik_zigbee_thermostat_setpoint_mode;

/*!
 * \brief The Thermostat system mode
 */
typedef enum {
	ARTIK_ZIGBEE_SYSTEM_MODE_OFF = 0x00,
	ARTIK_ZIGBEE_SYSTEM_MODE_AUTO,
	ARTIK_ZIGBEE_SYSTEM_MODE_COOL,
	ARTIK_ZIGBEE_SYSTEM_MODE_HEAT,
	ARTIK_ZIGBEE_SYSTEM_MODE_EMERGENCY_HEATING,
	ARTIK_ZIGBEE_SYSTEM_MODE_PRECOOLING,
	ARTIK_ZIGBEE_SYSTEM_MODE_FAN_ONLY,
	ARTIK_ZIGBEE_SYSTEM_MODE_RESERVED
} artik_zigbee_thermostat_system_mode;

/*!
 * \brief The Thermostat control sequence of operation
 */
typedef enum {
	ARTIK_ZIGBEE_CONTROL_SEQUENCE_COOLING_ONLY = 0x00,
	ARTIK_ZIGBEE_CONTROL_SEQUENCE_COOLING_WITH_REHEAT,
	ARTIK_ZIGBEE_CONTROL_SEQUENCE_HEATING_ONLY,
	ARTIK_ZIGBEE_CONTROL_SEQUENCE_HEATING_WITH_REHEAT,
	ARTIK_ZIGBEE_CONTROL_SEQUENCE_COOLING_HEATING,
	ARTIK_ZIGBEE_CONTROL_SEQUENCE_COOLING_HEATING_WITH_REHEAT,
	ARTIK_ZIGBEE_CONTROL_SEQUENCE_RESERVED
} artik_zigbee_thermostat_control_sequence;

#ifdef __cplusplus
}
#endif
#endif	/* __ARTIK_ZIGBEE_CLUSTERS_H__ */

