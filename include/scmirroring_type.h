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

#ifndef __TIZEN_MEDIA_SCMIRRORING_TYPE_H__
#define __TIZEN_MEDIA_SCMIRRORING_TYPE_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file scmirroring_type.h
 * @brief This file contains API related to screen mirroring enumerations for classes of errors.
 */

/**
 * @addtogroup CAPI_MEDIA_SCREEN_MIRRORING_MODULE
 * @{
 */

/**
 * @brief	The handle to the screen mirroring source.
 * @since_tizen 2.4
 */
typedef void *scmirroring_src_h;

/**
 * @brief	The handle to the screen mirroring sink.
 * @since_tizen 2.4
 */
typedef void *scmirroring_sink_h;

/**
 * @brief Enumeration for screen mirroring error.
 * @since_tizen 2.4
 */
typedef enum {
	SCMIRRORING_ERROR_NONE					= TIZEN_ERROR_NONE,					/**< Successful */
	SCMIRRORING_ERROR_INVALID_PARAMETER	= TIZEN_ERROR_INVALID_PARAMETER,		/**< Invalid parameter */
	SCMIRRORING_ERROR_OUT_OF_MEMORY		= TIZEN_ERROR_OUT_OF_MEMORY,			/**< Out of memory */
	SCMIRRORING_ERROR_INVALID_OPERATION	= TIZEN_ERROR_INVALID_OPERATION,		/**< Invalid Operation */
	SCMIRRORING_ERROR_CONNECTION_TIME_OUT	= TIZEN_ERROR_CONNECTION_TIME_OUT,  /**< Connection timeout */
	SCMIRRORING_ERROR_PERMISSION_DENIED	= TIZEN_ERROR_PERMISSION_DENIED,    /**< Permission denied */
	SCMIRRORING_ERROR_NOT_SUPPORTED		= TIZEN_ERROR_NOT_SUPPORTED,        /**< Not supported */
	SCMIRRORING_ERROR_UNKNOWN				= TIZEN_ERROR_UNKNOWN,        /**< Unknown error */
} scmirroring_error_e;

/**
 * @brief Enumeration for screen mirroring source state.
 * @since_tizen 2.4
 */
typedef enum {
	SCMIRRORING_STATE_NULL = 0,			/**< Screen mirroring is created, but not realized yet */
	SCMIRRORING_STATE_READY,				/**< Screen mirroring is ready to play media */
	SCMIRRORING_STATE_CONNECTION_WAIT,	/**< Screen mirroring is waiting for connection */
	SCMIRRORING_STATE_CONNECTED,		/**< Screen mirroring is connected */
	SCMIRRORING_STATE_PLAYING,			/**< Screen mirroring is now playing media */
	SCMIRRORING_STATE_PAUSED,          		 /**< Screen mirroring is paused while playing media */
	SCMIRRORING_STATE_TEARDOWN,			/**< Teardown Screen mirroring */
	SCMIRRORING_STATE_NONE,				/**< Screen mirroring is not created yet */
	SCMIRRORING_STATE_MAX				/* Number of screen mirroring states */
} scmirroring_state_e;

/**
 * @brief Enumeration for screen mirroring sink state.
 * @since_tizen 2.4
 */
typedef enum {
	SCMIRRORING_SINK_STATE_NONE = 0,		/**< Screen mirroring is not created yet */
	SCMIRRORING_SINK_STATE_NULL,			/**< Screen mirroring is created, but not prepared yet */
	SCMIRRORING_SINK_STATE_PREPARED,	/**< Screen mirroring is prepared to play media */
	SCMIRRORING_SINK_STATE_CONNECTED,	/**< Screen mirroring is connected */
	SCMIRRORING_SINK_STATE_PLAYING,		/**< Screen mirroring is now playing media */
	SCMIRRORING_SINK_STATE_PAUSED,           /**< Screen mirroring is paused while playing media */
	SCMIRRORING_SINK_STATE_DISCONNECTED,			/**< Screen mirroring is dieconnected */
	SCMIRRORING_SINK_STATE_MAX			/* Number of screen mirroring states */
} scmirroring_sink_state_e;

/**
 * @brief Enumeration for screen mirroring resolution.
 * @since_tizen 2.4
 */
typedef enum {
	SCMIRRORING_RESOLUTION_UNKNOWN = 0,
	SCMIRRORING_RESOLUTION_1920x1080_P30 =(1 << 0),   /**< W-1920, H-1080, 30 fps*/
	SCMIRRORING_RESOLUTION_1280x720_P30 =(1 << 1),    /**< W-1280, H-720, 30 fps*/
	SCMIRRORING_RESOLUTION_960x540_P30 = (1 << 2),    /**< W-960, H-540, 30 fps*/
	SCMIRRORING_RESOLUTION_864x480_P30 = (1 << 3),    /**< W-864, H-480, 30 fps*/
	SCMIRRORING_RESOLUTION_720x480_P60 = (1 << 4),    /**< W-720, H-480, 30 fps*/
	SCMIRRORING_RESOLUTION_640x480_P60 = (1 << 5),    /**< W-640, H-480, 60 fps*/
	SCMIRRORING_RESOLUTION_640x360_P30 = (1 << 6),    /**< W-640, H-360, 30 fps*/
	SCMIRRORING_RESOLUTION_MAX = 128,
} scmirroring_resolution_e;

/**
 * @brief Enumeration for screen mirroring connection mode.
 * @since_tizen 2.4
 */
typedef enum {
	SCMIRRORING_CONNECTION_WIFI_DIRECT = 0,		/**< Wi-Fi Direct*/
	SCMIRRORING_CONNECTION_MAX,					/* Number of Connection mode */
} scmirroring_connection_mode_e;

/**
 * @brief Enumeration for screen mirroring display surface type
 * @since_tizen 2.4
 */
typedef enum {
	SCMIRRORING_DISPLAY_TYPE_OVERLAY = 0,	/**< Use overlay surface to display streaming multimedia data */
	SCMIRRORING_DISPLAY_TYPE_EVAS,			/**< Use Evas pixmap surface to display streaming multimedia data */
	SCMIRRORING_DISPLAY_TYPE_MAX
} scmirroring_display_type_e;

/**
 * @brief Called when each status is changed
 * @since_tizen 2.4
 *
 * @details This callback is called for state and error of screen mirroring
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

/**
 * @brief Called when each status is changed
 * @since_tizen 2.4
 *
 * @details This callback is called for state and error of screen mirroring sink
 *
 * @param[in] error     The error code
 * @param[in] state     The screen mirroring sink state
 * @param[in] user_data The user data passed from the scmirroring_sink_set_state_cb() function
 *
 * @pre scmirroring_sink_create()
 *
 * @see scmirroring_sink_create()
 */
typedef void(*scmirroring_sink_state_cb)(scmirroring_error_e error, scmirroring_sink_state_e state, void *user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __TIZEN_MEDIA_SCMIRRORING_TYPE_H__ */
