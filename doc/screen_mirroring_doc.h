/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __TIZEN_SCREEN_MIRRORING_DOC_H__
#define __TIZEN_SCREEN_MIRRORING_DOC_H__

/**
 * @file screen_mirroring_doc.h
 * @brief This file contains high level documentation for the Screen Mirroring Sink API
 */

/**
 * @defgroup CAPI_MEDIA_SCREEN_MIRRORING_MODULE Screen Mirroring
 * @brief The @ref CAPI_MEDIA_SCREEN_MIRRORING_MODULE API provides functions for screen mirroring as sink.
 * @ingroup CAPI_MEDIA_FRAMEWORK
 *
 * @section CAPI_MEDIA_SCREEN_MIRRORING_MODULE_HEADER Required Header
 *    \#include <scmirroring_sink.h>
 *
 * @section CAPI_MEDIA_SCREEN_MIRRORING_OVERVIEW Overview
 * The @ref CAPI_MEDIA_SCREEN_MIRRORING_MODULE API allows you to implement screen mirroring application as sink.
 * It gives the ability to connect to and disconnect from a screen mirroring source, and start, pause, and resume the screen mirroring sink, set the resolution or display, register state change callback function.
 *
 * @subsection CAPI_MEDIA_SCREEN_MIRRORING_SINK_LIFE_CYCLE_STATE_DIAGRAM State Diagram
 * The following diagram shows the life cycle and states of the screen mirroring sink.
 * @image html capi_media_screen_mirroring_sink_state_diagram.png
 *
 *
 * @subsection CAPI_MEDIA_SCREEN_MIRRORING_SINK_LIFE_CYCLE_STATE_TRANSITIONS State Transitions
 * <div><table class="doxtable" >
 *	   <tr>
 *		 <th><b>FUNCTION</b></th>
 * 	   <th><b>PRE-STATE</b></th>
 *		  <th><b>POST-STATE</b></th>
 *		  <th><b>SYNC TYPE</b></th>
 *	  </tr>
 *	  <tr>
 *		<td>scmirroring_sink_create()</td>
 *		  <td>NONE</td>
 *		 <td>NULL</td>
 *		<td>SYNC</td>
 * 	</tr>
 * 	   <tr>
 *		  <td>scmirroring_sink_destroy()</td>
 *		  <td>NULL</td>
 *		  <td>NONE</td>
 *		  <td>SYNC</td>
 *	   </tr>
 *	   <tr>
 *		 <td>scmirroring_sink_prepare()</td>
 *		<td>NULL</td>
 * 	   <td>PREPARED</td>
 * 	   <td>SYNC</td>
 *	   </tr>
 *	   <tr>
 *		 <td>scmirroring_sink_unprepare()</td>
 *		  <td>PREARED, DISCONNECTED</td>
 *		 <td>NULL</td>
 *		 <td>SYNC</td>
 *	  </tr>
 *	  <tr>
 *		<td>scmirroring_sink_connect()</td>
 *		 <td>PREARED</td>
 *		<td>CONNECTED</td>
 * 	   <td>ASYNC</td>
 *	   </tr>
 *	   <tr>
 *		 <td>scmirroring_sink_start()</td>
 *		  <td>CONNECTED</td>
 *		<td>PLAYING</td>
 *		  <td>ASYNC</td>
 *	  </tr>
 *	  <tr>
 *		<td>scmirroring_sink_disconnect()</td>
 *		<td>CONNECTED, PAUSED or PLAYING </td>
 *		  <td>DISCONNECTED </td>
 *		  <td>SYNC</td>
 *	  </tr>
 *	  <tr>
 *		<td>scmirroring_sink_pause()</td>
 *		 <td>PLAYING</td>
 * 	   <td>PAUSED</td>
 *		<td>ASYNC</td>
 * 	</tr>
 *	  <tr>
 *		<td>scmirroring_sink_resume()</td>
 *		 <td>PAUSED</td>
 * 	   <td>PLAYING</td>
 *		<td>ASYNC</td>
 * 	</tr>
 * </table></div>
 *
 *
 *@subsection CAPI_MEDIA_SCREEN_MIRRORING_SINK_LIFE_CYCLE_STATE_DEPENDENT_FUNCTIONS State Dependent Function Calls
 * The following table shows state-dependent function calls.
 * It is forbidden to call the functions listed below in wrong state.
 * Violation of this rule may result in unpredictable behavior.
 * <div><table class="doxtable" >
 * <tr>
 *    <th><b>FUNCTION</b></th>
 *    <th><b>VALID STATES</b></th>
 *    <th><b>DESCRIPTION</b></th>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_create() </td>
 *    <td>NONE</td>
 *    <td>-</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_destroy()</td>
 *    <td>NULL</td>
 *    <td></td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_set_state_changed_cb()</td>
 *    <td>NULL/ PREPARED/ CONNECTED/ PLAYING/ PAUSED/ DISCONNECTED</td>
 *    <td>This function must be called after scmirroring_sink_create().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_unset_state_changed_cb()</td>
 *    <td>NULL/ PREPARED/ CONNECTED/ PLAYING/ PAUSED/ DISCONNECTED</td>
 *    <td></td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_set_display()</td>
 *    <td>NULL</td>
 *    <td>This function must be called before scmirroring_sink_prepare().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_set_resolution()</td>
 *    <td>NULL</td>
 *    <td></td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_prepare() </td>
 *    <td>NULL</td>
 *    <td>This function must be called after scmirroring_sink_create().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_unprepare() </td>
 *    <td>PREPARED/ DISCONNECTED </td>
 *    <td></td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_set_ip_and_port() </td>
 *    <td>NULL/ PREPARED</td>
 *    <td>This function must be called before scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_connect()</td>
 *    <td>PREPARED</td>
 *    <td>This function must be called after scmirroring_sink_prepare().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_pause()</td>
 *    <td>PLAYING</td>
 *    <td>This function must be called after scmirroring_sink_start().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_resume()</td>
 *    <td>PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_pause().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_start()</td>
 *    <td>CONNECTED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_disconnect() </td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td></td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_get_negotiated_video_codec()</td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_get_negotiated_video_resolution()</td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_get_negotiated_video_frame_rate()</td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_get_negotiated_audio_codec()</td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_get_negotiated_audio_channel()</td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_get_negotiated_audio_sample_rate()</td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * <tr>
 *    <td>scmirroring_sink_get_negotiated_audio_bitwidth()</td>
 *    <td>CONNECTED/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after scmirroring_sink_connect().</td>
 * </tr>
 * </table></div>
 *
 * @subsection CAPI_MEDIA_SCREEN_MIRRORING_SINK_LIFE_CYCLE_ASYNCHRONOUS_OPERATIONS Asynchronous Operations
 * All functions that change the state are synchronous except scmirroring_sink_connect(), scmirroring_sink_start(), scmirroring_sink_pause(), and scmirroring_sink_resume().
 * Thus the result is passed to the application via the callback mechanism.
 *
 * @subsection CAPI_MEDIA_SCREEN_MIRRORING_SINK_LIFE_CYCLE_CALLBACK_OPERATIONS Callback(Event) Operations
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b>REGISTER</b></th>
 *        <th><b>UNREGISTER</b></th>
 *        <th><b>CALLBACK</b></th>
 *        <th><b>DESCRIPTION</b></th>
 *     </tr>
 *		 <tr>
 *        <td>scmirroring_sink_set_state_changed_cb()</td>
 *        <td>scmirroring_sink_unset_state_changed_cb()</td>
 *        <td>scmirroring_state_cb()</td>
 *        <td>This callback is called for state and error of screen mirroring.</td>
 *     </tr>
 *</table></div>
 * @section CAPI_NETWORK_WIFI_DIRECT_MANAGER_MODULE_FEATURE Related Features
 * This API is related with the following feature:\n
 * - http://tizen.org/feature/network.wifi.direct.display\n
 *
 * It is recommended to design feature related codes in your application for reliability.
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.
 * More details on featuring your application can be found from <a href="../org.tizen.mobile.native.appprogramming/html/ide_sdk_tools/feature_element.htm"><b>Feature Element</b>.</a>
 */


#endif /* __TIZEN_SCREEN_MIRRORING_DOC_H__ */
