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

#ifndef __TIZEN_MEDIA_SCMIRRORING_SINK_H__
#define __TIZEN_MEDIA_SCMIRRORING_SINK_H__

#include <scmirroring_type.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file scmirroring_sink.h
 * @brief This file contains the screen mirroring source API and functions related with screen mirroring as sink device.
 */

/**
 * @addtogroup CAPI_MEDIA_SCREEN_MIRRORING_MODULE
 * @{
 */

/**
 * @brief Creates a new screen mirroring sink handle.
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks You must release @a scmirroring_sink using scmirroring_sink_destroy().
 *
 * @param[out] scmirroring_sink	A newly returned handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @post The screen mirroring state will be #SCMIRRORING_STATE_NULL
 *
 * @see scmirroring_sink_destroy()
 */
int scmirroring_sink_create(scmirroring_sink_h *scmirroring_sink);

/**
 * @brief Registers a callback function to be called when state change happens.
 * @details This function registers user callback and this callback is called when each status is changed.
 *
 * @since_tizen 2.4
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[in] callback The callback function to invoke
 * @param[in] user_data The user data passed to the callback registration function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 *
 * @see scmirroring_sink_create()
 */
int scmirroring_sink_set_state_changed_cb(scmirroring_sink_h scmirroring_sink, scmirroring_sink_state_cb callback, void *user_data);

/**
 * @brief Sets server IP and port.
 *
 * @since_tizen 2.4
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[in] ip The server IP address to connect to
 * @param[in] port The server port to connect to
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 *
 * @see scmirroring_sink_create()
 */
int scmirroring_sink_set_ip_and_port(scmirroring_sink_h scmirroring_sink, const char *ip, const char *port);

/**
 * @brief Pass window handle created by application and surface type(x11/evas).
 * @details This function will use handle created by the application to set the overlay &
 *          display on the surface passed by the application
 *
 * @since_tizen 2.4
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[in] type Surface type(x11/evas)
 * @param[in] display_surface The display_surface created by application to force sink to display content over it
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 *
 * @see scmirroring_sink_create()
 */
int scmirroring_sink_set_display(scmirroring_sink_h scmirroring_sink, scmirroring_display_type_e type, void *display_surface);

/**
 * @brief Sets resolutions of screen mirroring sink.
 * @details This function sets resolutions of screen mirroring sink using scmirroring_resolution_e as following.
 *          (ex. SCMIRRORING_RESOLUTION_1920x1080_P30 | SCMIRRORING_RESOLUTION_1280x720_P30)
 *          Use it only when you want to set specific resolutions but if sreen mirroring source dose not support
 *          the resolutions which you set, the screen mirroring sink will be disconnected.
 *
 * @since_tizen 2.4
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[in] resolution Resolution of screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 *
 * @see scmirroring_sink_create()
 */
int scmirroring_sink_set_resolution(scmirroring_sink_h scmirroring_sink, int resolution);

/**
 * @brief Prepares the screen mirroring sink handle and allocates specific resources.
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre The screen mirroring state should be #SCMIRRORING_SINK_STATE_NULL
 * @post The screen mirroring state will be #SCMIRRORING_SINK_STATE_PREPARED
 *
 * @see scmirroring_sink_create()
 * @see scmirroring_sink_set_state_changed_cb()
 */
int scmirroring_sink_prepare(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Creates connection and prepare for receiving data from SCMIRRORING source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre The screen mirroring state should be #SCMIRRORING_SINK_STATE_PREPARED
 * @post The screen mirroring state will be #SCMIRRORING_SINK_STATE_CONNECTED
 *
 * @see scmirroring_sink_create()
 * @see scmirroring_sink_set_state_changed_cb()
 * @see scmirroring_sink_prepare()
 */
int scmirroring_sink_connect(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Start receiving data from the SCMIRRORING source and display it(mirror).
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state should be #SCMIRRORING_SINK_STATE_CONNECTED
 * @post The screen mirroring state will be #SCMIRRORING_SINK_STATE_PLAYING
 *
 * @see scmirroring_sink_create()
 * @see scmirroring_sink_set_state_changed_cb()
 * @see scmirroring_sink_prepare()
 * @see scmirroring_sink_connect()
 */
int scmirroring_sink_start(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Pauses receiving data from the SCMIRRORING source.
 * @details This function pauses receiving data from the SCMIRRORING source,
 *	which means it sends RTSP PAUSE message to source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre The screen mirroring state should be #SCMIRRORING_SINK_STATE_PLAYING
 * @post The screen mirroring state will be #SCMIRRORING_SINK_STATE_PAUSED
 *
 * @see scmirroring_sink_create()
 * @see scmirroring_sink_set_state_changed_cb()
 * @see scmirroring_sink_prepare()
 * @see scmirroring_sink_connect()
 * @see scmirroring_sink_start()
 */
int scmirroring_sink_pause(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Pauses receiving data from the SCMIRRORING source.
 * @details This function pauses receiving data from the SCMIRRORING source, which means it sends RTSP PLAY message to source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre The screen mirroring state should be #SCMIRRORING_SINK_STATE_PAUSED
 * @post The screen mirroring state will be #SCMIRRORING_SINK_STATE_PLAYING
 *
 * @see scmirroring_sink_pause()
 */
int scmirroring_sink_resume(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Disconnects and stops receiving data from the SCMIRRORING source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre The screen mirroring state should be #SCMIRRORING_SINK_STATE_CONNECTED
 * 	or #SCMIRRORING_SINK_STATE_PLAYING or #SCMIRRORING_SINK_STATE_PAUSED
 * @post The screen mirroring state will be #SCMIRRORING_SINK_STATE_DISCONNECTED
 *
 * @see scmirroring_sink_create()
 * @see scmirroring_sink_set_state_changed_cb()
 * @see scmirroring_sink_prepare()
 * @see scmirroring_sink_connect()
 * @see scmirroring_sink_start()
 */
int scmirroring_sink_disconnect(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Unprepares screen mirroring.
 * @details This function unprepares screen mirroring, which closes specific resources.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @post The screen mirroring state will be #SCMIRRORING_STATE_NULL
 *
 * @see scmirroring_sink_create()
 * @see scmirroring_sink_set_state_changed_cb()
 * @see scmirroring_sink_prepare()
 */
int scmirroring_sink_unprepare(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Unregisters the callback function user registered.
 *
 * @since_tizen 2.4
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 *
 * @see scmirroring_sink_create()
 * @see scmirroring_sink_set_state_changed_cb()
 */
int scmirroring_sink_unset_state_changed_cb(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Destroys screen mirroring sink handle.
 *
 * @since_tizen 2.4
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre The screen mirroring state should be #SCMIRRORING_STATE_NULL
 *
 * @see scmirroring_sink_create()
 */
int scmirroring_sink_destroy(scmirroring_sink_h scmirroring_sink);

/**
 * @brief Gets negotiated video codec of screen mirroring sink.
 * @details The video codec is negotiated by screen mirroring source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[out] codec Codec of video
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state must be #SCMIRRORING_SINK_STATE_CONNECTED or #SCMIRRORING_SINK_STATE_PLAYING
 */
int scmirroring_sink_get_negotiated_video_codec(scmirroring_sink_h *scmirroring_sink, scmirroring_video_codec_e *codec);

/**
 * @brief Gets negotiated video resolution of screen mirroring sink.
 * @details The video resolution is negotiated by screen mirroring source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[out] width Width of video
 * @param[out] height Height of video
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state must be #SCMIRRORING_SINK_STATE_CONNECTED or #SCMIRRORING_SINK_STATE_PLAYING
 */
int scmirroring_sink_get_negotiated_video_resolution(scmirroring_sink_h *scmirroring_sink, int *width, int *height);

/**
 * @brief Gets negotiated frame rate of screen mirroring sink.
 * @details The video frame rate is negotiated by screen mirroring source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[out] frame_rate Frame rate of video
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state must be #SCMIRRORING_SINK_STATE_CONNECTED or #SCMIRRORING_SINK_STATE_PLAYING
 */
int scmirroring_sink_get_negotiated_video_frame_rate(scmirroring_sink_h *scmirroring_sink, int *frame_rate);

/**
 * @brief Gets negotiated audio codec of screen mirroring sink.
 * @details The audio codec is negotiated by screen mirroring source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[out] codec Codec of audio
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state must be #SCMIRRORING_SINK_STATE_CONNECTED or #SCMIRRORING_SINK_STATE_PLAYING
 */
int scmirroring_sink_get_negotiated_audio_codec(scmirroring_sink_h *scmirroring_sink, scmirroring_audio_codec_e *codec);

/**
 * @brief Gets negotiated audio channel of screen mirroring sink.
 * @details The audio channel is negotiated by screen mirroring source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[out] channel Channel of audio
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state must be #SCMIRRORING_SINK_STATE_CONNECTED or #SCMIRRORING_SINK_STATE_PLAYING
 */
int scmirroring_sink_get_negotiated_audio_channel(scmirroring_sink_h *scmirroring_sink, int *channel);

/**
 * @brief Gets negotiated audio sample rate of screen mirroring sink.
 * @details The audio sample rate is negotiated by screen mirroring source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[out] sample_rate Sample rate of audio
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state must be #SCMIRRORING_SINK_STATE_CONNECTED or #SCMIRRORING_SINK_STATE_PLAYING
 */
int scmirroring_sink_get_negotiated_audio_sample_rate(scmirroring_sink_h *scmirroring_sink, int *sample_rate);

/**
 * @brief Gets negotiated audio bitwidth of screen mirroring sink.
 * @details The audio bitwidth is negotiated by screen mirroring source.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] scmirroring_sink The handle to the screen mirroring sink
 * @param[out] bitwidth Bitwidth of audio
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SCMIRRORING_ERROR_NONE Successful
 * @retval #SCMIRRORING_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SCMIRRORING_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SCMIRRORING_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SCMIRRORING_ERROR_NOT_SUPPORTED Not supported
 * @retval #SCMIRRORING_ERROR_UNKNOWN Unknown Error
 *
 * @pre Create a screen mirroring sink handle by calling scmirroring_sink_create().
 * @pre Register user callback by calling scmirroring_sink_set_state_changed_cb().
 * @pre Call scmirroring_sink_prepare()
 * @pre Call scmirroring_sink_connect()
 * @pre The screen mirroring state must be #SCMIRRORING_SINK_STATE_CONNECTED or #SCMIRRORING_SINK_STATE_PLAYING
 */
int scmirroring_sink_get_negotiated_audio_bitwidth(scmirroring_sink_h *scmirroring_sink, int *bitwidth);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __TIZEN_MEDIA_SCMIRRORING_SINK_H__ */
