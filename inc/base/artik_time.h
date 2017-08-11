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

#ifndef	__ARTIK_TIME_H__
#define	__ARTIK_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "artik_error.h"
#include "artik_types.h"

	/*! \file artik_time.h
	 *
	 *  \brief Time module definition
	 *
	 *  Definitions and functions for accessing
	 *  the Time module and performing operations.
	 *
	 *  \example time_test/artik_time_test.c
	 */

	/*!
	 *  \brief Default format for Time module
	 *
	 *  Base representation of time into a string
	 *  with the Time module.
	 *  Each character has a specification :
	 *  '%I' = 'hours' for 12h format or '%H' = 'hours' for 24h format
	 *  '%M' = 'minutes'
	 *  '%S' = 'seconds'
	 *  '%w' = 'day of week'
	 *  '%d' = 'day of month'
	 *  '%m' = 'month'
	 *  '%Y' = 'year'
	 */
#define ARTIK_TIME_DFORMAT	"%I:%M:%S-%w-%d/%m/%Y"

	/*!
	 *  \brief Alarm handle type
	 *
	 *  Handle type used to carry instance specific
	 *  alarm information for the Time API.
	 */
	typedef void *artik_alarm_handle;

	/*!
	 *  \brief Time storage type
	 *
	 *  Storage type used to store the time
	 *  information of a Time object.
	 */
	typedef struct {
		/*!
		 *  \brief Seconds elapsed in the current minute (0-59)
		 */
		unsigned int second;
		/*!
		 *  \brief Minutes elapsed in the current hour (0-59)
		 */
		unsigned int minute;
		/*!
		 *  \brief Hours starting from midnight (0-23)
		 */
		unsigned int hour;
		/*!
		 *  \brief Day of the month (1-31)
		 */
		unsigned int day;
		/*!
		 *  \brief Month starting from January (1-12)
		 */
		unsigned int month;
		/*!
		 *  \brief Year
		 */
		unsigned int year;
		/*!
		 *  \brief Day of the week starting from Sunday (0-6)
		 */
		unsigned int day_of_week;
		/*!
		 *  \brief Milliseconds elapsed in the current second
		 */
		unsigned int msecond;
	} artik_time;

	/*!
	 *  \brief Time gmt type
	 *
	 *  GMT used to define the jetlag
	 *  of a Time object.
	 */
	typedef char artik_time_gmt;

	/*!
	 *  \brief Time millisecond type
	 *
	 *  Define the time cooldown in milliseconds.
	 */
	typedef unsigned int long artik_msecond;

	/*!
	 *  \brief Time zone type
	 *
	 *  Define the different time-zone.
	 */
	typedef enum {
		ARTIK_TIME_UTC = 0,
		ARTIK_TIME_GMT1,
		ARTIK_TIME_GMT2,
		ARTIK_TIME_GMT3,
		ARTIK_TIME_GMT4,
		ARTIK_TIME_GMT5,
		ARTIK_TIME_GMT6,
		ARTIK_TIME_GMT7,
		ARTIK_TIME_GMT8,
		ARTIK_TIME_GMT9,
		ARTIK_TIME_GMT10,
		ARTIK_TIME_GMT11,
		ARTIK_TIME_GMT12
	} artik_time_zone;

	/*!
	 * \brief     This callback function gets triggered after timeout
	 * \param[in] user_data The user data passed from the register
	 *            callback function
	 */
	typedef void(*alarm_callback)(void *user_data);

	/*! \struct artik_time_module
	 *
	 *  \brief Time module operations
	 *
	 *  Structure containing all the exposed operations exposed
	 *  by the module to operate on with the Time API
	 */
	typedef struct {
		/*!
		 *  \brief Set Time with the Time API
		 *
		 *  \param[in] date Date is the time to set for the
		 *             system clock.
		 *  \param[in] gmt The time-zone of the variable date
		 *
		 *  \return S_OK on success, error code otherwise
		 */
		artik_error(*set_time) (artik_time date, artik_time_zone gmt);
		/*!
		 *  \brief Get Time with the Time API
		 *
		 *  \param[in] gmt GMT is the target of local time.
		 *  \param[out] date Date is the time to get for the
		 *             system clock.
		 *
		 *  \return S_OK on success, error code otherwise
		 */
		artik_error(*get_time) (artik_time_zone gmt, artik_time * date);
		/*!
		 *  \brief Get Time String with the Time API
		 *
		 *  \param[out] date_str Preallocated string to be filled
		 *              up with the date.
		 *  \param[in] size Size of the preallocated date string
		 *  \param[in] format Format string to use for generating
		 *             the date string.
		 *  \param[in] gmt Local time zone in GMT format.
		 *
		 *  \return S_OK on success,
		 *          E_BAD_ARGS if 'date_str' and/or 'size' and/or
		 *          'format' is/are not well defined,
		 *          error code otherwise
		 */
		 artik_error(*get_time_str) (char *date_str, int size,
				char *const format, artik_time_zone gmt);
		/*!
		 *  \brief Get system tick
		 *
		 *  \return Number of milliseconds corresponding to the
		 *          system tick
		 */
		 artik_msecond(*get_tick) (void);
		/*!
		 *  \brief Create an alarm to set off after an amount of
		 *         time elapsed from now.
		 *
		 *  \param[in] gmt Local time zone in GMT format.
		 *  \param[in] value Number of seconds to elapse before the
		 *             alarm is triggered
		 *  \param[out] handle Handle referencing the alarm for
		 *              later use.
		 *  \param[in] func The callback function which will be called
		 *             when the alarm is triggered (mandatory).
		 *  \param[in] user_data The user data passed from the register
		 *             callback function.
		 *
		 *  \return S_OK on success, error code otherwise
		 */
		 artik_error(*create_alarm_second) (artik_time_zone gmt,
						    artik_msecond value,
						    artik_alarm_handle * handle,
						    alarm_callback func,
						    void *user_data);
		/*!
		 *  \brief Create an alarm to set off at a specific date
		 *
		 *  \param[in] gmt Local time zone in GMT format.
		 *  \param[in] value Date at which the alarm must be triggered
		 *  \param[out] handle Handle referencing the alarm for
		 *              later use.
		 *  \param[in] func The callback function which will be
		 *             called when the alarm is triggered (mandatory).
		 *  \param[in] user_data The user data passed from the register
		 *             callback function.
		 *
		 *  \return S_OK on success, error code otherwise
		 */
		 artik_error(*create_alarm_date) (artik_time_zone gmt,
						  artik_time value,
						  artik_alarm_handle * handle,
						  alarm_callback func,
						  void *user_data);
		/*!
		 *  \brief Wait for an alarm to set off. This function
		 *         is blocking until the alarm is triggered.
		 *
		 *  \param[in] handle Handle of the alarm to wait for.
		 *
		 *  \return S_OK on success, error code otherwise
		 */

		 artik_error(*delete_alarm) (artik_alarm_handle handle);
		/*!
		 *  \brief Get the amount of remaining seconds before an alarm
		 *         is triggered
		 *
		 *  \param[in] handle Handle of the alarm to delete.
		 *  \param[out] msecond Number of seconds from now after which
		 *              the alarm is triggered.
		 *
		 *  \return S_OK on success, error code otherwise
		 */
		 artik_error(*get_delay_alarm) (artik_alarm_handle handle,
						artik_msecond * msecond);
		/*!
		 *  \brief Synchronize system date with remote NTP server
		 *
		 *  \param[in] hostname Hostname of NTP server.
		 *
		 *  \return S_OK on success, error code otherwise
		 */
		 artik_error(*sync_ntp) (const char *hostname);
		/*
		 * \brief Compare two dates
		 *
		 * \param[in] date1 First date to compare
		 * \param[in] date2 Second date to compare
		 *
		 * \return 0 if dates are equal
		 *         -1 if date1 occurs before date2
		 *         1 if date1 occurs after date2
		 *         -2 if the parameters are invalid
		 */
		 int (*compare_dates)(const artik_time *date1, const artik_time *date2);

	} artik_time_module;

	extern artik_time_module time_module;

#ifdef __cplusplus
}
#endif
#endif				/* __ARTIK_TIME_H__ */
