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
#include <scmirroring_type.h>
#include <dlog.h>
#include <mm_types.h>

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

#define scmirroring_debug(fmt, arg...) do { \
			LOGD(""fmt"", ##arg);     \
		} while (0)

#define scmirroring_info(fmt, arg...) do { \
			LOGI(""fmt"", ##arg);     \
		} while (0)

#define scmirroring_error(fmt, arg...) do { \
			LOGE(""fmt"", ##arg);     \
		} while (0)

#define scmirroring_debug_fenter() do { \
			LOGD("<Enter>");     \
		} while (0)

#define scmirroring_debug_fleave() do { \
			LOGD("<Leave>");     \
		} while (0)

#define scmirroring_error_fenter() do { \
			LOGE("NO-ERROR : <Enter>");     \
		} while (0)

#define scmirroring_error_fleave() do { \
			LOGE("NO-ERROR : <Leave>");     \
		} while (0)

#define scmirroring_sucure_info(fmt, arg...) do { \
			SECURE_LOGI(""fmt"", ##arg);     \
		} while (0)

#define SCMIRRORING_SAFE_FREE(src)      { if(src) {free(src); src = NULL;}}
#define STRING_VALID(str)		((str != NULL && strlen(str) > 0) ? TRUE : FALSE)

typedef struct
{
	int error_code;
	int src_state;
	void *user_data;
	scmirroring_state_cb state_cb;
}scmirroring_state_cb_s;

typedef struct
{
	char *ip;
	char *port;
	bool use_hdcp;
	int resolution;
	/* To communicate with miracast server */
	int sock;
	int source_id;
	GIOChannel *channel;
	char *sock_path;
	int connect_mode;
	scmirroring_state_cb_s *scmirroring_state_cb;
} scmirroring_src_s;

typedef struct
{
	MMHandleType mm_handle;
	char *ip;
	char *port;
	bool use_hdcp;
	scmirroring_state_cb_s *scmirroring_state_cb;
} scmirroring_sink_s;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_SCMIRRORING_PRIVATE_H__ */
