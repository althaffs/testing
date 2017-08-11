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

#ifndef	__ARTIK_SENSOR_H__
#define	__ARTIK_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "artik_error.h"
#include "artik_types.h"

/*! \file artik_sensor.h
 *
 * \brief SENSOR module definition
 *
 * Definitions and functions for accessing
 * to the SENSOR module and performing operations
 * on hardware IOs
 *
 * \example sensor_test/artik_sensor_test.c
 */

/*!
 * \brief Maximum length for SENSOR names
 *
 * Maximum length allowed for user-friendly
 * names assigned when requesting a SENSOR.
 */
#define MAX_NAME_LEN		64

/*!
 *  \brief SENSOR device type
 *
 *  Device sensor used to operate on specific type of sensor
 *  store in the SENSOR module.
 */
typedef void *artik_sensor_ops;

/*!
 *  \brief SENSOR handle type
 *
 *  Handle type used to carry instance specific
 *  information for a SENSOR object.
 */
typedef void *artik_sensor_handle;

/*!
 *  \brief SENSOR device type
 *
 *  Type for specifying the type of device
 *  sensor use by the SENSOR module.
 */
typedef enum {
	ARTIK_SENSOR_ACCELEROMETER = 0x1L,
	ARTIK_SENSOR_HUMIDITY = 0x4L,
	ARTIK_SENSOR_LIGHT = 0x8L,
	ARTIK_SENSOR_TEMPERATURE = 0x10L,
	ARTIK_SENSOR_PROXIMITY = 0x20L,
	ARTIK_SENSOR_FLAME = 0x40L,
	ARTIK_SENSOR_HALL = 0x80L,
	ARTIK_SENSOR_BAROMETER = 0x100L,
	ARTIK_SENSOR_GYRO = 0x101L,
	ARTIK_SENSOR_NONE = 0
} artik_sensor_device_t;

/*! \struct artik_sensor_config
 *  \brief SENSOR configuration structure
 *
 *  Structure containing the configuration elements
 *  for a single requested SENSOR
 */
typedef struct {
	/*
	 *  \brief Pin used by the SENSOR module.
	 */
	artik_sensor_device_t type;
	/*
	 *  \brief Friendly name for the SENSOR module.
	 */
	char *name;

	/*!
	 *  \brief Pointer to config for internal use by the API.
	 */
	const void *config;
	/*!
	 *  \brief Pointer to data for internal use by the API.
	 */
	void *data_user;

} artik_sensor_config;

/*! \struct artik_sensor_accelerometer
 *  \brief SENSOR ACCELEROMETER devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a ACCELEROMETER SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested
	 *             ACCELEROMETER instance to be released.
	 *             This handle is returned by the
	 *             'request' function.
	 *  \param[in,out] config config is the configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release a TEMPERATURE SYS SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested
	 *             ACCELEROMETER instance to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);
	/*!
	 *  \brief get_speed_x permit to return the speed base on
	 *         the axis X
	 *
	 *  \param[in] handle handle tied to the requested
	 *             ACCELEROMETER communication module instance to be
	 *             released.
	 *             This handle is returned by the
	 *             'get_accelerometer_sensor' function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_speed_x) (artik_sensor_handle handle,
				   int *store);
	/*!
	 *  \brief get_speed_y permit to return the speed base on the axis Y
	 *
	 *  \param[in] handle handle tied to the requested ACCELEROMETER
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_accelerometer_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_speed_y) (artik_sensor_handle handle,
				   int *store);
	/*!
	 *  \brief get_speed_z permit to return the speed base on the axis Z
	 *
	 *  \param[in] handle handle tied to the requested ACCELEROMETER
	 *             communication
	 *             module instance to be released.
	 *             This handle is returned by the 'get_accelerometer_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_speed_z) (artik_sensor_handle handle,
				   int *store);

} artik_sensor_accelerometer;

/*! \struct artik_sensor_gyro
 *  \brief SENSOR GYROMETER devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a GYROMETER SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested GYROMETER
	 *             instance to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config config is the configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release the SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested GYROMETER instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);
	/*!
	 *  \brief get_yaw permit to return the Yaw as measured by the sensor
	 *
	 *  \param[in] handle handle tied to the requested GYROMETER
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_gyro_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_yaw) (artik_sensor_handle handle,
				   int *store);
	/*!
	 *  \brief get_roll returns the Roll value as measured by the sensor
	 *
	 *  \param[in] handle handle tied to the requested GYROMETER
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_gyro_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_roll) (artik_sensor_handle handle,
				   int *store);
	/*!
	 *  \brief get_pitch returns the Pitch value as measured by the sensor
	 *
	 *  \param[in] handle handle tied to the requested GYROMETER
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_gyro_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_pitch) (artik_sensor_handle handle,
				   int *store);

} artik_sensor_gyro;

/*! \struct artik_sensor_humidity
 *  \brief SENSOR HUMIDITY devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a HUMIDITY SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested HUMIDITY instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config config is the configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release a TEMPERATURE SYS SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested HUMIDITY instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);
	/*!
	 *  \brief get_wet permit to return the percent of the environement
	 *         humidity
	 *
	 *  \param[in] handle handle tied to the requested HUMIDITY
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_humidity_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_humidity) (artik_sensor_handle handle,
				    int *store);

} artik_sensor_humidity;

/*! \struct artik_sensor_light
 *  \brief SENSOR LIGHT devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a LIGHT SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested LIGHT instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config config is the configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release a TEMPERATURE SYS SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested LIGHT instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);
	/*!
	 *  \brief get_intensity permit to return the percent of the
	 *         environement brightness
	 *
	 *  \param[in] handle handle tied to the requested LIGHT
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_light_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_intensity) (artik_sensor_handle handle,
				     int *store);

} artik_sensor_light;

/*! \struct artik_sensor_temperature
 *  \brief SENSOR TEMPERATURE devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a TEMPERATURE SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested TEMPERATURE
	 *             instance to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config config is the configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release a TEMPERATURE SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested TEMPERATURE
	 *             instance to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);
	/*!
	 *  \brief get_celsius permit to return the degree of environement
	 *         in celsius
	 *
	 *  \param[in] handle handle tied to the requested TEMPERATURE
	 *             communication module instance to be released.
	 *             This handle is returned by the
	 *             'get_temperature_sensor' function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_celsius) (artik_sensor_handle handle,
				   int *store);
	/*!
	 *  \brief get_fahrenheit permit to return the degree of environement
	 *         in fahrenheit
	 *
	 *  \param[in] handle handle tied to the requested TEMPERATURE
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_temperature_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_fahrenheit) (artik_sensor_handle handle,
				      int *store);

} artik_sensor_temperature;

/*! \struct artik_sensor_proximity
 *  \brief SENSOR PROXIMITY devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a PROXIMITY SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested PROXIMITY instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config config is the configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release a PROXIMITY SENSOR instance
	 *
	 *  \param[in] handle handle tied to the requested PROXIMITY instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);
	/*!
	 *  \brief get_presence permit to return if an object is in front of
	 *         the sensor.
	 *
	 *  \param[in] handle handle tied to the requested PROXIMITY
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_proximity_sensor'
	 *             function.
	 *  \param[in] store permit to save the result of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_presence) (artik_sensor_handle handle,
				    int *store);

} artik_sensor_proximity;

/*! \struct artik_sensor_flame
 *  \brief SENSOR FLAME devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a FLAME SENSOR instance
	 *
	 *  \param[in] handle Handle tied to the requested FLAME instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config Configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release a FLAME SENSOR instance
	 *
	 *  \param[in] handle Handle tied to the requested FLAME instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);

	/*!
	 *  \brief get_signals permit to return if an object is in front
	 *         of the sensor.
	 *
	 *  \param[in] handle Handle tied to the requested FLAME
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_flame_sensor'
	 *             function.
	 *  \param[out] store Pointer to a variable returning the result
	 *              of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_signals) (artik_sensor_handle handle,
				   int *store);

} artik_sensor_flame;

/*! \struct artik_sensor_pressure
 *  \brief SENSOR PRESSURE devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a PRESSURE SENSOR instance
	 *
	 *  \param[in] handle Handle tied to the requested SENSOR instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config Configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release the PRESSURE SENSOR instance
	 *
	 *  \param[in] handle Handle tied to the requested SENSOR instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);

	/*!
	 *  \brief get_pressure returns the atmospheric pressure in millibars
	 *
	 *  \param[in] handle Handle tied to the requested PRESSURE SENSOR
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_pressure_sensor'
	 *             function.
	 *  \param[out] store Pointer to a variable returning the result
	 *              of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_pressure) (artik_sensor_handle handle,
				   int *store);

} artik_sensor_pressure;

/*! \struct artik_sensor_hall
 *  \brief SENSOR HALL devices data structure
 *
 *  Structure which serve as an interface
 *  for requested SENSOR device
 */
typedef struct {
	/*!
	 *  \brief Request a HALL SENSOR instance
	 *
	 *  \param[in] handle Handle tied to the requested SENSOR instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *  \param[in,out] config Configuration structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_sensor_handle * handle,
			       artik_sensor_config * config);
	/*!
	 *  \brief Release the HALL SENSOR instance
	 *
	 *  \param[in] handle Handle tied to the requested SENSOR instance
	 *             to be released.
	 *             This handle is returned by the 'request' function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_sensor_handle handle);

	/*!
	 *  \brief get_detection returns if the sensor can detect magnetic
	 *        field
	 *
	 *  \param[in] handle Handle tied to the requested HALL SENSOR
	 *             communication module instance to be released.
	 *             This handle is returned by the 'get_hall_sensor'
	 *             function.
	 *  \param[out] store Pointer to a variable returning the result
	 *              of the function.
	 *
	 *  \return S_OK and store the value into the parameter on success,
	 *          error code otherwise
	 */
	artik_error(*get_detection) (artik_sensor_handle handle,
				   int *store);

} artik_sensor_hall;

/*! \struct artik_sensor_module
 *
 *  \brief SENSOR module operations
 *
 *  Structure containing all the exposed operations exposed
 *  by the module to operate on a SENSOR instance
 */
typedef struct {
	/*!
	 *  \brief Request a SENSOR instance
	 *
	 *  \param[in,out] config Configuration to apply to the requested
	 *                 SENSOR.
	 *
	 *  \param[in,out] handle Handle tied to the requested SENSOR
	 *                 instance filled by the function.
	 *
	 *  \param[in] sensor Sensor tied to the requested SENSOR Device
	 *             instance to be released. This sensor is stored
	 *             statically in \ref artik_a5_platform.h or
	 *             artik_a10_platform with the specific config
	 *             structure.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request)(artik_sensor_config * config,
			       artik_sensor_handle * handle,
			       artik_sensor_ops * sensor);
	/*!
	 *  \brief List all sensor configuration for a SENSOR instance
	 *
	 *  \return The config list for the platform on success, NULL
	 *          otherwise
	 */
	artik_sensor_config * (*list)(void);
	/*!
	 *  \brief Get a device sensor config for instanciate a SENSOR device
	 *
	 *  \param[in] index Index of the sensor in the configuration table of
	 *             a Sensor device.
	 *  \param[in] type Type is the type of the Sensor device
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_sensor)(unsigned int index,
					    artik_sensor_device_t type);

	/*!
	 *  \brief Get Accelerometer config for instantiating a Sensor device
	 *
	 *  \param[in] index Index of the sensor in the configuration
	 *             table of a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_accelerometer_sensor)(unsigned int index);

	/*!
	 *  \brief Get Humidity sensor config for instantiating a Sensor device
	 *
	 *  \param[in] index Index of the sensor in the configuration
	 *             table of a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_humidity_sensor)(unsigned int index);

	/*!
	 *  \brief Get Light sensor config for instantiating a Sensor device
	 *
	 *  \param[in] index Index of the sensor in the configuration
	 *             table of a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_light_sensor)(unsigned int index);

	/*!
	 *  \brief Get Temperature sensor config for instantiating a Sensor
	 *         device
	 *
	 *  \param[in] index Index of the sensor in the configuration
	 *             table of a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_temperature_sensor)(unsigned int index);

	/*!
	 *  \brief Get Proximity sensor config for instantiating a Sensor
	 *         device
	 *
	 *  \param[in] index Index of the sensor in the configuration table of
	 *             a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_proximity_sensor)(unsigned int index);

	/*!
	 *  \brief Get Flame sensor config for instantiating a Sensor device
	 *
	 *  \param[in] index Index of the sensor in the configuration table of
	 *             a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_flame_sensor)(unsigned int index);

	/*!
	 *  \brief Get Pressure sensor config for instantiating a Sensor device
	 *
	 *  \param[in] index Index of the sensor in the configuration table of
	 *             a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_pressure_sensor)(unsigned int index);

	/*!
	 *  \brief Get Gyro sensor config for instantiating a Sensor device
	 *
	 *  \param[in] index Index of the sensor in the configuration table of
	 *             a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_gyro_sensor)(unsigned int index);

	/*!
	 *  \brief Get Hall sensor config for instantiating a Sensor device
	 *
	 *  \param[in] index Index of the sensor in the configuration table of
	 *             a Sensor device.
	 *
	 *  \return S_OK on success, NULL otherwise
	 */
	artik_sensor_config * (*get_hall_sensor)(unsigned int index);

} artik_sensor_module;

extern artik_sensor_module sensor_module;

#ifdef __cplusplus
}
#endif
#endif				/* __ARTIK_SENSOR_H__ */
