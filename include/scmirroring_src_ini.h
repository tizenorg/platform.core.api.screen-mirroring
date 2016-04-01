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

#ifndef __SCMIRRORING_SRC_INI_H__
#define __SCMIRRORING_SRC_INI_H__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_NONE 0
#define ERROR_FILE_NOT_FOUND 1

#define SCMIRRORING_SRC_INI_DEFAULT_PATH	SYSCONFDIR"/multimedia/scmirroring_src.ini"/*sysconfdir is defined at .spec file*/
#define SCMIRRORING_SRC_INI_GST_START_PARAMS_NUM	5

#define SCMIRRORING_SRC_INI() scmirroring_src_ini_get_structure()

#define SCMIRRORING_SRC_INI_MAX_STRLEN	80
#define SCMIRRORING_SRC_INI_MAX_PARAM_STRLEN 256

typedef enum __scmirroring_ini_videosink_element {
	SCMIRRORING_INI_VSINK_V4l2SINK = 0,
	SCMIRRORING_INI_VSINK_XIMAGESINK,
	SCMIRRORING_INI_VSINK_XVIMAGESINK,
	SCMIRRORING_INI_VSINK_FAKESINK,
	SCMIRRORING_INI_VSINK_EVASIMAGESINK,
	SCMIRRORING_INI_VSINK_GLIMAGESINK,
	SCMIRRORING_INI_VSINK_NUM
} SCMIRRORING_INI_VSINK_ELEMENT;

typedef enum __scmirroring_ini_videosrc_element {
	SCMIRRORING_INI_VSRC_XVIMAGESRC,
	SCMIRRORING_INI_VSRC_FILESRC,
	SCMIRRORING_INI_VSRC_CAMERASRC,
	SCMIRRORING_INI_VSRC_VIDEOTESTSRC,
	SCMIRRORING_INI_VSRC_NUM
} SCMIRRORING_INI_VSRC_ELEMENT;

typedef enum __scmirroring_ini_session_mode {
	SCMIRRORING_INI_AUDIO_VIDEO_MUXED,
	SCMIRRORING_INI_VIDEO_ONLY,
	SCMIRRORING_INI_AUDIO_ONLY,
	SCMIRRORING_INI_AUDIO_VIDEO_SAPERATE
} SCMIRRORING_INI_SESSION_MODE;


/* NOTE : MMPlayer has no initalizing API for library itself
 * so we cannot decide when those ini values to be released.
 * this is the reason of all string items are static array.
 * make it do with malloc when MMPlayerInitialize() API created
 * before that time, we should be careful with size limitation
 * of each string item.
 */

/* @ mark means the item has tested */
typedef struct __scmirroring_src_ini {
	/* general */
	SCMIRRORING_INI_VSRC_ELEMENT videosrc_element;
	gchar name_of_video_encoder[SCMIRRORING_SRC_INI_MAX_STRLEN];
	gint session_mode;
	SCMIRRORING_INI_VSINK_ELEMENT videosink_element;
	gchar name_of_video_converter[SCMIRRORING_SRC_INI_MAX_STRLEN];
	gboolean skip_rescan;
	gboolean generate_dot;
	gboolean provide_clock;
	gint mtu_size;

	gchar name_of_audio_device[SCMIRRORING_SRC_INI_MAX_STRLEN];
	gchar name_of_audio_device_property[SCMIRRORING_SRC_INI_MAX_STRLEN];
	gchar name_of_audio_encoder_aac[SCMIRRORING_SRC_INI_MAX_STRLEN];
	gchar name_of_audio_encoder_ac3[SCMIRRORING_SRC_INI_MAX_STRLEN];
	guint audio_codec;
	gint audio_latency_time;
	gint audio_buffer_time;
	gint audio_do_timestamp;
	guint64 video_reso_supported;
	guint decide_udp_bitrate[21];
	gint video_native_resolution;
	gint hdcp_enabled;
	guint8 uibc_gen_capability;

	gchar gst_param[SCMIRRORING_SRC_INI_GST_START_PARAMS_NUM][SCMIRRORING_SRC_INI_MAX_PARAM_STRLEN];
	gboolean disable_segtrap;

	gint dump_ts;

} scmirroring_src_ini_t;

/* default values if each values are not specified in inifile */
/* general */
#define DEFAULT_SKIP_RESCAN				TRUE
#define DEFAULT_GENERATE_DOT				FALSE
#define DEFAULT_PROVIDE_CLOCK				TRUE
#define DEFAULT_VIDEOSINK				SCMIRRORING_INI_VSINK_XVIMAGESINK
#define DEFAULT_VIDEOSRC				SCMIRRORING_INI_VSRC_XVIMAGESRC
#define DEFAULT_VIDEOENC				"omxh264enc"
#define DEFAULT_VIDEO_BITRATE 				3072000 /* bps */
#define DEFAULT_MTU_SIZE        1400 /* bytes */
#define DEFAULT_SESSION_MODE				0
#define DEFAULT_GST_PARAM				""
#define DEFAULT_DISABLE_SEGTRAP				TRUE
#define DEFAULT_VIDEO_CONVERTER				""
/* hw accel */
#define DEFAULT_AUDIO_DEVICE_NAME "alsa_output.0.analog-stereo.monitor"
#define DEFAULT_AUDIO_DEVICE_PROPERTY_NAME "props,media.role=loopback-mirroring"
#define DEFAULT_AUDIOENC_AAC				"avenc_aac"
#define DEFAULT_AUDIOENC_AC3				"avenc_ac3"
#define DEFAULT_AUDIO_CODEC	2
#define DEFAULT_AUDIO_LATENCY_TIME	10000
#define DEFAULT_AUDIO_BUFFER_TIME	200000
#define DEFAULT_AUDIO_DO_TIMESTAMP	0
#define DEFAULT_VIDEO_RESOLUTION_SUPPORTED	0x00000001
#define DEFAULT_NATIVE_VIDEO_RESOLUTION  0
#define DEFAULT_HDCP_ENABLED 1
#define DEFAULT_UIBC_GEN_CAPABILITY 0
#define DEFAULT_DUMP_TS				0

/* NOTE : following content should be same with above default values */
/* FIXIT : need smarter way to generate default ini file. */
/* FIXIT : finally, it should be an external file */
#define SCMIRRORING_SRC_DEFAULT_INI \
" \
[general] \n\
\n\
; set default video source element\n\
; 0: xvimagesrc, 1: filesrc, 2: camerasrc, 3: videotestsrc\n\
videosrc element = 0 \n\
\n\
; sending video only mirroring mode\n\
; 0: audio-video-muxed sending, 1:video-only, 2:audio-only, 3:audio-video-saperate\n\
session_mode = 0 \n\
disable segtrap = yes ; same effect with --gst-disable-segtrap \n\
\n\
; set default video sink when video is rendered on the SCMIRRORING source as well\n\
; 0:v4l2sink, 1:ximagesink, 2:xvimagesink, 3:fakesink 4:evasimagesink 5:glimagesink\n\
videosink element = 2 \n\
\n\
mtu_size value = 1400 \n\
\n\
video converter element = \n\
\n\
; if yes. gstreamer will not update registry \n\
skip rescan = yes \n\
\n\
; parameters for initializing gstreamer \n\
gstparam1 = \n\
gstparam2 = \n\
gstparam3 = \n\
gstparam4 = \n\
gstparam5 = \n\
\n\
; generating dot file representing pipeline state \n\
generate dot = no \n\
\n\
\n\
\n\
"

int
scmirroring_src_ini_load(void);

scmirroring_src_ini_t*
scmirroring_src_ini_get_structure(void);


#ifdef __cplusplus
}
#endif

#endif
