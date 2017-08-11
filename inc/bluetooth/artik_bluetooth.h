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

#ifndef __ARTIK_BLUETOOTH_H
#define __ARTIK_BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "artik_error.h"
#include "artik_types.h"

	/*! \file artik_bluetooth.h
	 *
	 * \brief Bluetooth module definition
	 *
	 * Definitions and functions for accessing
	 * the Bluetooth modules
	 *
	 * \example bluetooth_test/artik_bluetooth_test.c
	 * \example bluetooth_test/artik_bluetooth_test_agent.c
	 * \example bluetooth_test/artik_bluetooth_test_advertiser.c
	 * \example bluetooth_test/artik_bluetooth_test_avrcp.c
	 * \example bluetooth_test/artik_bluetooth_test_ftp.c
	 * \example bluetooth_test/artik_bluetooth_test_gatt_client.c
	 * \example bluetooth_test/artik_bluetooth_test_gatt_server.c
	 * \example bluetooth_test/artik_bluetooth_test_nap.c
	 * \example bluetooth_test/artik_bluetooth_test_panu.c
	 * \example bluetooth_test/artik_bluetooth_test_spp.c
	 * \example bluetooth_test/artik_bluetooth_test_spp_client.c
	 * \example bluetooth_test/artik_bluetooth_test_commandline.c
	 * \example bluetooth_test/artik_bluetooth_test_commandline.h
	 */

#define MAX_BT_NAME_LEN		128
#define MAX_BT_ADDR_LEN		128
#define MAX_BT_UUID_LEN		128

#define BT_ADV_TYPE_BROADCAST "broadcast"
#define BT_ADV_TYPE_PERIPHERAL "peripheral"

	/*!
	 * \brief This enumeration reflects the input and output capabilities of the agent.
	 */
	typedef enum {
		BT_CAPA_KEYBOARDDISPLAY = 0, /*<! The agent is able to display and to write text*/
		BT_CAPA_DISPLAYONLY, /*<! The agent is only able to display text*/
		BT_CAPA_DISPLAYYESNO, /*<! The agent is able to display and confirm with yes/no*/
		BT_CAPA_KEYBOARDONLY, /*<! The agent is only able to write text*/
		BT_CAPA_NOINPUTNOOUTPUT, /*<! The agent has not input or output capability */
		BT_CAPA_END
	} artik_bt_agent_capability;

	/*!
	 * \brief All events come from the Bluetooth module.
	 */
	typedef enum {
		BT_EVENT_SCAN = 0, /*<! This event is raised when a Bluetooth device is discovered*/
		BT_EVENT_BOND, /*<! This event is raised when a Bluetooth device is paired/unpaired*/
		BT_EVENT_CONNECT, /*<! This event is raised when a Bluetooth device is connected/disconnected*/
		BT_EVENT_PROXIMITY,
		BT_EVENT_PF_HEARTRATE, /*<! This event is raised when heart rate data is received. */
		BT_EVENT_FTP, /*<! This event is raised to monitor a FTP transfer. */
		BT_EVENT_GATT_PROPERTY, /*<! This event is raised when a gatt property changed. */
		BT_EVENT_PF_CUSTOM, /*<! This event is raised when custom gatt data is received. */
		BT_EVENT_END
	} artik_bt_event;

	/*!
	 * \brief Event callback function prototype.
	 *
	 * \param[in] event The type of event raised.
	 * \param[in] data This parameter contains data that describe an event.
	 *                   Event type           | Data type
	 *                 ----------------------------------------
	 *                 BT_EVENT_SCAN          | \ref artik_bt_device
	 *                 BT_EVENT_BOND          | A boolean, true if the device is paired, false otherwise.
	 *                 BT_EVENT_CONNECT       | A bollean, true if the device is connected, false otherwise.
	 *                 BT_EVENT_PROXIMITY     | \ref artik_bt_gatt_data
	 *                 BT_EVENT_PF_HEARTRATE  | \ref artik_bt_hrp_data
	 *                 BT_EVENT_FTP           | \ref artik_bt_ftp_property
	 *                 BT_EVENT_GATT_PROPERTY | not used
	 *                 BT_EVENT_PF_CUSTOM     | \ref artik_bt_gatt_data
	 * \param[in] user_data The user data passed from the \ref set_callback.
	 */
	typedef void (*artik_bt_callback) (artik_bt_event event, void *data,
			void *user_data);

	/*!
	 * \brief Enumerations of major device class.
	 */
	typedef enum {
		/*<! Miscellaneous major device class */
		BT_MAJOR_DEVICE_CLASS_MISC = 0x00,
		/*<! Computer major device class */
		BT_MAJOR_DEVICE_CLASS_COMPUTER = 0x01,
		/*<! Phone major device class */
		BT_MAJOR_DEVICE_CLASS_PHONE = 0x02,
		/*<! LAN/Network access point major device class */
		BT_MAJOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT = 0x03,
		/*<! Audio/Video major device class */
		BT_MAJOR_DEVICE_CLASS_AUDIO_VIDEO = 0x04,
		/*<! Peripheral major device class */
		BT_MAJOR_DEVICE_CLASS_PERIPHERAL = 0x05,
		/*<! Imaging major device class */
		BT_MAJOR_DEVICE_CLASS_IMAGING = 0x06,
		/*<! Wearable device class */
		BT_MAJOR_DEVICE_CLASS_WEARABLE = 0x07,
		/*<! Toy device class */
		BT_MAJOR_DEVICE_CLASS_TOY = 0x08,
		/*<! Health device class */
		BT_MAJOR_DEVICE_CLASS_HEALTH = 0x09,
		/*<! Uncategorized major device class */
		BT_MAJOR_DEVICE_CLASS_UNCATEGORIZED = 0x1F
	} artik_bt_major_device_class;

	/*!
	 * \brief Enumerations of minor device class.
	 */
	typedef enum {
		/*<! Uncategorized minor device class of computer */
		BT_MINOR_DEVICE_CLASS_COMPUTER_UNCATEGORIZED = 0x00,
		/*<! Desktop workstation minor device class of computer */
		BT_MINOR_DEVICE_CLASS_COMPUTER_DESKTOP_WORKSTATION = 0x04,
		/*<! Server minor device class of computer */
		BT_MINOR_DEVICE_CLASS_COMPUTER_SERVER_CLASS = 0x08,
		/*<! Laptop minor device class of computer */
		BT_MINOR_DEVICE_CLASS_COMPUTER_LAPTOP = 0x0C,
		/*<! Handheld PC/PDA minor device class of computer */
		BT_MINOR_DEVICE_CLASS_COMPUTER_HANDHELD_PC_OR_PDA = 0x10,
		/*<! Palm sized PC/PDA minor device class of computer */
		BT_MINOR_DEVICE_CLASS_COMPUTER_PALM_SIZED_PC_OR_PDA = 0x14,
		/*<! Wearable(watch sized) minor device class of computer */
		BT_MINOR_DEVICE_CLASS_COMPUTER_WEARABLE_COMPUTER = 0x18,

		/*<! Uncategorized minor device class of phone */
		BT_MINOR_DEVICE_CLASS_PHONE_UNCATEGORIZED = 0x00,
		/*<! Cellular minor device class of phone */
		BT_MINOR_DEVICE_CLASS_PHONE_CELLULAR = 0x04,
		/*<! Cordless minor device class of phone */
		BT_MINOR_DEVICE_CLASS_PHONE_CORDLESS = 0x08,
		/*<! Smart phone minor device class of phone */
		BT_MINOR_DEVICE_CLASS_PHONE_SMART_PHONE = 0x0C,
		/*<! Wired modem or voice gateway minor device class of phone */
		BT_MINOR_DEVICE_CLASS_PHONE_WIRED_MODEM_OR_VOICE_GATEWAY = 0x10,
		/*<! Common ISDN minor device class of phone */
		BT_MINOR_DEVICE_CLASS_PHONE_COMMON_ISDN_ACCESS = 0x14,

		/*<! Fully available minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_FULLY_AVAILABLE = 0x04,
		/*<! 1-17% utilized minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_1_TO_17_PERCENT_UTILIZED = 0x20,
		/*<! 17-33% utilized minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_17_TO_33_PERCENT_UTILIZED = 0x40,
		/*<! 33-50% utilized minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_33_TO_50_PERCENT_UTILIZED = 0x60,
		/*<! 50-67% utilized minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_50_to_67_PERCENT_UTILIZED = 0x80,
		/*<! 67-83% utilized minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_67_TO_83_PERCENT_UTILIZED = 0xA0,
		/*<! 83-99% utilized minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_83_TO_99_PERCENT_UTILIZED = 0xC0,
		/*<! No service available minor device class of LAN/Network access point */
		BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_NO_SERVICE_AVAILABLE = 0xE0,

		/*<! Uncategorized minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_UNCATEGORIZED = 0x00,
		/*<! Wearable headset minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_WEARABLE_HEADSET = 0x04,
		/*<! Hands-free minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HANDS_FREE = 0x08,
		/*<! Microphone minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_MICROPHONE = 0x10,
		/*<! Loudspeaker minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_LOUDSPEAKER = 0x14,
		/*<! Headphones minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HEADPHONES = 0x18,
		/*<! Portable audio minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_PORTABLE_AUDIO = 0x1C,
		/*<! Car audio minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_CAR_AUDIO = 0x20,
		/*<! Set-top box minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_SET_TOP_BOX = 0x24,
		/*<! Hifi audio minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HIFI_AUDIO_DEVICE = 0x28,
		/*<! VCR minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VCR = 0x2C,
		/*<! Video camera minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_CAMERA = 0x30,
		/*<! Camcorder minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_CAMCORDER = 0x34,
		/*<! Video monitor minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_MONITOR = 0x38,
		/*<! Video display and loudspeaker minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_DISPLAY_LOUDSPEAKER = 0x3C,
		/*<! Video conferencing minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_CONFERENCING = 0x40,
		/*<! Gaming/toy minor device class of audio/video */
		BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_GAMING_TOY = 0x48,

		/*<! Uncategorized minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERA_UNCATEGORIZED = 0x00,
		/*<! Key board minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_KEY_BOARD = 0x40,
		/*<! Pointing device minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_POINTING_DEVICE = 0x80,
		/*<! Combo keyboard or pointing device minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_COMBO_KEYBOARD_POINTING_DEVICE = 0xC0,
		/*<! Joystick minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_JOYSTICK = 0x04,
		/*<! Game pad minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_GAME_PAD = 0x08,
		/*<! Remote control minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_REMOTE_CONTROL = 0x0C,
		/*<! Sensing device minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_SENSING_DEVICE = 0x10,
		/*<! Digitizer minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_DIGITIZER_TABLET = 0x14,
		/*<! Card reader minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_CARD_READER = 0x18,
		/*<! Digital pen minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_DIGITAL_PEN = 0x1C,
		/*<! Handheld scanner minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_HANDHELD_SCANNER = 0x20,
		/*<! Handheld gestural input device minor device class of peripheral */
		BT_MINOR_DEVICE_CLASS_PERIPHERAL_HANDHELD_GESTURAL_INPUT_DEVICE = 0x24,

		/*<! Display minor device class of imaging */
		BT_MINOR_DEVICE_CLASS_IMAGING_DISPLAY = 0x10,
		/*<! Camera minor device class of imaging */
		BT_MINOR_DEVICE_CLASS_IMAGING_CAMERA = 0x20,
		/*<! Scanner minor device class of imaging */
		BT_MINOR_DEVICE_CLASS_IMAGING_SCANNER = 0x40,
		/*<! Printer minor device class of imaging */
		BT_MINOR_DEVICE_CLASS_IMAGING_PRINTER = 0x80,

		/*<! Wrist watch minor device class of wearable */
		BT_MINOR_DEVICE_CLASS_WEARABLE_WRIST_WATCH = 0x04,
		/*<! Pager minor device class of wearable */
		BT_MINOR_DEVICE_CLASS_WEARABLE_PAGER = 0x08,
		/*<! Jacket minor device class of wearable */
		BT_MINOR_DEVICE_CLASS_WEARABLE_JACKET = 0x0C,
		/*<! Helmet minor device class of wearable */
		BT_MINOR_DEVICE_CLASS_WEARABLE_HELMET = 0x10,
		/*<! Glasses minor device class of wearable */
		BT_MINOR_DEVICE_CLASS_WEARABLE_GLASSES = 0x14,

		/*<! Robot minor device class of toy */
		BT_MINOR_DEVICE_CLASS_TOY_ROBOT = 0x04,
		/*<! Vehicle minor device class of toy */
		BT_MINOR_DEVICE_CLASS_TOY_VEHICLE = 0x08,
		/*<! Doll/Action minor device class of toy */
		BT_MINOR_DEVICE_CLASS_TOY_DOLL_ACTION = 0x0C,
		/*<! Controller minor device class of toy */
		BT_MINOR_DEVICE_CLASS_TOY_CONTROLLER = 0x10,
		/*<! Game minor device class of toy */
		BT_MINOR_DEVICE_CLASS_TOY_GAME = 0x14,

		/*<! Uncategorized minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_UNCATEGORIZED = 0x00,
		/*<! Blood pressure monitor minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_BLOOD_PRESSURE_MONITOR = 0x04,
		/*<! Thermometer minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_THERMOMETER = 0x08,
		/*<! Weighing scale minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_WEIGHING_SCALE = 0x0C,
		/*<! Glucose minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_GLUCOSE_METER = 0x10,
		/*<! Pulse oximeter minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_PULSE_OXIMETER = 0x14,
		/*<! Heart/Pulse rate monitor minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_HEART_PULSE_RATE_MONITOR = 0x18,
		/*<! Health data display minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_DATA_DISPLAY = 0x1C,
		/*<! Step counter minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_STEP_COUNTER = 0x20,
		/*<! Body composition analyzer minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_BODY_COMPOSITION_ANALYZER = 0x24,
		/*<! Peak flow monitor minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_PEAK_FLOW_MONITOR = 0x28,
		/*<! Medication monitor minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_MEDICATION_MONITOR = 0x2C,
		/*<! Knee prosthesis minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_KNEE_PROSTHESIS = 0x30,
		/*<! Ankle prosthesis minor device class of health */
		BT_MINOR_DEVICE_CLASS_HEALTH_ANKLE_PROSTHESIS = 0x34
	} artik_bt_minor_device_class;

	/*!
	 * \brief Service class part of class of device returned from device discovery
	 */
	typedef enum {
		BT_SERVICE_CLASS_LIMITED_DISCOVERABLE_MODE = 0x002000,
		BT_SERVICE_CLASS_POSITIONING = 0x010000,
		BT_SERVICE_CLASS_NETWORKING = 0x020000,
		BT_SERVICE_CLASS_RENDERING = 0x040000,
		BT_SERVICE_CLASS_CAPTURING = 0x080000,
		BT_SERVICE_CLASS_OBJECT_TRANSFER = 0x100000,
		BT_SERVICE_CLASS_AUDIO = 0x200000,
		BT_SERVICE_CLASS_TELEPHONY = 0x400000,
		BT_SERVICE_CLASS_INFORMATION = 0x800000
	} artik_bt_device_service_class;


	/*!
	 * \brief Class structure of device and service.
	 */
	typedef struct {
		/*<! Major device class. */
		artik_bt_major_device_class major;
		/*<! Minor device class. */
		artik_bt_minor_device_class minor;
		/*<! Device service class */
		artik_bt_device_service_class service_class;
	} artik_bt_class;

	/*!
	 *  \brief Bluetooth UUID definition
	 *
	 *  Structure containing the elements
	 *  defining Bluetooth profile UUID
	 */
	typedef struct {
		char *uuid; /*<! UUID of the profile*/
		char *uuid_name; /*<! The friendly name of the profile */
	} artik_bt_uuid;

	/*!
	 * \brief Characteristic property
	 */
	typedef enum {
		/*<! The characteristic is broadcastable */
		BT_GATT_CHAR_PROPERTY_BROADCAST = 0x01,
		/*<! The characteristic is readable*/
		BT_GATT_CHAR_PROPERTY_READ = 0x02,
		/*<! The characteristic can be written without response */
		BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE = 0x04,
		/*<! The characteristic is writable*/
		BT_GATT_CHAR_PROPERTY_WRITE = 0x08,
		/*<! The characteristic supports notification */
		BT_GATT_CHAR_PROPERTY_NOTIFY = 0x10,
		/*<! The characteristic supports indication*/
		BT_GATT_CHAR_PROPERTY_INDICATE = 0x20,
		/*<! The characteristic supports write with signature */
		BT_GATT_CHAR_PROPERTY_SIGNED_WRITE = 0x40
	} artik_bt_gatt_char_properties;

	/*! \brief Descriptor property
	 *
	 */
	typedef enum {
		/*<! The descriptor is readable*/
		BT_GATT_DESC_PROPERTY_READ = 0x01,
		/*<! The descriptor is writable */
		BT_GATT_DESC_PROPERTY_WRITE = 0x02,
		/*<! The descriptor supports encrypted read */
		BT_GATT_DESC_PROPERTY_ENC_READ = 0x04,
		/*<! The descriptor supports encrypted write */
		BT_GATT_DESC_PROPERTY_ENC_WRITE = 0x08,
		/*<! The descriptor supports encrypted and authenticated read */
		BT_GATT_DESC_PROPERTY_ENC_AUTH_READ = 0x10,
		/*<! The descriptor supports encrypted and authenticated write */
		BT_GATT_DESC_PROPERTY_ENC_AUTH_WRITE = 0x20,
		/*<! The descriptor supports secure read */
		BT_GATT_DESC_PROPERTY_SEC_READ = 0x40,
		/*<! The descriptor supports secure write */
		BT_GATT_DESC_PROPERTY_SEC_WRITE = 0x80
	} artik_bt_gatt_desc_properties;

	/*!
	 * \brief Request status
	 */
	typedef enum {
		/*<! The request is successful*/
		BT_GATT_REQ_STATE_TYPE_OK,
		/*<! The request is failed*/
		BT_GATT_REQ_STATE_TYPE_FAILED,
		/*<! The request is in progress*/
		BT_GATT_REQ_STATE_TYPE_IN_PROGRESS,
		/*<! The request is not permitted */
		BT_GATT_REQ_STATE_TYPE_NOT_PERMITTED,
		/*<! The request has an invalid value length */
		BT_GATT_REQ_STATE_TYPE_INVALID_VALUE_LENGTH,
		/*<! The request is not authorized */
		BT_GATT_REQ_STATE_TYPE_NOT_AUTHORIZED,
		/*<! The request is not supported */
		BT_GATT_REQ_STATE_TYPE_NOT_SUPPORTED
	} artik_bt_gatt_req_state_type;

	/*!
	 *  \brief Bluetooth GATT data definition
	 *
	 *  Structure containing the elements
	 *  defining GATT related date
	 */
	typedef struct {
		/*<! GATT characteristic UUID */
		char *char_uuid;
		/*<! GATT service UUID */
		char *srv_uuid;
		/*<! GATT property key */
		char *key;
		/*<! GATT property value */
		char *value;
		/*<! The value of the characteristic*/
		unsigned char *bytes;
		/*<! Number of byte in bytes */
		int length;
	} artik_bt_gatt_data;

	/*!
	 * \brief GATT service definition
	 */
	typedef struct {
		const char *uuid; /*<! 128-bit service UUID */
		bool primary; /*<! If true, this GATT service is a primary service */
	} artik_bt_gatt_service;

	/*! \brief GATT characteristic definition
	 *
	 */
	typedef struct {
		char *uuid; /*<! 128-bit characteristic UUID */
		unsigned char *value; /*<! The initial value of the GATT characteristic */
		int length; /*<! The value's length */
		int property; /*<! The GATT characteristic property see \ref artik_bt_gatt_char_properties */
	} artik_bt_gatt_chr;

	/*! \brief GATT descriptor definition
	 *
	 */
	typedef struct {
		char *uuid; /*<! 128-bit descriptor UUID */
		unsigned char *value; /*<! The initial value of the GATT descriptor */
		int length; /*<! The value's length */
		int property; /*<! The GATT descriptor property see \ref artik_bt_gatt_desc_properties */
	} artik_bt_gatt_desc;

	/*! \brief GATT request handle type
	 *
	 * This type is used to carry specific infomations about a GATT request.
	 * The application must give a response to the bluetooth service with the functions
	 * \ref gatt_req_set_value and \ref gatt_req_set_result.
	 */
	typedef void *artik_bt_gatt_req;

	/*! \brief Method called when a client performs write on a characteristic.
	 *
	 * Use \ref gatt_req_set_result to accept or reject the request.
	 *
	 * \param[in] request Handle tied to a GATT request
	 * \param[in] value The value of the GATT request
	 * \param[in] len The size of value
	 * \param[in] user_data The user data passed from the \ref gatt_set_char_on_write_request
	 *                        or \ref gatt_set_desc_on_write_request function.
	 */
	typedef void (*artik_bt_gatt_req_write)(artik_bt_gatt_req request,
						const unsigned char *value,
						int len,
						void *user_data);

	/*! \brief Method called when a client performs read on a characteristic
	 *
	 * Use \ref gatt_req_set_value to send a response to the client or
	 * \ref gatt_req_set_result to return error.
	 *
	 * \param[in] request Handle tied to a GATT request
	 * \param [in] user_data The user data passed from the \ref gatt_set_char_on_read_request
	 *                        or \ref gatt_set_desc_on_read_request function.
	 */
	typedef void (*artik_bt_gatt_req_read)(artik_bt_gatt_req request,
					       void *user_data);

	/*! \brief Method called when a client wants subscribe or unsubscribe to GATT notification.
	 *
	 * \param[in] state True when the client subscribe, false otherwise.
	 * \param [in] user_data The user data passed from the \ref gatt_set_char_on_read_request function.
	 */
	typedef void (*artik_bt_gatt_req_notify)(bool state,
						 void *user_data);

	/*!
	 *  \brief Bluetooth device definition
	 *
	 *  Structure containing the elements
	 *  defining a Bluetooth device
	 */
	typedef struct {
		/*<! The address of remote device */
		char *remote_address;
		/*<! The name of remote device */
		char *remote_name;
		/*<! The Bluetooth classes */
		artik_bt_class cod;
		/*<! The strength indicator of received signal  */
		short rssi;
		/*<! The UUID list */
		artik_bt_uuid *uuid_list;
		/*<! The number of uuid */
		int uuid_length;
		/*<! The bonding state */
		bool is_bonded;
		/*<! The connection state */
		bool is_connected;
		/*<! The authorization state */
		bool is_authorized;
		/*<! manufacturer specific data length */
		int manufacturer_data_len;
		/*<! manufacturer specific data */
		char *manufacturer_data;
		/*<! manufacturer ID */
		short manufacturer_id;
		/*<! manufacturer name */
		char manufacturer_name[MAX_BT_NAME_LEN];
		/*<! service data length */
		int svc_data_len;
		/*<! service data */
		char *svc_data;
		/*<! 128 bit service uuid */
		char svc_uuid[MAX_BT_UUID_LEN];
	} artik_bt_device;

	/*!
	 *  \brief Bluetooth adapter definition
	 *
	 *  Structure containing the elements
	 *  defining a Bluetooth adapter
	 */
	typedef struct {
		char *address; /*!< The bluetooth device address. */
		char *name; /*!< The bluetooth system name. */
		char *alias; /*!< The bluetooth friendly name. */
		artik_bt_class cod; /*!< The bluetooth class of device. */
		bool discoverable; /*!< True if the device is disocverable; false otherwise. */
		bool pairable; /*!< True if the device is pairable; false otherwise */
		unsigned int pair_timeout; /*!< The pairable timeout in seconds */
		unsigned int discover_timeout; /*!< The discoverable timeout in seconds */
		bool discovering; /*!< Indicates that a device discovery procedure is active */
		artik_bt_uuid *uuid_list; /*!< List of UUIDs of the available local services */
		int uuid_length; /*!< The size of \ref uuid_list */
	} artik_bt_adapter;

	/*!
	 *  \brief Bluetooth advertisement definition
	 *
	 *  Structure containing the elements
	 *  defining Bluetooth advertisement
	 */
	typedef struct {
		/*<! Type of advertising ("broadcast" or "peripheral") */
		const char *type;
		/*<! List of UUIDs (128-bit version) to include in the "Service UUID" field of the advertising data */
		const char **svc_uuid;
		/*<! Length of svc_uuid */
		int svc_uuid_len;
		/*<! Manufacturer ID */
		unsigned short mfr_id;
		/*<! Manufacturer Data to include in the advertising data */
		unsigned char *mfr_data;
		/*<! The length of mfr_data*/
		int mfr_data_len;
		/*<! Array of UUID to include in "Service Solicitation" advertisement data. */
		const char **solicit_uuid;
		/*<! The length of solicit_uuid */
		int solicit_uuid_len;
		/*<! The UUID of the service associate with the svc_data*/
		const char *svc_id;
		/*<! Data to include in the advertisement packet*/
		unsigned char *svc_data;
		/*<! The length of svc_data */
		int svc_data_len;
		/*<! If true, the Tx Power is included in the advertisement packet*/
		bool tx_power;
	} artik_bt_advertisement;

	/*!
	 *  \brief This enum type is used to specify the type of scan
	 */
	typedef enum {
		BT_SCAN_AUTO, /*!< Indicate interleaved scan */
		BT_SCAN_BREDR, /*!< Indicate BR/EDR (basic rate/enhanced data rate) scan */
		BT_SCAN_LE /*!< Indicate LE scan only */
	} artik_bt_scan_type;

	/*!
	 *  \brief Bluetooth scan filter definition
	 *
	 *  Structure containing the elements
	 *  needed to define scan filters
	 *
	 *  When a remote device is found that advertises any UUID from \ref uuid_list,
	 *  it will be reported if received RSSI is higher than \ref rssi.
	 */
	typedef struct {
		artik_bt_uuid *uuid_list; /*!< Filtered service UUIDs */
		unsigned int uuid_length; /*!< Size of \ref uuid_list */
		int16_t rssi; /*!< RSSI threshold value */
		artik_bt_scan_type type; /*!< Type of scan */
	} artik_bt_scan_filter;

	/*!
	 *  \brief Bluetooth AD2P source definition
	 *
	 *  Structure containing the elements
	 *  defining Bluetooth A2P source properties
	 */
	typedef struct {
		char *device;
		char *uuid;
		unsigned char codec;
		unsigned char *configuration;
		char *state;
	} artik_bt_a2dp_source_property;

	/*!
	 * \brief This enum type is used to specify the repeat mode
	 */
	typedef enum {
		BT_AVRCP_REPEAT_SINGLETRACK = 0x00, /*!< Repeat a single track */
		BT_AVRCP_REPEAT_ALLTRACKS, /*!< Repeat all tracks */
		BT_AVRCP_REPEAT_GROUP, /*!< Repeat a group of tracks */
		BT_AVRCP_REPEAT_OFF /*!< Turn off the repeat mode */
	} artik_bt_avrcp_repeat_mode;

	/*!
	 * \brief Description of AVRCP item
	 */
	typedef struct {
		/*!
		 * \brief Player object path
		 *
		 * The item belongs to this object
		 */
		char *player;
		/*!
		 * \brief Displayable name
		 */
		char *name;
		/*!
		 * \brief Type of the item
		 *
		 * Type can takes the following values "video", "audio" or "folder"
		 */
		char *type;
		/*!
		 * \brief Folder type
		 *
		 * The possible values for this field are "mixed", "titles", "albums" or "artists"
		 *
		 * This field is available if the property \ref type is "folder",
		 * otherwise the field is set to NULL
		 */
		char *folder;
		/*!
		 *\brief Indicate if the media can be played
		 *
		 * True if the item can be played, false otherwise
		 */
		bool  playable;

		/*metadata*/
		/*!
		 * \brief Title of the media
		 *
		 * This field is available if the property \ref type is set to "audio" or "video"
		 */
		char *title;
		/*!
		 * \brief Name of the artist
		 *
		 * This field is available if the property \ref type is set to "audio" or "video"
		 */
		char *artist;
		/*!
		 * \brief Name of the album
		 *
		 * This field is available if the property \ref type is set to "audio" or "video"
		 */
		char *album;
		/*!
		 * \brief Genre
		 *
		 * This field is available if the property \ref type is set to "audio" or "video"
		 */
		char *genre;
		/*!
		 * \brief Number of tracks in the album
		 *
		 * This field is available if the property \ref type is set to "audio" or "video"
		 */
		unsigned int number_of_tracks;
		/*!
		 * \brief Number of the media
		 *
		 * This field is available if the property \ref type is set to "audio" or "video"
		 */
		unsigned int number;
		/*
		 * \brief Playing time in milliseconds
		 *
		 * This field is available if the property \ref type is set to "audio" or "video"
		 */
		unsigned int duration;
	} artik_bt_avrcp_item_property;

	/*!
	 * \brief List of AVRCP items
	 */
	typedef struct artik_bt_avrcp_item {
		/*!
		 * \brief Pointer to string internally used by the API.
		 */
		char *item_obj_path;

		/*!
		 * \brief Properties for this item.
		 */
		artik_bt_avrcp_item_property *property;

		/*!
		 * \brief Next item in the list
		 */
		struct artik_bt_avrcp_item *next_item;
	} artik_bt_avrcp_item;

	/*!
	 * \brief Callback prototype for release event
	 *
	 * This callback is called when \ref spp_unregister_profile
	 * is called.
	 *
	 * \param [in] user_data The user data passed from the \ref artik_bluetooth_spp_set_callback function
	 */
	typedef void (*release_callback)(void *user_data);
	/*!
	 * \brief Callback prototype for new connection event
	 *
	 * This callback is called when a new connection has been made and
	 * authorized
	 *
	 * \param [in] addr The bluetooth address of the remote device
	 * \param [in] fd file descriptor
	 * \param [in] version profile version
	 * \param [in] features profile features
	 * \param [in] user_data The user data passed from the \ref artik_bluetooth_spp_set_callback function
	 */
	typedef void (*new_connection_callback)(char *addr, int fd,
			int version, int features, void *user_data);
	/*!
	 * \brief Callback prototype for disconnection event
	 *
	 * This callback is called when a disconnection occurs.
	 *
	 * \param [in] addr The bluetooth address of the remote device
	 * \param [in] user_data The user data passed from the \ref artik_bluetooth_spp_set_callback function
	 */
	typedef void (*request_disconnect_callback)(char *addr,
			void *user_data);

	/*!
	 * \brief Description of SPP profile option
	 */
	typedef struct {
		char *name; /*!< Human readable name */
		char *service;/*!< The primary service class UUID */
		/*!
		 * \brief This field is used by asymetric profile that
		 * do not have UUIDs available to uniquely identify
		 * each side
		 *
		 * The possible value for this field is "client" or
		 * "server".
		 */
		char *role;
		long channel;/*!< RFCOMM channel number */
		long PSM;/*!< Protocol and Service Multiplexer id */
		bool require_authentication;/*!< If true pairing is required before connections will be established */
		bool require_authorization;/*!< Request authorization before any connection */
		bool auto_connect;/*!< Enable auto-connect*/
		long version;/*!< Profile version */
		long features;/*!< Profile features */
	} artik_bt_spp_profile_option;

	typedef unsigned char * (*select_config_callback)(
		unsigned char *capabilities, int *len);
	typedef void (*set_config_callback)(
			artik_bt_a2dp_source_property * properties);
	typedef void (*clear_config_callback)(void);

	/*!
	 *  \brief Bluetooth FTP definition
	 *
	 *  Structure containing the elements
	 *  defining Bluetooth FTP properties
	 */
	typedef struct {
		char *object_path;/*!< The dbus object path of the Transfer object. */
		char *name; /*!< Name of the transfered object */
		char *file_name; /*!< Complete name of the file being received or sent */
		/*!
		 * \brief Give the current status of the transfer
		 *
		 * This field can takes the following values "queued", "active", "suspended",
		 * "complete" or "error"
		 */
		char *status;
		unsigned long long transfered; /*!< Number of bytes transferred */
		unsigned long long size; /*!< Size of the transferred file. */
	} artik_bt_ftp_property;

	/*!
	 * \brief List of FTP items
	 *
	 *  Structure containing the elements
	 *  defining a Bluetooth FTP file
	 */
	typedef struct artik_bt_ftp_file {
		char *file_type; /*! Either "folder" or "file"*/
		char *file_name; /*! Object name in UTF-8 format */
		char *modified; /*! Last change */
		char *file_permission; /*! Group, owner and other permission */
		unsigned long long size; /*! Object size or number of items in folder */
		struct artik_bt_ftp_file *next_file; /*! Next item in the list */
	} artik_bt_ftp_file;

	typedef void (*prop_change_callback)(artik_bt_ftp_property property,
			char *prop_str, long long prop_int);

	/*!
	 *  \brief Bluetooth HRP data definition
	 *
	 *  Structure containing the elements
	 *  defining Bluetooth HRP data
	 */
	typedef struct {
		unsigned short bpm;
		unsigned short energy;
		bool contact;
		unsigned short *interval;
		unsigned short interval_count;
	} artik_bt_hrp_data;

	/*!
	 * \brief Bluetooth service request handle type
	 *
	 * Handle type used to carry specific information for a bluetooth service request.
	 * The agent register must give a response to the bluetooth service with the methods \ref agent_send_pincode,
	 * \ref agent_send_passkey, \ref agent_send_error or \ref agent_send_empty_response.
	 *
	 */
	typedef void *artik_bt_agent_request_handle;

	/*!
	 * \brief The errors returned to the bluetooth service when a request failed.
	 *
	 * The possible errors returned by the agent handler to the bluetooth service.
	 */
	typedef enum {
		BT_AGENT_REQUEST_REJECTED, /*!< The request has been rejected by the agent. */
		BT_AGENT_REQUEST_CANCELED /*!< The request has been canceled by the agent. */
	} artik_bt_agent_request_error;

	/*!
	 * \brief Method called when the bluetooth service unregisters the agent.
	 *
	 * There is no need to call the method \ref agent_unregister in this callback,
	 * because when this method gets called the agent has already been unregistered
	 *
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*agent_release_callback)(void *user_data);

	/*!
	 * \brief Method called when the bluetooth service needs to get the pincode for an authentification.
	 *
	 * The pincode or an error should be returned to the bluetooth service with \ref agent_send_pincode
	 * or \ref agent_send_error functions
	 *
	 * \param[in] handle Handle tied to a bluetooth service request
	 * \param[in] device The bluetooth address of the remote device
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*request_pincode_callback)(artik_bt_agent_request_handle handle, char *device, void *user_data);

	/*
	 * \brief Method called when the bluetooth service needs to display a pincode for an authentication
	 *
	 * \param[in] device The bluetooth address of the remote device
	 * \param[in] pincode An alphanumeric string of 1-16 characters
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*display_pincode_callback)(char *device,
			char *pincode,
			void *user_data);

	/*!
	 * \brief Method called when the bluetooth service needs to get the passkey for an authentication.
	 * The passkey or an error should be returned to the bluetooth service with \ref agent_send_ or
	 * \ref agent_send_error functions.
	 *
	 * \param[in] handle Handle tied to a bluetooth service request
	 * \param[in] device The bluetooth address of the remote device
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*request_passkey_callback)(artik_bt_agent_request_handle handle, char *device, void *user_data);

	/*!
	 * \brief Method called when the bluetooth service needs to display a passkey for an authentifcation
	 *
	 * \param[in] device The bluetooth address of the remote device
	 * \param[in] passkey A numeric value between 0-999999
	 * \param[in] entered A numeric value indicates the number of already typed keys on the remote side
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*display_passkey_callback)(char *device,
			unsigned int passkey,
			unsigned int entered,
			void *user_data);

	/*!
	 * \brief Method called when the bluetooth service needs to confirm a passkey for an authentication
	 *
	 * To confirm the request, you should use \ref agent_send_empty_response function.
	 * If you want to reject or cancel the request, you should use \ref agent_send_error
	 *
	 * \param[in] handle Handle tied to a bluetooth service request
	 * \param[in] device The bluetooth address of the remote device
	 * \param[in] passkey A numeric value between 0-999999
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*request_confirmation_callback)(artik_bt_agent_request_handle handle,
			char *device,
			unsigned int passkey,
			void *user_data);

	/*!
	 * \brief Method called when the bluetooth service request the user to authorize an incoming pairing attempt.
	 *
	 * To confirm the request, you should use \ref agent_send_empty_response.
	 * If you want to reject or cancel the request, you should use \ref agent_send_error
	 *
	 * \param[in] handle Handle tied to a bluetooth service request
	 * \param[in] device The bluetooth address of the remote device
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*request_authorization_callback)(artik_bt_agent_request_handle handle,
			char *device,
			void *user_data);

	/*!
	 * \brief Method called when the bluetooth service needs to authorize a connection to a service specified
	 * by its \ref uuid
	 *
	 * To confirm the request, you should use \ref agent_send_empty_response function.
	 * If you want to reject or cancel the request, you should use \ref agent_send_error
	 *
	 * \param[in] handle Handle tied to a bluetooth service request
	 * \param[in] device The bluetooth address of the remote device
	 * \param[in] uuid The uuid of the service
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*authorize_service_callback)(artik_bt_agent_request_handle handle,
			char *device,
			char *uuid,
			void *user_data);

	/*!
	 * \brief Method called to indicate that the bluetooth service request failed before the agent returned a reply.
	 *
	 * \param[in] user_data The user data passed from the \ref agent_set_callback function
	 */
	typedef void (*cancel_callback)(void *user_data);

	/*!
	 * \brief This struct represents a Bluetooth agent.
	 */
	typedef struct {
		agent_release_callback release_func;
		request_pincode_callback request_pincode_func;
		display_pincode_callback display_pincode_func;
		request_passkey_callback request_passkey_func;
		display_passkey_callback display_passkey_func;
		request_confirmation_callback request_confirmation_func;
		request_authorization_callback request_authorization_func;
		authorize_service_callback authorize_service_func;
		cancel_callback cancel_func;
		void *user_data; /*<! The user data to be passed to the callbakc functions */
	} artik_bt_agent_callbacks;

	/*! \struct artik_bluetooth_module
	 *
	 *  \brief Bluetooth module operations
	 *
	 *  Structure containing all the operations exposed
	 *  by the Bluetooth module.
	 */
	typedef struct {
		/*!
		 * \brief Starts the device discovery
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*start_scan) ();
		/*!
		 * \brief Stops the device discovery
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*stop_scan) ();
		/*!
		 * \brief Get devices list.
		 *
		 * \param[out] devices t Device list
		 * \param[out] num_devices Number of devices
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*get_devices) (artik_bt_device**devices, int *num_devices);
		/*!
		 * \brief Get paired devices list.
		 *
		 * \param[out] devices Bluetooth device list
		 * \param[out] num_devices Number of devices
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*get_paired_devices) (artik_bt_device**devices, int *num_devices);
		/*!
		 * \brief Get connected devices list.
		 *
		 * \param[out] devices Bluetooth device list
		 * \param[out] num_devices Number of devices
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*get_connected_devices) (artik_bt_device**devices, int *num_devices);
		/*!
		 * \brief Create a bond with a remote Bluetooth device, asynchronously.
		 *
		 * \param[in] addr The address of the remote Bluetooth device with
		 *                 which the bond should be created.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*start_bond) (const char *addr);
		/*!
		 * \brief Cancel the bonding process.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*stop_bond) (const char *addr);
		/*!
		 * \brief Connect to the device
		 *
		 * \param[in] addr The address of the remote Bluetooth device to
		 *                 unbond with.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*connect) (const char *addr);
		/**
		 * \brief Disconnect to the device
		 *
		 * \param[in] addr The address of the remote Bluetooth device to
		 *                 disconnect from.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*disconnect) (const char *addr);
		/*!
		 * \brief Free devices list
		 *
		 * \param[in] device_list \ref artik_bt_device array to free
		 * \param[in] count Number of \ref artik_bt_device objects contained in the array
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*free_devices) (artik_bt_device*device_list, int count);
		/*!
		 * \brief Register a callback function to be invoked when the device
		 *        discovery state changes.
		 *
		 * \param[in] event The event to register
		 * \param[in] callback The callback function to register
		 * \param[in] user_data The user data to be passed to the callback function
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*set_callback) (artik_bt_event event,
				artik_bt_callback user_callback, void *user_data);
		/*!
		 * \brief Unregister the callback function.
		 *
		 * \param[in] event The event to unregister
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*unset_callback) (artik_bt_event event);
		/*!
		 * \brief Remove unpaired device in list, synchronously.
		 *
		 */
		artik_error(*remove_unpaired_devices) ();
		/*!
		 * \brief Destroy the bond, asynchronously.
		 *
		 * \param[in] addr The address of the remote Bluetooth device to
		 *                 remove.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*remove_device) (const char *addr);
		/*!
		 * \brief Set the device discovery filter.
		 *
		 * \param[in] filter The filter to apply
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_scan_filter) (artik_bt_scan_filter * filter);
		/*!
		 * \brief Set the bluetooth friendly name
		 *
		 * In case no alias is set, the bluetooth friendly name is the system provided name.
		 * Setting an empty string as \ref alias and the system provided name is used as
		 * bluetooth friendly name.
		 *
		 * \param[in] alias The bluetooth friendly name you want to set.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_alias) (const char *alias);
		/*!
		 * \brief Switch the bluetooth adapter on or off.
		 *
		 * The value is not persistent. After restart the adapter it will reset back
		 * to the default value.
		 *
		 * \param[in] powered If powered is true the bluetooth adapter is switched on,
		 *                    otherwise it is switched off.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		/*!
		 * \brief Switch the bluetooth adapter to discoverable or non-discoverable to either
		 * make it visible or hide it.
		 *
		 * \param[in] discoverable If discoverable is true, sets the bluetooth adapter to discoverable;
		 * otherwise the non-discoverable mode is used
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_discoverable) (bool discoverable);
		/*!
		 * \brief Switch an adapter to pairable or non-pairable.
		 *
		 * This property only affects incoming pairing.
		 *
		 * \param[in] pairable If pairable is true, sets the bluetooth adapter to pairable;
		 * otherwise the non-pairable mode is used.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_pairable) (bool pairable);
		/*!
		 * \brief Set the pairable timeout.
		 *
		 * When the pairable timeout expired the bluetooth adapter switchs to non-pairable
		 *
		 * \param[in] timeout Pairable timeout in seconds. A value of zero disabled the timer.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_pairableTimeout) (unsigned int timeout);
		/*!
		 * \brief Set the discoverable timeout.
		 *
		 * When tie discoverable timeout expired the bluetooth adapter switchs to non-discoverable.
		 *
		 * \param[in] timeout Discoverable timeout in seconds. A value of zero disabled the timer.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_discoverableTimeout) (unsigned int timeout);
		/*!
		 * \brief Indicate that a device discovery procedure is active.
		 *
		 * \return True when a device discovery procedure is active, false otherwise.
		 */
		bool (*is_scanning)(void);
		/*!
		 * \brief Get a property of a remote device.
		 *
		 * The possible values for \ref property are "Address", "Name", "Icon", "Alias", "Adapter", "Modalias".
		 *
		 * property | Description
		 * ------------------------------------------------------------
		 * Address  | The bluetooth device address.
		 * Name     | The bluetooth device name.
		 * Alias    | The name alias for the remote device.
		 *
		 * \param[in] addr The bluetooth address of the remote device.
		 * \param[in] property The property you want to get
		 * \param[out] value The value of the property.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*get_device_property) (const char *addr,
				const char *property, char **value);
		/*!
		 * \brief Get the adapter information
		 *
		 * \param[out] adapter The bluetooth adapter information
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*get_adapter_info) (artik_bt_adapter * adapter);
		/*!
		 * \brief Remove all discovred devices.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*remove_devices) (void);
		/*!
		 * \brief Connect a specific profile of a remote device.
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] uuid The UUID of the remote profile
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*connect_profile) (const char *addr, const char *uuid);
		/*!
		 * \brief Seen the remote device as trusted
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_trust) (const char *addr);
		/*!
		 * \brief Seen the remote device as untrusted
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*unset_trust) (const char *addr);
		/*!
		 * \brief Reject any incomming connections from the remote device.
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*set_block) (const char *addr);
		/*!
		 * \brief Accept all incoming connections from the remote device
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*unset_block) (const char *addr);
		/*!
		 * \brief Free the memory used by the remote device
		 *
		 * \param[in] device The remote device
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*free_device) (artik_bt_device * device);
		/*!
		 * \brief Indicate if the remote device is paired.
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return True if the remote device is paired; false otherwise.
		 */
		bool (*is_paired)(const char *addr);
		/*!
		 * \brief Indicate if the remote device is connected.
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return True if the remote device is connected; false otherwise.
		 */
		bool (*is_connected)(const char *addr);
		/*!
		 * \brief Indicate if the remote device is trusted.
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return True if the remote device is trusted; false otherwise.
		 */
		bool (*is_trusted)(const char *addr);
		/*!
		 * \brief Indicate if the remote device is blocked.
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 *
		 * \return True if the remote device is trusted; false otherwise.
		 */
		bool (*is_blocked)(const char *addr);
		/*! \brief Add a new GATT service.
		 *
		 * \param[in] svc The service properties.
		 * \param[out] id The internal id of the service.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_add_service) (artik_bt_gatt_service svc, int *id);
		/*! \brief Add a new GATT characteristic
		 *
		 * \param[in] svc_id The internal id of the GATT service to which
		 *                   the characteristic belongs to
		 * \param[in] chr The characteristic properties
		 * \param[out] id The internal id of the characteristic.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_add_characteristic) (int svc_id,
				artik_bt_gatt_chr chr, int *id);
		/*! \brief Set callback for handle read request of a GATT characteristic
		 *
		 * \param[in] svc_id The internal id of the GATT service
		 * \param[in] char_id The internal id of the GATT characteristic
		 * \param[in] callback The handler of read request
		 * \param[in] user_data The user data to be passed to the callback functions.
		 */
		artik_error(*gatt_set_char_on_read_request) (int svc_id, int char_id,
				artik_bt_gatt_req_read callback, void *user_data);
		/*! \brief Set callback for handle write request of a GATT characteristic
		 *
		 * \param[in] svc_id The internal id of the GATT service
		 * \param[in] char_id The internal id of the GATT characteristic
		 * \param[in] callback The handler of write request
		 * \param[in] user_data The user data to be passed to the callback functions.
		 */
		artik_error(*gatt_set_char_on_write_request) (int svc_id, int char_id,
				artik_bt_gatt_req_write callback, void *user_data);
		/*! \brief Set callback for handle notify request of a GATT characteristic
		 *
		 * \param[in] svc_id The internal id of the GATT service
		 * \param[in] char_id The internal id of the GATT characteristic
		 * \param[in] callback The handler of notify request
		 * \param[in] user_data The user data to be passed to the callback functions.
		 */
		artik_error(*gatt_set_char_on_notify_request) (int svc_id, int char_id,
				artik_bt_gatt_req_notify callback, void *user_data);
		/* \brief Add a new GATT descriptor
		 *
		 * \param[in] service_id The internal id of the GATT service to which
		 *                       the descriptor belongs to
		 * \param[in] char_id The internal id of the GATT characteristic to wich the
		 *                    the descriptor belongs to
		 * \param[in] desc The descriptor properties.
		 * \param[out] id The internal id of the descriptor.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_add_descriptor) (int service_id, int char_id,
				artik_bt_gatt_desc desc, int *id);
		/*! \brief Set callback for handle read request of a GATT descriptor
		 *
		 * \param[in] svc_id The internal id of the GATT service
		 * \param[in] char_id The internal id of the GATT characteristic
		 * \param[in] desc_id The internal id of the GATT descriptor
		 * \param[in] callback The handler of read request
		 * \param[in] user_data The user data to be passed to the callback functions.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_set_desc_on_read_request) (int svc_id, int char_id, int desc_id,
				artik_bt_gatt_req_read callback, void *user_data);
		/*! \brief Set callback for handle write request of a GATT descriptor
		 *
		 * \param[in] svc_id The internal id of the GATT service
		 * \param[in] char_id The internal id of the GATT characteristic
		 * \param[in] desc_id The internal id of the GATT descriptor
		 * \param[in] callback The handler of write request
		 * \param[in] user_data The user data to be passed to the callback functions.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_set_desc_on_write_request) (int svc_id, int char_id, int desc_id,
				artik_bt_gatt_req_write callback, void *user_data);
		/*! \brief Register a local GATT services hierarchy
		 *
		 * \param[in] id The internal id of the GATT service
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_register_service) (int id);
		/*! \brief Unregister the service that has been previously registred with \ref gatt_register_service
		 *
		 * \param[in] id The internal id of the GATT service
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_unregister_service) (int id);
		/*! \brief List the service of a remote device
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[out] uuid_list The list of service UUID
		 * \param[out] len The size of uuid_list
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_get_service_list) (const char *addr,
				artik_bt_uuid **uuid_list, int *len);
		/*! \brief List the characteristic of a remote GATT service
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[out] uuid_list The list of characteristic UUID
		 * \param[out] len The size of uuid_list
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_get_characteristic_list) (const char *addr,
				const char *srv_uuid, artik_bt_uuid **uuid_list, int *len);
		/*! \brief List the descriptors of a remote GATT characteristic
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 * \param[out] uuid_list The list of descriptor UUID
		 * \param[out] len The size of uuid_list
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_get_descriptor_list) (const char *addr, const char *srv_uuid,
				const char *char_uuid, artik_bt_uuid **uuid_list, int *len);
		/*! \brief Read the value of a remote GATT characteristic
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 * \param[out] byte The characteristic's value
		 * \param[out] byte_len Size of byte
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_char_read_value) (const char *addr, const char *srv_uuid,
				const char *char_uuid, unsigned char **byte, int *byte_len);
		/*! \brief Write the value contains in \ref byte in a remote GATT characteristic
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 * \param[in] byte The characteristic's value
		 * \param[in] byte_len Size of byte
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_char_write_value) (const char *addr, const char *srv_uuid,
				const char *char_uuid, const unsigned char byte[], int byte_len);
		/*! \brief Read the value of a remote GATT descriptor
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 * \param[in] desc_uuid UUID of the remote GATT descriptor
		 * \param[out] byte The descriptor's value
		 * \param[out] byte_len Size of byte
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_desc_read_value) (const char *addr, const char *srv_uuid,
				const char *char_uuid, const char *desc_uuid,
				unsigned char **byte, int *byte_len);
		/*! \brief Write the value contains in \ref byte in a remote GATT characteristic
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 * \param[in] desc_uuid UUID of the remote GATT descriptor
		 * \param[out] byte The descriptor's value
		 * \param[out] byte_len Size of byte
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_desc_write_value) (const char *addr, const char *srv_uuid,
				const char *char_uuid, const char *desc_uuid,
				const unsigned char byte[], int byte_len);
		/*! \brief Start a notification session from a remote GATT characteristic
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_start_notify) (const char *addr, const char *srv_uuid,
				const char *char_uuid);
		/*! \brief Stop a notification session from a remote GATT characteristic
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_stop_notify) (const char *addr, const char *srv_uuid,
				const char *char_uuid);
		/*! \brief Get the properties of a remote GATT characteristic
		 *
		 * \param[in] addr The bluetooth address of the remote device
		 * \param[in] srv_uuid UUID of the remote GATT service
		 * \param[in] char_uuid UUID of the remote GATT characteristic
		 * \param[out] properties The characteristic's properties
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_get_char_properties) (const char *addr, const char *srv_uuid,
				const char *char_uuid, artik_bt_gatt_char_properties * properties);
		/*! \brief Register an advertisement object to sent over the LE advertising chanel.
		 *
		 * \param[in] user_adv The advertisement object
		 * \param[out] id The internal id of the advertisement
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*register_advertisement)
				(artik_bt_advertisement * user_adv, int *id);
		/*! \brief Unregister an advertisement object
		 *
		 * \param[in] id The internal id of the advertisement object.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*unregister_advertisement) (int id);
		/*! \brief Send a response to a read request
		 *
		 * \param[in] req Handle tied to the read request
		 * \param[in] len The size of value
		 * \param[in] value The response
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_req_set_value) (artik_bt_gatt_req req,
				int len, const unsigned char *value);
		/*! \brief Send a status to a request
		 *
		 * \param[in] req Handle tied to the request.
		 * \param[in] state The status of the request.
		 * \param[in] err_msg An err_msg (can be NULL)
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_req_set_result) (artik_bt_gatt_req req,
				artik_bt_gatt_req_state_type state, const char *err_msg);
		/*! \brief Send a characteristic value notification or indication.
		 *
		 * \param[in] svc_id The internal service id
		 * \param[in] char_id The internal characteristic id
		 * \param[in] byte The new value of the characteristic
		 * \param[in] len Size of \ref bytes
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*gatt_notify) (int svc_id, int char_id, unsigned char *byte,
				int len);
		/*!
		 * \brief Change to the specified folder
		 *
		 * \param[in] folder The path of folder it will change to
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_change_folder) (const char *folder);
		/*!
		 * \brief List the items of the folder
		 *
		 * \param[in] start_item The start index of the item
		 * \param[in] end_item The end index of the item
		 *
		 * \param[out] item_list The item list
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_list_item)(int start_item, int end_item,
				artik_bt_avrcp_item * *item_list);
		/*!
		 * \brief Get the repeat mode of the player
		 *
		 * \param[out] repeat_mode The repeat mode of the player
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_repeat_mode)(
				artik_bt_avrcp_repeat_mode * repeat_mode);
		/*!
		 * \brief Change the repeat mode of the player
		 *
		 * \param[in] repeat_mode The repeat mode of the player
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_set_repeat_mode)(artik_bt_avrcp_repeat_mode repeat_mode);
		/*!
		 * \brief Whether an AVRCP target is connected with current device.
		 *
		 * \param[out] is_connected The state of whether an AVRCP target is
		 *                         connected.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_is_connected) (bool *is_connected);
		/*!
		 * \brief Control remote AVRCP target to resume play.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_resume_play) (void);
		/*!
		 * \brief Control remote AVRCP target to pause play.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_pause) (void);
		/*!
		 * \brief Control remote AVRCP target to stop play.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_stop) (void);
		/*!
		 * \brief Control remote AVRCP target to play the next item.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_next) (void);
		/*!
		 * \brief Control remote AVRCP target to play the previous item.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_previous) (void);
		/*!
		 * \brief Control remote AVRCP target to fast forward current item.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_fast_forward) (void);
		/*!
		 * \brief Control remote AVRCP target to rewind current item.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_rewind) (void);
		/*!
		 * \brief Get the item property
		 *
		 * \param[in] item To be get property
		 * \param[in] properties Item's property be sorted
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_property)(char *item, artik_bt_avrcp_item_property **properties);
		/*!
		 * \brief Play the item
		 *
		 * \param[in] item To be played item
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_play_item)(char *item);
		/*!
		 * \brief Add the item to the playing list
		 *
		 * \param[in] item To be add to the playing list
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_add_to_playing)(char *item);
		/*!
		 * \brief Get a player name from an AVRCP target .
		 *
		 * \param[out] name The player name got from an AVRCP target.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_name) (char **name);
		/*!
		 * \brief Get a player status from an AVRCP target .
		 *
		 * \param[out] status The player status got from an AVRCP target.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_status) (char **status);
		/*!
		 * \brief Get a player subtype from an AVRCP target .
		 *
		 * \param[out] subtype The player subtype got from an AVRCP target.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_subtype) (char **subtype);
		/*!
		 * \brief Get a player type from an AVRCP target .
		 *
		 * \param[out] type The player type got from an AVRCP target.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_type) (char **type);
		/*!
		 * \brief Whether the player is browsable.
		 *
		 * \param[out] browsable The player browsable got from an AVRCP target.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_browsable) (bool *is_browsable);
		/*!
		 * \brief Get a playback position from an AVRCP target .
		 *
		 * \param[out] position The player position got from an AVRCP target.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*avrcp_controller_get_position) (unsigned int *position);
		/*!
		 * \brief register the pan services.
		 *
		 * \param[in] uuid To be register service uuid.
		 * \param[in] bridge To be register bridge.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*pan_register) (const char *uuid, const char *bridge);
		/*!
		 * \brief unregister the pan services.
		 *
		 * \param[in] uuid To be unregistered.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*pan_unregister) (const char *uuid);
		/*!
		 * \brief connect the pan services to another.
		 *
		 * \param[in] mac_addr To be connect target device mac address.
		 * \param[in] uuid target device uuid.
		 *
		 * \param[out] network_interface The interface of the connection.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*pan_connect) (const char *mac_addr,
				const char *uuid, char **network_interface);
		/*!
		 * \brief disconnect the pan services.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*pan_disconnect) (void);
		/*!
		 * \brief get the status of the connection.
		 *
		 * \param[out] connected The connection status.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*pan_get_connected) (bool *connected);
		/*!
		 * \brief get the connection interface of the connection.
		 *
		 * \param[out] _interface The connection interface.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*pan_get_interface) (char **_interface);
		/*!
		 * \brief get the device uuid.
		 *
		 * \param[out] uuid The device uuid.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*pan_get_UUID) (char **uuid);
		/*!
		 * \brief spp register profile.
		 *
		 * \param[in] artik_bt_spp_profile_option The option of the profile.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*spp_register_profile)(artik_bt_spp_profile_option * opt);
		/*!
		 * \brief spp unregister profile.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*spp_unregister_profile)();
		/*!
		 * \brief set the callback of the spp.
		 *
		 * \param[in] release_callback The handler of release.
		 * \param[in] new_connection_callback The handler of new connection.
		 * \param[in] request_disconnect_callback The handler of request disconnect.
		 * \param[in] user_data The user data to be passed to the callback functions.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*spp_set_callback)(release_callback release_func, new_connection_callback connect_func,
				request_disconnect_callback disconnect_func, void *user_data);
		/*!
		 * \brief create ftp session with FTP server.
		 *
		 * \param[in] dest_addr The BT MAC address of FTP server.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_create_session)(char *dest_addr);
		/*!
		 * \brief remove ftp session.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_remove_session)(void);
		/*!
		 * \brief ftp browse change folder.
		 *
		 * \param[in] folder The folder will change to.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_change_folder)(char *folder);
		/*!
		 * \brief ftp create folder.
		 *
		 * \param[in] folder The folder to be created.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_create_folder)(char *folder);
		/*!
		 * \brief ftp delete file.
		 *
		 * \param[in] file The file to be deleted.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_delete_file)(char *file);
		/*!
		 * \brief ftp list the files.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_list_folder)(artik_bt_ftp_file * *file_list);
		/*!
		 * \brief ftp download file.
		 *
		 * \param[in] target_file The saved file name.
		 * \param[in] source_file The file to be downloaded.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_get_file)(char *target_file, char *source_file);
		/*!
		 * \brief ftp upload file.
		 *
		 * \param[in] source_file The file to be uploaded.
		 * \param[in] target_file The uploaded file name.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_put_file)(char *source_file, char *target_file);
		/*!
		 * \brief ftp resume transfer file.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_resume_transfer)(void);
		/*!
		 * \brief ftp suspend transfer file.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*ftp_suspend_transfer)(void);
		/*!
		 * \brief Register agent with capability.
		 *
		 * \param[in] e	The capability which reflects the input and
		 *				output capabilities of the agent
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*agent_register_capability)(artik_bt_agent_capability e);
		/*!
		 * \brief Set the registered agent as default agent.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*agent_set_default)(void);
		/*!
		 * \brief Unregisters the agent that has been registered.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*agent_unregister)(void);
		/*!
		 * \brief Set agent callback functions.
		 *		 If it is not invoked, default callback function will be set.
		 *
		 * \param[in] agent_callback The callback functions needed by blueZ.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 *
		 */
		artik_error(*agent_set_callback)(artik_bt_agent_callbacks * agent_callback);
		/*!
		 * \brief Send the pincode to the bluetooth service
		 *
		 * \param[in] handle Handle tied to a bluetooth service request
		 * \param[in] pincode String containing the pincode sends to the bluetooth service
		 *                    The pincode should be an alphanumeric string of 1-16 characters.
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*agent_send_pincode)(artik_bt_agent_request_handle handle, char *pincode);
		/*!
		 * \brief Send the passkey to the bluetooth service
		 *
		 * After a call to this function handle becomes an invalid value.
		 *
		 * \param[in] handle Handle tied to a bluetooth service request
		 * \param[in] passkey Numeric value containing the passkey sends to the bluetooth service.
		 *                    The passkey shoule be a numeric value between 0-999999
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*agent_send_passkey)(artik_bt_agent_request_handle handle, unsigned int passkey);
		/*!
		 * \brief Send an error to the bluetooth service
		 *
		 * After a call to this function handle becomes an invalid value.
		 *
		 * \param[in] handle Handle tied to a bluetooth service request
		 * \param[in] e The error sends to the bluetooth service
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*agent_send_error)(artik_bt_agent_request_handle handle, artik_bt_agent_request_error e,
				const char *err_msg);
		/*!
		 * \brief Send an empty response to the bluetooth service
		 *
		 * After a call to this function handle becomes an invalid value.
		 *
		 * \param[in] handle Handle tied to a bluetooth service request
		 *
		 * \return S_OK on success, otherwise a negative error value.
		 */
		artik_error(*agent_send_empty_response)(artik_bt_agent_request_handle handle);
	} artik_bluetooth_module;

	extern const artik_bluetooth_module bluetooth_module;

#ifdef __cplusplus
}
#endif
#endif  /*__ARTIK_BLUETOOTH_H */
