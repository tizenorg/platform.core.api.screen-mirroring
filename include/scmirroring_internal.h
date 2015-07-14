/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __TIZEN_MEDIA_SCMIRRORING_INTERNAL_H__
#define __TIZEN_MEDIA_SCMIRRORING_INTERNAL_H__

#include <tizen.h>
#include <scmirroring_type.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file scmirroring_internal.h
 * @brief This file contains API related to screen mirroring internal enumerations for classes of errors.
 */

/**
 * @brief	The handle to the screen mirroring source.
 */
typedef void *scmirroring_src_h;

/**
 * @brief Enumeration for screen mirroring connection mode.
 */
typedef enum {
	SCMIRRORING_CONNECTION_WIFI_DIRECT = 0,     /**< Wi-Fi Direct*/
	SCMIRRORING_CONNECTION_MAX,                 /* Number of Connection mode */
} scmirroring_connection_mode_e;

/**
 * @brief Enumeration for screen mirroring source state.
 */
typedef enum {
	SCMIRRORING_STATE_NULL = 0,         /**< Screen mirroring is created, but not realized yet */
	SCMIRRORING_STATE_READY,            /**< Screen mirroring is ready to play media */
	SCMIRRORING_STATE_CONNECTION_WAIT,  /**< Screen mirroring is waiting for connection */
	SCMIRRORING_STATE_CONNECTED,        /**< Screen mirroring is connected */
	SCMIRRORING_STATE_PLAYING,          /**< Screen mirroring is now playing media */
	SCMIRRORING_STATE_PAUSED,           /**< Screen mirroring is paused while playing media */
	SCMIRRORING_STATE_TEARDOWN,         /**< Teardown Screen mirroring */
	SCMIRRORING_STATE_NONE,             /**< Screen mirroring is not created yet */
	SCMIRRORING_STATE_MAX               /* Number of screen mirroring states */
} scmirroring_state_e;

/**
 * @brief Called when each status is changed.
 *
 * @details This callback is called for state and error of screen mirroring.
 *
 * @param[in] error     The error code
 * @param[in] state     The screen mirroring state
 * @param[in] user_data The user data passed from the scmirroring_src_set_state_cb() function
 *
 * @pre scmirroring_src_create()
 *
 * @see scmirroring_src_create()
 */
typedef void(*scmirroring_state_cb)(scmirroring_error_e error, scmirroring_state_e state, void *user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_SCMIRRORING_INTERNAL_H__ */

