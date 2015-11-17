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

#ifndef __TIZEN_MEDIA_SCMIRRORING_SRC_H__
#define __TIZEN_MEDIA_SCMIRRORING_SRC_H__

#include <scmirroring_internal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file scmirroring_src.h
 * @brief This file contains the screen mirroring source API and functions related with screen mirroring as source device.
 */

/**
 * @brief Creates screen mirroring source handle.
 * @remarks You must release @a scmirroring_src using scmirroring_src_destroy().
 *
 * @param[out] scmirroring_src The handle to screen mirroring source
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @see scmirroring_src_destroy()
 */
int scmirroring_src_create(scmirroring_src_h *scmirroring_src);

/**
 * @brief Registers user callback to get status of screen mirroring.
 * @details This function registers user callback and this callback is called when each status is changed.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 * @param[in] callback The callback function to invoke
 * @param[in] user_data The user data passed to the callback registration function
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 *
 * @see scmirroring_src_create()
 */
int scmirroring_src_set_state_changed_cb(scmirroring_src_h scmirroring_src, scmirroring_state_cb callback, void *user_data);

/**
 * @brief Sets connection mode of screen mirroring.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 * @param[in] connection_mode connection mode of screen mirroring
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 *
 * @see scmirroring_src_create()
 */
int scmirroring_src_set_connection_mode(scmirroring_src_h scmirroring_src, scmirroring_connection_mode_e connection_mode);

/**
 * @brief Sets IP address and port number of screen mirroring source.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 * @param[in] ip Server IP address
 * @param[in] port Server port
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 *
 * @see scmirroring_src_create()
 */
int scmirroring_src_set_ip_and_port(scmirroring_src_h scmirroring_src, const char *ip, const char *port);

/**
 * @brief Sets resolution of screen mirroring source.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 * @param[in] resolution Resolution of screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 *
 * @see scmirroring_src_create()
 */
int scmirroring_src_set_resolution(scmirroring_src_h scmirroring_src, scmirroring_resolution_e resolution);

/**
 * @brief Sets name of screen mirroring source server.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 * @param[in] name Name of screen mirroring source server
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 *
 * @see scmirroring_src_create()
 */
int scmirroring_src_set_server_name(scmirroring_src_h scmirroring_src, const char *name);

/**
 * @brief Enables/Disables screen mirroring multisink.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 * @param[in] multisink Ability to send to multisink
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 *
 * @see scmirroring_src_create()
 */
int scmirroring_src_set_multisink_ability(scmirroring_src_h scmirroring_src, scmirroring_multisink_e multisink);

/**
 * @brief Connects to server for screen mirroring as source, asynchronously.
 * @details This function launches server and connects to the server for screen mirroring as source to command server to start/pause/resume/stop.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_CONNECTION_TIME_OUT Connection timeout
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 * @pre Register user callback by calling scmirroring_src_set_state_changed_cb().
 * @post The screen mirroring state will be SCMIRRORING_STATE_READY
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 */
int scmirroring_src_connect(scmirroring_src_h scmirroring_src);

/**
 * @brief Prepares screen mirroring as source.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 * @pre Register user callback by calling scmirroring_src_set_state_changed_cb().
 * @pre Connects to server for screen mirroring source by calling scmirroring_src_connect().
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 * @see scmirroring_src_connect()
 */
int scmirroring_src_prepare(scmirroring_src_h scmirroring_src);

/**
 * @brief Starts screen mirroring, asynchronously.
 * @details This function starts screen mirroring, which means it starts to negotiate and stream RTP multimedia data.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_CONNECTION_TIME_OUT Connection timeout
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 * @pre Register user callback by calling scmirroring_src_set_state_changed_cb().
 * @pre Call scmirroring_src_connect()
 * @pre Call scmirroring_src_prepare()
 * @post The screen mirroring state will be SCMIRRORING_STATE_CONNECTION_WAIT if server starts to listen
 * @post The screen mirroring state will be SCMIRRORING_STATE_CONNECTED if client connects to the server
 * @post The screen mirroring state will be SCMIRRORING_STATE_PLAYING if server starts to stream multimedia data
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 * @see scmirroring_src_connect()
 * @see scmirroring_src_prepare()
 */
int scmirroring_src_start(scmirroring_src_h scmirroring_src);

/**
 * @brief Pauses screen mirroring, asynchronously.
 * @details This function pauses screen mirroring, which means it sends RTSP PAUSE trigger message to sink.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre The screen mirroring state should be SCMIRRORING_STATE_PLAYING
 * @post The screen mirroring state will be SCMIRRORING_STATE_PAUSED
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 * @see scmirroring_src_prepare()
 * @see scmirroring_src_connect()
 * @see scmirroring_src_start()
 */
int scmirroring_src_pause(scmirroring_src_h scmirroring_src);

/**
 * @brief Resumes screen mirroring, asynchronously.
 * @details This function resumes screen mirroring, which means it sends RTSP PLAY message to sink.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre The screen mirroring state should be SCMIRRORING_STATE_PAUSED
 * @post The screen mirroring state will be SCMIRRORING_STATE_PLAYING
 *
 * @see scmirroring_src_pause()
 */
int scmirroring_src_resume(scmirroring_src_h scmirroring_src);

/**
 * @brief Stops screen mirroring, asynchronously.
 * @details This function stops screen mirroring, which means it sends RTSP TEARDOWN trigger message to sink.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_CONNECTION_TIME_OUT Connection timeout
 *
 * @pre The screen mirroring state should be SCMIRRORING_STATE_PAUSED or SCMIRRORING_STATE_PLAYING
 * @post The screen mirroring state will be SCMIRRORING_STATE_TEARDOWN
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 * @see scmirroring_src_prepare()
 * @see scmirroring_src_connect()
 * @see scmirroring_src_start()
 */
int scmirroring_src_stop(scmirroring_src_h scmirroring_src);

/**
 * @brief Unprepares screen mirroring.
 * @details This function unprepares screen mirroring, which closes specific resources.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_CONNECTION_TIME_OUT Connection timeout
 *
 * @pre The screen mirroring state should be SCMIRRORING_STATE_READY
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 * @see scmirroring_src_connect()
 * @see scmirroring_src_prepare()
 */
int scmirroring_src_unprepare(scmirroring_src_h scmirroring_src);

/**
 * @brief Disconnects server for screen mirroring.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_CONNECTION_TIME_OUT Connection timeout
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 * @pre Register user callback by calling scmirroring_src_set_state_changed_cb().
 * @pre Connects to server for screen mirroring source by calling scmirroring_src_connect().
 * @post The screen mirroring state will be SCMIRRORING_STATE_NULL
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 * @see scmirroring_src_connect()
 */
int scmirroring_src_disconnect(scmirroring_src_h scmirroring_src);

/**
 * @brief Unregisters the callback function user registered
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 * @pre Register user callback by calling scmirroring_src_set_state_changed_cb().
 *
 * @see scmirroring_src_create()
 * @see scmirroring_src_set_state_changed_cb()
 */
int scmirroring_src_unset_state_changed_cb(scmirroring_src_h scmirroring_src);

/**
 * @brief Destroys server and screen mirroring source handle.
 *
 * @param[in] scmirroring_src The handle to screen mirroring source
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_CONNECTION_TIME_OUT Connection timeout
 *
 * @pre Create a screen mirroring source handle by calling scmirroring_src_create().
 * @pre The screen mirroring state should be SCMIRRORING_STATE_NULL
 *
 * @see scmirroring_src_create()
 */
int scmirroring_src_destroy(scmirroring_src_h scmirroring_src);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_SCMIRRORING_SRC_H__ */
