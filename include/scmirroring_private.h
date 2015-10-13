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

#ifndef __TIZEN_MEDIA_SCMIRRORING_PRIVATE_H__
#define __TIZEN_MEDIA_SCMIRRORING_PRIVATE_H__

#include <stdlib.h>
#include <dlog.h>
#include <mm_types.h>
#include <system_info.h>
#include <scmirroring_internal.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CAPI_MEDIA_SCMIRRORING"

#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#define scmirroring_debug(fmt, arg...) do { \
		LOGD(FONT_COLOR_RESET""fmt"", ##arg);     \
	} while (0)

#define scmirroring_info(fmt, arg...) do { \
		LOGI(FONT_COLOR_GREEN""fmt""FONT_COLOR_RESET, ##arg);     \
	} while (0)

#define scmirroring_error(fmt, arg...) do { \
		LOGE(FONT_COLOR_RED""fmt""FONT_COLOR_RESET, ##arg);     \
	} while (0)

#define scmirroring_debug_fenter() do { \
		LOGD(FONT_COLOR_RESET"<Enter>");     \
	} while (0)

#define scmirroring_debug_fleave() do { \
		LOGD(FONT_COLOR_RESET"<Leave>");     \
	} while (0)

#define scmirroring_error_fenter() do { \
		LOGE(FONT_COLOR_RESET"NO-ERROR : <Enter>");     \
	} while (0)

#define scmirroring_error_fleave() do { \
		LOGE(FONT_COLOR_RESET"NO-ERROR : <Leave>");     \
	} while (0)

#define scmirroring_retvm_if(expr, val, fmt, arg...) do { \
		if(expr) { \
			LOGE(FONT_COLOR_RED""fmt""FONT_COLOR_RESET, ##arg);     \
			return (val); \
		} \
	} while (0)

#define scmirroring_sucure_info(fmt, arg...) do { \
		SECURE_LOGI(FONT_COLOR_GREEN""fmt""FONT_COLOR_RESET, ##arg);     \
	} while (0)

#define SCMIRRORING_SAFE_FREE(src)      { if(src) {free(src); src = NULL;}}
#define STRING_VALID(str)		((str != NULL && strlen(str) > 0) ? TRUE : FALSE)

#define SCMIRRORING_STATE_CMD_START			"START"
#define SCMIRRORING_STATE_CMD_PAUSE			"PAUSE"
#define SCMIRRORING_STATE_CMD_RESUME		"RESUME"
#define SCMIRRORING_STATE_CMD_STOP			"STOP"
#define SCMIRRORING_STATE_CMD_DESTROY		"DESTROY"

typedef struct {
	int error_code;
	int src_state;
	void *user_data;
	scmirroring_state_cb state_cb;
}scmirroring_state_cb_s;

typedef struct {
	int error_code;
	int sink_state;
	void *user_data;
	scmirroring_sink_state_cb state_cb;
}scmirroring_sink_state_cb_s;

typedef struct {
	char *ip;
	char *port;
	char *server_name;
	bool use_hdcp;
	int resolution;
	/* To communicate with miracast server */
	int connected;
	int sock;
	int source_id;
	GIOChannel *channel;
	char *sock_path;
	int connect_mode;
	int current_state;
	scmirroring_state_cb_s *scmirroring_state_cb;
} scmirroring_src_s;

typedef struct {
	MMHandleType mm_handle;
	char *ip;
	char *port;
	bool use_hdcp;
	scmirroring_sink_state_cb_s *scmirroring_sink_state_cb;
} scmirroring_sink_s;

#define WIFIDIRECT_DISPLAY_FEATURE "http://tizen.org/feature/network.wifi.direct.display"

#define CHECK_FEATURE_SUPPORTED(feature_name)\
	do {\
		bool feature_supported = FALSE;\
		if(!system_info_get_platform_bool(feature_name, &feature_supported)){\
			if(feature_supported == FALSE){\
				scmirroring_error("%s feature is disabled", feature_name);\
				return SCMIRRORING_ERROR_NOT_SUPPORTED;\
			}\
		} else {\
			scmirroring_error("Error - Feature getting from System Info");\
			return SCMIRRORING_ERROR_UNKNOWN;\
		}\
	} while (0);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_SCMIRRORING_PRIVATE_H__ */
