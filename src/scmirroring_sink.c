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
#include <stdio.h>
#include <mmf/mm_wfd_sink.h>
#include <scmirroring_sink.h>
#include <scmirroring_private.h>

static scmirroring_error_e __scmirroring_sink_error_convert(const char *func, int error)
{
	int ret = SCMIRRORING_ERROR_NONE;
	const char *errorstr = NULL;

	switch (error) {
		case MM_ERROR_NONE:
			ret = SCMIRRORING_ERROR_NONE;
			errorstr = "ERROR_NONE";
			break;

		case MM_ERROR_WFD_NOT_INITIALIZED:
		case MM_ERROR_COMMON_INVALID_ATTRTYPE:
		case MM_ERROR_COMMON_INVALID_PERMISSION:
		case MM_ERROR_COMMON_OUT_OF_ARRAY:
		case MM_ERROR_COMMON_OUT_OF_RANGE:
		case MM_ERROR_COMMON_ATTR_NOT_EXIST:
			ret = SCMIRRORING_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";
			break;

		default:
			ret = SCMIRRORING_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION";
	}

	if (ret != SCMIRRORING_ERROR_NONE)
		scmirroring_error("[%s] %s (0x%08x) : core frameworks error code(0x%08x)", func, errorstr, ret, error);
	else
		scmirroring_debug("[%s] %s", func, errorstr);

	return ret;
}

static scmirroring_sink_state_e __scmirroring_sink_state_convert(MMWFDSinkStateType mm_state)
{
	scmirroring_sink_state_e state = SCMIRRORING_SINK_STATE_NONE;

	switch (mm_state) {
		case MM_WFD_SINK_STATE_NONE:
			state = SCMIRRORING_SINK_STATE_NONE;
			break;
		case MM_WFD_SINK_STATE_NULL:
			state = SCMIRRORING_SINK_STATE_NULL;
			break;
		case MM_WFD_SINK_STATE_PREPARED:
			state = SCMIRRORING_SINK_STATE_PREPARED;
			break;
		case MM_WFD_SINK_STATE_CONNECTED:
			state = SCMIRRORING_SINK_STATE_CONNECTED;
			break;
		case MM_WFD_SINK_STATE_PLAYING:
			state = SCMIRRORING_SINK_STATE_PLAYING;
			break;
		case MM_WFD_SINK_STATE_PAUSED:
			state = SCMIRRORING_SINK_STATE_PAUSED;
			break;
		case MM_WFD_SINK_STATE_DISCONNECTED:
			state = SCMIRRORING_SINK_STATE_DISCONNECTED;
			break;
		default:
			state = SCMIRRORING_SINK_STATE_NONE;
			break;
	}

	return state;
}

void __mm_scmirroring_sink_set_message_cb(int error_type, MMWFDSinkStateType state_type, void *uData)
{
	scmirroring_error_e error = __scmirroring_sink_error_convert(__func__, error_type);
	scmirroring_sink_state_e state = __scmirroring_sink_state_convert(state_type);
	scmirroring_sink_s *handle = (scmirroring_sink_s *)uData;

	/* call application callback */
	if (handle->scmirroring_sink_state_cb->state_cb)
		handle->scmirroring_sink_state_cb->state_cb(error, state, handle->scmirroring_sink_state_cb->user_data);

	return;
}

int scmirroring_sink_create(scmirroring_sink_h *scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_error_fenter();

	scmirroring_retvm_if(scmirroring_sink == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	scmirroring_sink_s *handle = (scmirroring_sink_s *)calloc(1, sizeof(scmirroring_sink_s));
	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Fail to allocate memory for handle");

	handle->mm_handle = 0;
	handle->ip = NULL;
	handle->port = NULL;
	handle->use_hdcp = TRUE;
	handle->scmirroring_sink_state_cb = NULL;

	ret = mm_wfd_sink_create(&handle->mm_handle);
	if (ret != MM_ERROR_NONE) {
		SCMIRRORING_SAFE_FREE(handle);
		scmirroring_error("Fail to Create");
		return __scmirroring_sink_error_convert(__func__, ret);
	}

	*scmirroring_sink = (scmirroring_sink_h)handle;

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_set_ip_and_port(scmirroring_sink_h scmirroring_sink, const char *ip, const char *port)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(ip == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "ip is NULL");
	scmirroring_retvm_if(port == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "port is NULL");

	scmirroring_debug("ip[%s] port[%s]", ip, port);

	handle->ip = strdup(ip);
	handle->port = strdup(port);

	scmirroring_error_fleave();

	return SCMIRRORING_ERROR_NONE;
}

int scmirroring_sink_prepare(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_prepare(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_connect(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	char server_uri[255] = {0, };

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	if (handle->ip == NULL) {
		scmirroring_error("INVALID_IP(NULL) (0x%08x)", SCMIRRORING_ERROR_INVALID_PARAMETER);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if (handle->port == NULL) {
		scmirroring_error("INVALID_PORT(NULL) (0x%08x)", SCMIRRORING_ERROR_INVALID_PARAMETER);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	memset(server_uri, 0x00, sizeof(server_uri));
	snprintf(server_uri, sizeof(server_uri), "rtsp://%s:%s/wfd1.0/streamid=0", handle->ip, handle->port);

	scmirroring_error("server_uri[%s]", server_uri);
	printf("server_uri[%s]", server_uri);

	ret = mm_wfd_sink_connect(handle->mm_handle, server_uri);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_unprepare(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_unprepare(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_destroy(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_destroy(handle->mm_handle);

	handle->mm_handle = 0;
	SCMIRRORING_SAFE_FREE(handle->ip);
	SCMIRRORING_SAFE_FREE(handle->port);
	SCMIRRORING_SAFE_FREE(handle->scmirroring_sink_state_cb);
	SCMIRRORING_SAFE_FREE(handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_start(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_start(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_disconnect(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_disconnect(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_set_state_changed_cb(scmirroring_sink_h scmirroring_sink, scmirroring_sink_state_cb callback, void *user_data)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(callback == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "callback is NULL");

	if (handle->scmirroring_sink_state_cb == NULL) {
		handle->scmirroring_sink_state_cb = (scmirroring_sink_state_cb_s *)calloc(1, sizeof(scmirroring_sink_state_cb_s));
		if (handle->scmirroring_sink_state_cb == NULL) {
			scmirroring_error("Error Set CB");
			return SCMIRRORING_ERROR_OUT_OF_MEMORY;
		}
	} else {
		memset(handle->scmirroring_sink_state_cb, 0, sizeof(scmirroring_sink_state_cb_s));
	}

	handle->scmirroring_sink_state_cb->user_data = user_data;
	handle->scmirroring_sink_state_cb->state_cb  = callback;

	ret = mm_wfd_sink_set_message_callback(handle->mm_handle, __mm_scmirroring_sink_set_message_cb, handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_unset_state_changed_cb(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_set_message_callback(handle->mm_handle, NULL, NULL);

	SCMIRRORING_SAFE_FREE(handle->scmirroring_sink_state_cb);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_set_display(scmirroring_sink_h scmirroring_sink, scmirroring_display_type_e type, void *display_surface)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(display_surface == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "display_surface is NULL");

	if ((type != SCMIRRORING_DISPLAY_TYPE_OVERLAY) && (type != SCMIRRORING_DISPLAY_TYPE_EVAS)) {
		scmirroring_error("Invalid display type [%d]", type);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	scmirroring_debug("display type(%d)", type);

	ret = mm_wfd_sink_set_attribute(handle->mm_handle, NULL, "display_surface_type", type, NULL);
	if (ret != MM_ERROR_NONE) {
		scmirroring_error("Fail to Set Display Type");
		return __scmirroring_sink_error_convert(__func__, ret);
	}

	ret = mm_wfd_sink_set_attribute(handle->mm_handle, NULL, "display_overlay", display_surface, sizeof(void *), NULL);
	if (ret != MM_ERROR_NONE) {
		scmirroring_error("Fail to Set Display Overlay");
		return __scmirroring_sink_error_convert(__func__, ret);
	}

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_set_resolution(scmirroring_sink_h scmirroring_sink, int resolution)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	if ((resolution < SCMIRRORING_RESOLUTION_1920x1080_P30) || (resolution >= SCMIRRORING_RESOLUTION_MAX)) {
		scmirroring_error("Invalid resolution : %d", resolution);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	scmirroring_debug("resolution(%d)", resolution);
	ret = mm_wfd_sink_set_resolution(handle->mm_handle, resolution);
	if (ret != MM_ERROR_NONE) {
		scmirroring_error("Fail to Set resolution");
		return __scmirroring_sink_error_convert(__func__, ret);
	}

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_pause(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_pause(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_resume(scmirroring_sink_h scmirroring_sink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");

	ret = mm_wfd_sink_resume(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_get_negotiated_video_codec(scmirroring_sink_h *scmirroring_sink, scmirroring_video_codec_e *codec)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	int mm_codec = MM_WFD_SINK_VIDEO_CODEC_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;
	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(codec == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "codec is NULL");

	*codec = SCMIRRORING_VIDEO_CODEC_NONE;

	ret = mm_wfd_sink_get_negotiated_video_codec(handle->mm_handle, &mm_codec);
	ret = __scmirroring_sink_error_convert(__func__, ret);
	if (ret != SCMIRRORING_ERROR_NONE)
		return ret;

	switch (mm_codec) {
		case MM_WFD_SINK_VIDEO_CODEC_H264:
			*codec = SCMIRRORING_VIDEO_CODEC_H264;
			break;
		default:
			*codec = SCMIRRORING_VIDEO_CODEC_NONE;
			break;
	}

	scmirroring_debug("codec: %d", *codec);
	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_get_negotiated_video_resolution(scmirroring_sink_h *scmirroring_sink, int *width, int *height)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;
	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(width == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "width is NULL");
	scmirroring_retvm_if(height == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "height is NULL");

	*width = 0;
	*height = 0;

	ret = mm_wfd_sink_get_negotiated_video_resolution(handle->mm_handle, width, height);
	ret = __scmirroring_sink_error_convert(__func__, ret);
	if (ret != SCMIRRORING_ERROR_NONE)
		return ret;

	scmirroring_debug("width: %d, height: %d", *width, *height);
	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_get_negotiated_video_frame_rate(scmirroring_sink_h *scmirroring_sink, int *frame_rate)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;
	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(frame_rate == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "frame_rate is NULL");

	*frame_rate = 0;

	ret = mm_wfd_sink_get_negotiated_video_frame_rate(handle->mm_handle, frame_rate);
	ret = __scmirroring_sink_error_convert(__func__, ret);
	if (ret != SCMIRRORING_ERROR_NONE)
		return ret;

	scmirroring_debug("frame rate: %d", *frame_rate);
	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_get_negotiated_audio_codec(scmirroring_sink_h *scmirroring_sink, scmirroring_audio_codec_e *codec)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	int mm_codec = MM_WFD_SINK_AUDIO_CODEC_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;
	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(codec == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "codec is NULL");

	*codec = SCMIRRORING_AUDIO_CODEC_NONE;

	ret = mm_wfd_sink_get_negotiated_audio_codec(handle->mm_handle, &mm_codec);
	ret = __scmirroring_sink_error_convert(__func__, ret);
	if (ret != SCMIRRORING_ERROR_NONE)
		return ret;

	switch (mm_codec) {
		case MM_WFD_SINK_AUDIO_CODEC_AAC:
			*codec = SCMIRRORING_AUDIO_CODEC_AAC;
			break;
		case MM_WFD_SINK_AUDIO_CODEC_AC3:
			*codec = SCMIRRORING_AUDIO_CODEC_AC3;
			break;
		case MM_WFD_SINK_AUDIO_CODEC_LPCM:
			*codec = SCMIRRORING_AUDIO_CODEC_LPCM;
			break;
		default:
			*codec = SCMIRRORING_AUDIO_CODEC_NONE;
			break;
	}

	scmirroring_debug("codec: %d", *codec);
	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_get_negotiated_audio_channel(scmirroring_sink_h *scmirroring_sink, int *channel)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;
	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(channel == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "channel is NULL");

	*channel = 0;

	ret = mm_wfd_sink_get_negotiated_audio_channel(handle->mm_handle, channel);
	ret = __scmirroring_sink_error_convert(__func__, ret);
	if (ret != SCMIRRORING_ERROR_NONE)
		return ret;

	scmirroring_debug("channel: %d", *channel);
	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_get_negotiated_audio_sample_rate(scmirroring_sink_h *scmirroring_sink, int *sample_rate)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;
	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(sample_rate == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "sample_rate is NULL");

	*sample_rate = 0;

	ret = mm_wfd_sink_get_negotiated_audio_sample_rate(handle->mm_handle, sample_rate);
	ret = __scmirroring_sink_error_convert(__func__, ret);
	if (ret != SCMIRRORING_ERROR_NONE)
		return ret;

	scmirroring_debug("sample rate: %d", *sample_rate);
	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_get_negotiated_audio_bitwidth(scmirroring_sink_h *scmirroring_sink, int *bitwidth)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s *)scmirroring_sink;
	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring_sink is NULL");
	scmirroring_retvm_if(bitwidth == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "bitwidth is NULL");

	*bitwidth = 0;

	ret = mm_wfd_sink_get_negotiated_audio_bitwidth(handle->mm_handle, bitwidth);
	ret = __scmirroring_sink_error_convert(__func__, ret);
	if (ret != SCMIRRORING_ERROR_NONE)
		return ret;

	scmirroring_debug("bitwidth: %d", *bitwidth);
	scmirroring_error_fleave();

	return ret;
}
