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

#ifndef ARTIK_MQTT_H_
#define ARTIK_MQTT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "artik_error.h"
#include "artik_types.h"
#include "artik_ssl.h"

/*! \file artik_mqtt.h
 *
 * \brief MQTT module definition
 *
 * Definitions and functions for accessing
 * the MQTT module and performing operations
 * using MQTT protocol
 *
 * \example mqtt_test/artik_mqtt_cloud_test.c
 */

/*!
 *  \brief MQTT PSK parameters definition
 *
 *  Structure containing the elements
 *  for defining PSK parameters
 */
typedef struct {
	const char *psk; /**< pre-defined key */
	const char *identity; /**< user's identity */
	const char *ciphers; /**< openssl cipher suite name */
} artik_mqtt_psk_param;

/*!
 *  \brief MQTT message definition
 *
 *  Structure containing the elements
 *  for defining a MQTT message
 */
typedef struct {
	int msg_id; /**< message identification */
	char *topic; /**< message topic */
	void *payload; /**< message payload content */
	int payload_len; /**< message pay load length */
	int qos; /**< message qos flag */
	bool retain; /**< message retain flag */
} artik_mqtt_msg;

/*!
 *  \brief MQTT handle type
 *
 *  Handle type used to carry a client instance for a mqtt object
 */
typedef void *artik_mqtt_handle;

/*!
 *  \brief MQTT configuration definition
 *
 *  Structure containing the elements
 *  for configuring a MQTT connection
 */
typedef struct {
	const char *client_id; /**< client identification */
	const char *user_name; /**< client user name */
	const char *pwd; /**< password corresponding to the user name */
	bool clean_session; /**< mqtt clean session flag */
	int keep_alive_time; /**< mqtt keep alive time in ms */
	artik_mqtt_msg *will_msg; /**< mqtt will message pointer */
	bool block; /**< mqtt block flag */
	/**< on_unsubscribe call back function */
	artik_ssl_config *tls;
	/**< TLS parameter, TLS should be mutually exclusive with PSK */
	artik_mqtt_psk_param *psk;
	/**< PSK parameter, PSK should be mutually exclusive with TLS */
	artik_mqtt_handle handle; /**< user defined data */
} artik_mqtt_config;

/*!
 *  \brief MQTT Connect callback type
 *
 *  Connect callback type used to add an optional process at the end
 *  of theprocess of the client method connect.
 */
typedef void (*connect_callback)(artik_mqtt_config *client_config,
				void *data_user, int artik_error);

/*!
 *  \brief MQTT Disconnect callback type
 *
 *  Disconnect callback type used to add an optional process at the end
 *  of theprocess of the client method disconnect.
 */
typedef void (*disconnect_callback)(artik_mqtt_config *client_config,
				void *data_user, int artik_error);
/*!
 *  \brief MQTT Subscribe callback type
 *
 *  Subscribe callback type used to add an optional process at the end
 *  of theprocess of the client method subscribe.
 */
typedef void (*subscribe_callback)(artik_mqtt_config *client_config,
				void *data_user, int mid, int qos_count,
				const int *granted_qos);
/*!
 *  \brief MQTT Unsubscribe callback type
 *
 *  Unsubscribe callback type used to add an optional process at the end
 *  of theprocess of the client method Unsubscribe.
 */
typedef void (*unsubscribe_callback)(artik_mqtt_config *client_config,
				void *data_user, int mid);
/*!
 *  \brief MQTT Publish callback type
 *
 *  Publish callback type used to add an optional process at the end of
 *  theprocess of the client method publish.
 */
typedef void (*publish_callback)(artik_mqtt_config *client_config,
				void *data_user, int mid);
/*!
 *  \brief MQTT Message callback type
 *
 *  Message callback type used to add an optional process at the end of
 *  theprocess of the client method message.
 */
typedef void (*message_callback)(artik_mqtt_config *client_config,
				void *data_user, artik_mqtt_msg *msg);

/*! \struct artik_mqtt_module
 *
 *  \brief MQTT module operations
 *
 *  Structure containing all the exposed operations exposed
 *  by the MQTT module
 */
typedef struct {
	/**
	 * Create a new mqtt client instance
	 * \param[in] client Pointer of an artik mqtt handle pointer
	 * \param[in] config The pointer of artik_mqtt_config structure
	 *            configuring mqtt instance
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*create_client)(artik_mqtt_handle *client,
				artik_mqtt_config *config);
	/**
	 * Free memory associated with a mqtt client instance.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*destroy_client)(artik_mqtt_handle handle);
	/**
	 * Set the WILLTOPIC and WILLMSG for connection.
	 * \param[in/out] config Pointer of artik_mqtt_config structure
	 *                configuring mqtt instance
	 * \param[in] willtopic WILLTOPIC to be set
	 * \param[in] willmsg WILLMSG to be set
	 * \param[in] qos qos value
	 * \param[in] retain retain flag
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*set_willmsg)(artik_mqtt_config *config,
			const char *willtopic, const char *willmsg,
			int qos, bool retain);
	/**
	 * Free the will message memory which is allocated by
	 * <set_willmsg>.
	 * \param[in/out] config Pointer of artik_mqtt_config structure
	 *                configuring mqtt instance
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*free_willmsg)(artik_mqtt_config *config);
	/**
	 * Remove a previously configured will.
	 * This must be called before calling <connect_server>.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*clear_willmsg)(artik_mqtt_handle client);

	/**
	 * Setter of the callback onConnect.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] cb the user callback to register into the client
	 *            instance.
	 * \param[in] data the container provided by the user.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*set_connect)(artik_mqtt_handle client,
			connect_callback cb, void *user_connect_data);

	/**
	 * Setter of the callback onDisconnect.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] cb the user callback to register into the client
	 *            instance.
	 * \param[in] data the container provided by the user.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*set_disconnect)(artik_mqtt_handle client,
		disconnect_callback cb, void *user_disconnect_data);

	/**
	 * Setter of the callback onSubscribe.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] cb the user callback to register into the client
	 *            instance.
	 * \param[in] data the container provided by the user.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*set_subscribe)(artik_mqtt_handle client,
		subscribe_callback cb, void *user_subscribe_data);

	/**
	 * Setter of the callback onUnsubscribe.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] cb the user callback to register into the client instance.
	 * \param[in] data the container provided by the user.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*set_unsubscribe)(artik_mqtt_handle client,
		unsubscribe_callback cb, void *user_unsubscribe_data);

	/**
	 * Setter of the callback onPublish.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] cb the user callback to register into the client instance.
	 * \param[in] data the container provided by the user.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*set_publish)(artik_mqtt_handle client,
			publish_callback cb, void *user_publish_data);

	/**
	 * Setter of the callback onMessage.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] cb the user callback to register into the client instance.
	 * \param[in] data the container provided by the user.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*set_message)(artik_mqtt_handle client,
			message_callback cb, void *data);

	/**
	 * Connect to an MQTT broker. This is a non-blocking call.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] host the hostname or ip address of the broker to
	 *            connect to.
	 * \param[in] port the port of the broker to connect to.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*connect)(artik_mqtt_handle client, const char *host,
							int port);
	/**
	 * Disconnect from the broker.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*disconnect)(artik_mqtt_handle client);
	/**
	 * Subscribe to a topic.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] qos the requested Quality of Service for
	 *            this subscription.
	 * \param[in] msgtopic the subscription pattern.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*subscribe)(artik_mqtt_handle client, int qos,
					const char *msgtopic);
	/**
	 * Unsubscribe from a topic.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] msgtopic the unsubscription pattern.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*unsubscribe)(artik_mqtt_handle client,
				const char *msgtopic);
	/**
	 * Publish a message on a given topic.
	 * \param[in] client Pointer of an artik mqtt handle
	 * \param[in] qos integer value 0, 1 or 2 indicating the Quality
	 *            of Service to be used for the message.
	 * \param[in] retain set to true to make the message retained.
	 * \param[in] msg_topic a message will be published on which.
	 * \param[in] payload_len the size of the payload (bytes).
	 *            Valid values are between 0 and 268,435,455.
	 * \param[in] msg_content The published message content.
	 * \return S_OK on success, otherwise a negative error value.
	 */
	artik_error(*publish)(artik_mqtt_handle client, int qos,
				bool retain, const char *msg_topic,
				int payload_len, const char *msg_content);
} artik_mqtt_module;

extern const artik_mqtt_module mqtt_module;

#ifdef __cplusplus
}
#endif

#endif /* ARTIK_MQTT_H_ */
