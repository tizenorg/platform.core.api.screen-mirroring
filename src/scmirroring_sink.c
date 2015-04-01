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

static scmirroring_error_e __scmirroring_sink_error_convert(const char* func, int error)
{
	int ret = SCMIRRORING_ERROR_NONE;
	const char *errorstr = NULL;

	switch(error)
	{
		case MM_ERROR_NONE:
			ret = SCMIRRORING_ERROR_NONE;
			errorstr = "ERROR_NONE";
			break;

		case MM_ERROR_WFD_NOT_INITIALIZED :
		case MM_ERROR_COMMON_INVALID_ATTRTYPE :
		case MM_ERROR_COMMON_INVALID_PERMISSION :
		case MM_ERROR_COMMON_OUT_OF_ARRAY :
		case MM_ERROR_COMMON_OUT_OF_RANGE :
		case MM_ERROR_COMMON_ATTR_NOT_EXIST :
			ret = SCMIRRORING_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";
			break;

		default:
			ret = SCMIRRORING_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION";
	}

	if(ret != SCMIRRORING_ERROR_NONE)
		scmirroring_error("[%s] %s (0x%08x) : core frameworks error code(0x%08x)", func, errorstr, ret, error);
	else
		scmirroring_debug("[%s] %s", func, errorstr);

	return ret;
}

static scmirroring_state_e __scmirroring_sink_state_convert(MMWfdSinkStateType mm_state)
{
	scmirroring_state_e state = SCMIRRORING_STATE_NULL;

	switch( mm_state )
	{
		case MM_WFD_SINK_STATE_NULL:
			state = SCMIRRORING_STATE_NULL;
			break;
		case MM_WFD_SINK_STATE_READY:
			state = SCMIRRORING_STATE_READY;
			break;
		case MM_WFD_SINK_STATE_PLAYING:
			state = SCMIRRORING_STATE_PLAYING;
			break;
		case MM_WFD_SINK_STATE_PAUSED:
			state = SCMIRRORING_STATE_PAUSED;
			break;
		case MM_WFD_SINK_STATE_TEARDOWN:
			state = SCMIRRORING_STATE_TEARDOWN;
			break;
		case MM_WFD_SINK_STATE_NONE:
			state = SCMIRRORING_STATE_NULL;
			break;
		default:
			state = SCMIRRORING_STATE_NULL;
			break;
	}

	return state;
}

void __mm_scmirroring_sink_set_message_cb(MMWfdSinkStateType type, void *uData)
{
	int error_value = SCMIRRORING_ERROR_NONE;
	scmirroring_state_e state = __scmirroring_sink_state_convert(type);
	scmirroring_sink_s *handle = (scmirroring_sink_s*)uData;

	/* call application callback */
	if (handle->scmirroring_state_cb->state_cb)
		handle->scmirroring_state_cb->state_cb((scmirroring_error_e)error_value, state, handle->scmirroring_state_cb->user_data);

	return;
}

int scmirroring_sink_create(scmirroring_sink_h *scmirroring_sink)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_error_fenter();

	scmirroring_retvm_if(scmirroring_sink == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	scmirroring_sink_s *handle = (scmirroring_sink_s*)calloc(1, sizeof(scmirroring_sink_s));
	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	handle->mm_handle = 0;
	handle->ip= NULL;
	handle->port = NULL;
	handle->use_hdcp = TRUE;

	ret = mm_wfd_sink_create(&handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		SCMIRRORING_SAFE_FREE(handle);
		scmirroring_error("Fail to Create");
		return __scmirroring_sink_error_convert(__func__,ret);
	}

	*scmirroring_sink = (scmirroring_sink_h)handle;

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_set_ip_and_port(scmirroring_sink_h scmirroring_sink, const char *ip, const char *port)
{
	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");
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
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mm_wfd_sink_realize(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_connect(scmirroring_sink_h scmirroring_sink)
{
	int ret = SCMIRRORING_ERROR_NONE;
	char server_uri[255] = {0,};

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if(handle->ip == NULL)
	{
		scmirroring_error("INVALID_IP (0x%08x)",SCMIRRORING_ERROR_INVALID_PARAMETER);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if(handle->port == NULL)
	{
		scmirroring_error("INVALID_PORT (0x%08x)",SCMIRRORING_ERROR_INVALID_PARAMETER);
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
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mm_wfd_sink_unrealize(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_destroy(scmirroring_sink_h scmirroring_sink)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mm_wfd_sink_destroy(handle->mm_handle);

	SCMIRRORING_SAFE_FREE(handle->ip);
	SCMIRRORING_SAFE_FREE(handle->port);
	SCMIRRORING_SAFE_FREE(handle->scmirroring_state_cb);
	SCMIRRORING_SAFE_FREE(handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_start(scmirroring_sink_h scmirroring_sink)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mm_wfd_sink_start(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_stop(scmirroring_sink_h scmirroring_sink)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mm_wfd_sink_stop(handle->mm_handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_set_state_changed_cb(scmirroring_sink_h scmirroring_sink, scmirroring_state_cb callback, void *user_data)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	handle->scmirroring_state_cb = (scmirroring_state_cb_s*)calloc(1, sizeof(scmirroring_state_cb_s));

	if(handle->scmirroring_state_cb == NULL)
	{
		scmirroring_error("Error Set CB");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	handle->scmirroring_state_cb->user_data = user_data;
	handle->scmirroring_state_cb->state_cb  = callback;

	ret = mm_wfd_sink_set_message_callback(handle->mm_handle, __mm_scmirroring_sink_set_message_cb, handle);

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}

int scmirroring_sink_set_display(scmirroring_sink_h scmirroring_sink, scmirroring_display_type_e type, void * display_surface)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_sink_s *handle = (scmirroring_sink_s*)scmirroring_sink;

	scmirroring_error_fenter();

	scmirroring_retvm_if(handle == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");
	scmirroring_retvm_if(display_surface == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "display_surface is NULL");

	if((type != SCMIRRORING_DISPLAY_TYPE_OVERLAY) && (type != SCMIRRORING_DISPLAY_TYPE_EVAS))
	{
		scmirroring_error("Invalid display type [%d]", type);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	scmirroring_debug("display type (%d)", type);

	ret = mm_wfd_sink_set_attribute(handle->mm_handle, NULL, "display_surface_type", type, NULL);
	if(ret != MM_ERROR_NONE)
	{
		scmirroring_error("Fail to Set Display Type");
		return __scmirroring_sink_error_convert(__func__,ret);
	}

	ret = mm_wfd_sink_set_attribute(handle->mm_handle, NULL, "display_overlay", display_surface, NULL);
	if(ret != MM_ERROR_NONE)
	{
		scmirroring_error("Fail to Set Display Overlay");
		return __scmirroring_sink_error_convert(__func__,ret);
	}

	ret = __scmirroring_sink_error_convert(__func__, ret);

	scmirroring_error_fleave();

	return ret;
}
