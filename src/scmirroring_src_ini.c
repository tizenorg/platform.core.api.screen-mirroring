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

#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <iniparser.h>

#include <scmirroring_src_ini.h>
#include <scmirroring_private.h>

/* global variables here */
static scmirroring_src_ini_t g_scmirroring_src_ini;
static void __scmirroring_ini_check_ini_status(void);
/* static void __get_string_list(gchar** out_list, gchar* str); */

/* macro */
#define SCMIRRORING_INI_GET_STRING(x_item, x_ini, x_default) \
do { \
	gchar* str = NULL; \
	gint length = 0; \
	str = iniparser_getstring(dict, x_ini, (char *)x_default); \
	if (str) { \
		length = strlen(str); \
		if ((length > 1) && (length < SCMIRRORING_SRC_INI_MAX_STRLEN)) \
			strncpy(x_item, str, SCMIRRORING_SRC_INI_MAX_STRLEN - 1); \
		else \
			strncpy(x_item, x_default, SCMIRRORING_SRC_INI_MAX_STRLEN - 1); \
	} \
	else { \
		strncpy(x_item, x_default, SCMIRRORING_SRC_INI_MAX_STRLEN - 1); \
	} \
} while (0);


int
scmirroring_src_ini_load(void)
{
	static gboolean loaded = FALSE;
	dictionary *dict = NULL;

	if (loaded)
		return ERROR_NONE;

	dict = NULL;

	/* disabling ini parsing for launching */
	/* get scmirroring ini status because system will be crashed
	* if ini file is corrupted.
	*/
	/* FIXIT : the api actually deleting illregular ini. but the function name said it's just checking. */
	__scmirroring_ini_check_ini_status();

	/* first, try to load existing ini file */
	dict = iniparser_load(SCMIRRORING_SRC_INI_DEFAULT_PATH);

	/* get ini values */
	memset(&g_scmirroring_src_ini, 0, sizeof(scmirroring_src_ini_t));

	if (dict) { /* if dict is available */
		/* general */
		gchar temp[SCMIRRORING_SRC_INI_MAX_STRLEN];
		g_scmirroring_src_ini.videosrc_element = iniparser_getint(dict, "general:videosrc element", DEFAULT_VIDEOSRC);
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.name_of_video_encoder, "general:video encoder element", DEFAULT_VIDEOENC);
		g_scmirroring_src_ini.session_mode = iniparser_getint(dict, "general:session_mode", DEFAULT_SESSION_MODE);
		g_scmirroring_src_ini.videosink_element = iniparser_getint(dict, "general:videosink element", DEFAULT_VIDEOSINK);
		g_scmirroring_src_ini.disable_segtrap = iniparser_getboolean(dict, "general:disable segtrap", DEFAULT_DISABLE_SEGTRAP);
		g_scmirroring_src_ini.skip_rescan = iniparser_getboolean(dict, "general:skip rescan", DEFAULT_SKIP_RESCAN);
		g_scmirroring_src_ini.mtu_size = iniparser_getint(dict, "general:mtu_size value", DEFAULT_MTU_SIZE);
		g_scmirroring_src_ini.generate_dot = iniparser_getboolean(dict, "general:generate dot", DEFAULT_GENERATE_DOT);
		g_scmirroring_src_ini.provide_clock = iniparser_getboolean(dict, "general:provide clock", DEFAULT_PROVIDE_CLOCK);
		
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.name_of_audio_encoder_aac, "general:audio encoder aac name", DEFAULT_AUDIOENC_AAC);
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.name_of_audio_encoder_ac3, "general:audio encoder ac3 name", DEFAULT_AUDIOENC_AC3);
		g_scmirroring_src_ini.audio_codec = iniparser_getint(dict, "general:audio codec", DEFAULT_AUDIO_CODEC);
#ifndef ENABLE_QC_SPECIFIC
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.name_of_audio_device, "general:exynosaudio device name", DEFAULT_AUDIO_EXYNOS_DEVICE_NAME);
		g_scmirroring_src_ini.audio_buffer_time = iniparser_getint(dict, "general:exynosaudio buffer_time", DEFAULT_AUDIO_EXYNOS_BUFFER_TIME);
#else
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.name_of_audio_device, "general:qcmsmaudio device name", DEFAULT_AUDIO_QC_DEVICE_NAME);
		g_scmirroring_src_ini.audio_buffer_time = iniparser_getint(dict, "general:qcmsmaudio buffer_time", DEFAULT_AUDIO_QC_BUFFER_TIME);
#endif
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.name_of_audio_device_property, "general:audio properties name", DEFAULT_AUDIO_DEVICE_PROPERTY_NAME);
		g_scmirroring_src_ini.audio_latency_time = iniparser_getint(dict, "general:audio latency_time", DEFAULT_AUDIO_LATENCY_TIME);
		g_scmirroring_src_ini.audio_do_timestamp = iniparser_getint(dict, "general:audio do_timestamp", DEFAULT_AUDIO_DO_TIMESTAMP);
		SCMIRRORING_INI_GET_STRING(temp, "general:video resolution_supported", "");
		if (strlen(temp) > 0) g_scmirroring_src_ini.video_reso_supported = strtoull(temp, NULL, 16);
		else g_scmirroring_src_ini.video_reso_supported = DEFAULT_VIDEO_RESOLUTION_SUPPORTED;
		g_scmirroring_src_ini.decide_udp_bitrate[0] = iniparser_getint(dict, "general:INIT_UDP_resolution_set_1", DEFAULT_VIDEO_BITRATE_INIT_1);
		g_scmirroring_src_ini.decide_udp_bitrate[1] = iniparser_getint(dict, "general:MIN_UDP_resolution_set_1", DEFAULT_VIDEO_BITRATE_MIN_1);
		g_scmirroring_src_ini.decide_udp_bitrate[2] = iniparser_getint(dict, "general:MAX_UDP_resolution_set_1", DEFAULT_VIDEO_BITRATE_MAX_1);
		g_scmirroring_src_ini.decide_udp_bitrate[3] = iniparser_getint(dict, "general:INIT_UDP_resolution_set_2", DEFAULT_VIDEO_BITRATE_INIT_2);
		g_scmirroring_src_ini.decide_udp_bitrate[4] = iniparser_getint(dict, "general:MIN_UDP_resolution_set_2", DEFAULT_VIDEO_BITRATE_MIN_2);
		g_scmirroring_src_ini.decide_udp_bitrate[5] = iniparser_getint(dict, "general:MAX_UDP_resolution_set_2", DEFAULT_VIDEO_BITRATE_MAX_2);
		g_scmirroring_src_ini.decide_udp_bitrate[6] = iniparser_getint(dict, "general:INIT_UDP_resolution_set_3", DEFAULT_VIDEO_BITRATE_INIT_3);
		g_scmirroring_src_ini.decide_udp_bitrate[7] = iniparser_getint(dict, "general:MIN_UDP_resolution_set_3", DEFAULT_VIDEO_BITRATE_MIN_3);
		g_scmirroring_src_ini.decide_udp_bitrate[8] = iniparser_getint(dict, "general:MAX_UDP_resolution_set_3", DEFAULT_VIDEO_BITRATE_MAX_3);
		g_scmirroring_src_ini.decide_udp_bitrate[9] = iniparser_getint(dict, "general:INIT_UDP_resolution_set_4", DEFAULT_VIDEO_BITRATE_INIT_4);
		g_scmirroring_src_ini.decide_udp_bitrate[10] = iniparser_getint(dict, "general:MIN_UDP_resolution_set_4", DEFAULT_VIDEO_BITRATE_MIN_4);
		g_scmirroring_src_ini.decide_udp_bitrate[11] = iniparser_getint(dict, "general:MAX_UDP_resolution_set_4", DEFAULT_VIDEO_BITRATE_MAX_4);
		g_scmirroring_src_ini.decide_udp_bitrate[12] = iniparser_getint(dict, "general:INIT_UDP_resolution_set_5", DEFAULT_VIDEO_BITRATE_INIT_5);
		g_scmirroring_src_ini.decide_udp_bitrate[13] = iniparser_getint(dict, "general:MIN_UDP_resolution_set_5", DEFAULT_VIDEO_BITRATE_MIN_5);
		g_scmirroring_src_ini.decide_udp_bitrate[14] = iniparser_getint(dict, "general:MAX_UDP_resolution_set_5", DEFAULT_VIDEO_BITRATE_MAX_5);
		g_scmirroring_src_ini.decide_udp_bitrate[15] = iniparser_getint(dict, "general:INIT_UDP_resolution_set_6", DEFAULT_VIDEO_BITRATE_INIT_6);
		g_scmirroring_src_ini.decide_udp_bitrate[16] = iniparser_getint(dict, "general:MIN_UDP_resolution_set_6", DEFAULT_VIDEO_BITRATE_MIN_6);
		g_scmirroring_src_ini.decide_udp_bitrate[17] = iniparser_getint(dict, "general:MAX_UDP_resolution_set_6", DEFAULT_VIDEO_BITRATE_MAX_6);
		g_scmirroring_src_ini.decide_udp_bitrate[18] = iniparser_getint(dict, "general:INIT_UDP_resolution_set_7", DEFAULT_VIDEO_BITRATE_INIT_7);
		g_scmirroring_src_ini.decide_udp_bitrate[19] = iniparser_getint(dict, "general:MIN_UDP_resolution_set_7", DEFAULT_VIDEO_BITRATE_MIN_7);
		g_scmirroring_src_ini.decide_udp_bitrate[20] = iniparser_getint(dict, "general:MAX_UDP_resolution_set_7", DEFAULT_VIDEO_BITRATE_MAX_7);
		g_scmirroring_src_ini.video_native_resolution = iniparser_getint(dict, "general:video native_resolution", DEFAULT_NATIVE_VIDEO_RESOLUTION);
		g_scmirroring_src_ini.hdcp_enabled = iniparser_getint(dict, "general:encryption HDCP_enabled", DEFAULT_HDCP_ENABLED);
		g_scmirroring_src_ini.uibc_gen_capability = iniparser_getint(dict, "general:uibc UIBC_GEN_capability", DEFAULT_UIBC_GEN_CAPABILITY);
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.name_of_video_converter, "general:video converter element", DEFAULT_VIDEO_CONVERTER);

		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.gst_param[0], "general:gstparam1", DEFAULT_GST_PARAM);
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.gst_param[1], "general:gstparam2", DEFAULT_GST_PARAM);
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.gst_param[2], "general:gstparam3", DEFAULT_GST_PARAM);
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.gst_param[3], "general:gstparam4", DEFAULT_GST_PARAM);
		SCMIRRORING_INI_GET_STRING(g_scmirroring_src_ini.gst_param[4], "general:gstparam5", DEFAULT_GST_PARAM);

		g_scmirroring_src_ini.dump_ts = iniparser_getint(dict, "general:dump ts", DEFAULT_DUMP_TS);

	} else { /* if dict is not available just fill the structure with default value */
		scmirroring_debug("failed to load ini. using hardcoded default\n");

		/* general */
		g_scmirroring_src_ini.videosrc_element = DEFAULT_VIDEOSRC;
		strncpy(g_scmirroring_src_ini.name_of_video_encoder, DEFAULT_VIDEOENC, SCMIRRORING_SRC_INI_MAX_STRLEN - 1);
		g_scmirroring_src_ini.session_mode = DEFAULT_SESSION_MODE;
		g_scmirroring_src_ini.videosink_element = DEFAULT_VIDEOSINK;
		g_scmirroring_src_ini.disable_segtrap = DEFAULT_DISABLE_SEGTRAP;
		g_scmirroring_src_ini.skip_rescan = DEFAULT_SKIP_RESCAN;
		g_scmirroring_src_ini.mtu_size = DEFAULT_MTU_SIZE;
		g_scmirroring_src_ini.generate_dot = DEFAULT_GENERATE_DOT;
		g_scmirroring_src_ini.provide_clock = DEFAULT_PROVIDE_CLOCK;

		strncpy(g_scmirroring_src_ini.name_of_audio_encoder_aac, DEFAULT_AUDIOENC_AAC, SCMIRRORING_SRC_INI_MAX_STRLEN - 1);
		strncpy(g_scmirroring_src_ini.name_of_audio_encoder_ac3, DEFAULT_AUDIOENC_AC3, SCMIRRORING_SRC_INI_MAX_STRLEN - 1);
		g_scmirroring_src_ini.audio_codec = DEFAULT_AUDIO_CODEC;
#ifndef ENABLE_QC_SPECIFIC
		strncpy(g_scmirroring_src_ini.name_of_audio_device, DEFAULT_AUDIO_EXYNOS_DEVICE_NAME, SCMIRRORING_SRC_INI_MAX_STRLEN - 1);
		g_scmirroring_src_ini.audio_buffer_time = DEFAULT_AUDIO_EXYNOS_BUFFER_TIME;
#else
		strncpy(g_scmirroring_src_ini.name_of_audio_device, DEFAULT_AUDIO_QC_DEVICE_NAME, SCMIRRORING_SRC_INI_MAX_STRLEN - 1);
		g_scmirroring_src_ini.audio_buffer_time = DEFAULT_AUDIO_QC_BUFFER_TIME;
#endif
		strncpy(g_scmirroring_src_ini.name_of_audio_device_property, DEFAULT_AUDIO_DEVICE_PROPERTY_NAME, SCMIRRORING_SRC_INI_MAX_STRLEN - 1);
		g_scmirroring_src_ini.audio_latency_time = DEFAULT_AUDIO_LATENCY_TIME;
		g_scmirroring_src_ini.audio_do_timestamp = DEFAULT_AUDIO_DO_TIMESTAMP;
		g_scmirroring_src_ini.video_reso_supported = DEFAULT_VIDEO_RESOLUTION_SUPPORTED;
		g_scmirroring_src_ini.decide_udp_bitrate[0] = DEFAULT_VIDEO_BITRATE_INIT_1;
		g_scmirroring_src_ini.decide_udp_bitrate[1] = DEFAULT_VIDEO_BITRATE_MIN_1;
		g_scmirroring_src_ini.decide_udp_bitrate[2] = DEFAULT_VIDEO_BITRATE_MAX_1;
		g_scmirroring_src_ini.decide_udp_bitrate[3] = DEFAULT_VIDEO_BITRATE_INIT_2;
		g_scmirroring_src_ini.decide_udp_bitrate[4] = DEFAULT_VIDEO_BITRATE_MIN_2;
		g_scmirroring_src_ini.decide_udp_bitrate[5] = DEFAULT_VIDEO_BITRATE_MAX_2;
		g_scmirroring_src_ini.decide_udp_bitrate[6] = DEFAULT_VIDEO_BITRATE_INIT_3;
		g_scmirroring_src_ini.decide_udp_bitrate[7] = DEFAULT_VIDEO_BITRATE_MIN_3;
		g_scmirroring_src_ini.decide_udp_bitrate[8] = DEFAULT_VIDEO_BITRATE_MAX_3;
		g_scmirroring_src_ini.decide_udp_bitrate[9] = DEFAULT_VIDEO_BITRATE_INIT_4;
		g_scmirroring_src_ini.decide_udp_bitrate[10] = DEFAULT_VIDEO_BITRATE_MIN_4; 
		g_scmirroring_src_ini.decide_udp_bitrate[11] = DEFAULT_VIDEO_BITRATE_MAX_4; 
		g_scmirroring_src_ini.decide_udp_bitrate[12] = DEFAULT_VIDEO_BITRATE_INIT_5; 
		g_scmirroring_src_ini.decide_udp_bitrate[13] = DEFAULT_VIDEO_BITRATE_MIN_5; 
		g_scmirroring_src_ini.decide_udp_bitrate[14] = DEFAULT_VIDEO_BITRATE_MAX_5; 
		g_scmirroring_src_ini.decide_udp_bitrate[15] = DEFAULT_VIDEO_BITRATE_INIT_6; 
		g_scmirroring_src_ini.decide_udp_bitrate[16] = DEFAULT_VIDEO_BITRATE_MIN_6; 
		g_scmirroring_src_ini.decide_udp_bitrate[17] = DEFAULT_VIDEO_BITRATE_MAX_6; 
		g_scmirroring_src_ini.decide_udp_bitrate[18] = DEFAULT_VIDEO_BITRATE_INIT_7; 
		g_scmirroring_src_ini.decide_udp_bitrate[19] = DEFAULT_VIDEO_BITRATE_MIN_7; 
		g_scmirroring_src_ini.decide_udp_bitrate[20] = DEFAULT_VIDEO_BITRATE_MAX_7; 
		g_scmirroring_src_ini.video_native_resolution = DEFAULT_NATIVE_VIDEO_RESOLUTION;
		g_scmirroring_src_ini.hdcp_enabled = DEFAULT_HDCP_ENABLED;
		g_scmirroring_src_ini.uibc_gen_capability = DEFAULT_UIBC_GEN_CAPABILITY;

		strncpy(g_scmirroring_src_ini.name_of_video_converter, DEFAULT_VIDEO_CONVERTER, SCMIRRORING_SRC_INI_MAX_STRLEN - 1);

		strncpy(g_scmirroring_src_ini.gst_param[0], DEFAULT_GST_PARAM, SCMIRRORING_SRC_INI_MAX_PARAM_STRLEN - 1);
		strncpy(g_scmirroring_src_ini.gst_param[1], DEFAULT_GST_PARAM, SCMIRRORING_SRC_INI_MAX_PARAM_STRLEN - 1);
		strncpy(g_scmirroring_src_ini.gst_param[2], DEFAULT_GST_PARAM, SCMIRRORING_SRC_INI_MAX_PARAM_STRLEN - 1);
		strncpy(g_scmirroring_src_ini.gst_param[3], DEFAULT_GST_PARAM, SCMIRRORING_SRC_INI_MAX_PARAM_STRLEN - 1);
		strncpy(g_scmirroring_src_ini.gst_param[4], DEFAULT_GST_PARAM, SCMIRRORING_SRC_INI_MAX_PARAM_STRLEN - 1);
		g_scmirroring_src_ini.dump_ts = DEFAULT_DUMP_TS;
	}

	/* free dict as we got our own structure */
	if(dict)
		iniparser_freedict(dict);

	loaded = TRUE;

	/* The simulator uses a separate ini file. */
	/*__scmirroring_ini_force_setting(); */


	/* dump structure */
	scmirroring_debug("scmirroring settings-----------------------------------\n");

	/* general */
	scmirroring_debug("videosrc element : %d\n", g_scmirroring_src_ini.videosrc_element);
	scmirroring_debug("session mode in mirroring : %d\n", g_scmirroring_src_ini.session_mode);
	scmirroring_debug("disable_segtrap : %d\n", g_scmirroring_src_ini.disable_segtrap);
	scmirroring_debug("skip rescan : %d\n", g_scmirroring_src_ini.skip_rescan);
	scmirroring_debug("videosink element(0:v4l2sink, 1:ximagesink, 2:xvimagesink, 3:fakesink) : %d\n", g_scmirroring_src_ini.videosink_element);
	scmirroring_debug("mtu_size : %d\n", g_scmirroring_src_ini.mtu_size);
	scmirroring_debug("generate_dot : %d\n", g_scmirroring_src_ini.generate_dot);
	scmirroring_debug("provide_clock : %d\n", g_scmirroring_src_ini.provide_clock);
	scmirroring_debug("name_of_video_converter : %s\n", g_scmirroring_src_ini.name_of_video_converter);

	scmirroring_debug("audio device name : %s\n", g_scmirroring_src_ini.name_of_audio_device);
	scmirroring_debug("audio device property name : %s\n", g_scmirroring_src_ini.name_of_audio_device_property);
	scmirroring_debug("audio latency_time : %d\n", g_scmirroring_src_ini.audio_latency_time);
	scmirroring_debug("audio buffer_time : %d\n", g_scmirroring_src_ini.audio_buffer_time);
	scmirroring_debug("audio do_timestamp : %d\n", g_scmirroring_src_ini.audio_do_timestamp);
	scmirroring_debug("video resolution_supported : %d\n", g_scmirroring_src_ini.video_reso_supported);
	scmirroring_debug("video native_resolution : %d\n", g_scmirroring_src_ini.video_native_resolution);
	scmirroring_debug("encryption HDCP_enabled : %d\n", g_scmirroring_src_ini.hdcp_enabled);
	scmirroring_debug("uibc UIBC_GEN_capability : %d\n", g_scmirroring_src_ini.uibc_gen_capability);

	scmirroring_debug("gst_param1 : %s\n", g_scmirroring_src_ini.gst_param[0]);
	scmirroring_debug("gst_param2 : %s\n", g_scmirroring_src_ini.gst_param[1]);
	scmirroring_debug("gst_param3 : %s\n", g_scmirroring_src_ini.gst_param[2]);
	scmirroring_debug("gst_param4 : %s\n", g_scmirroring_src_ini.gst_param[3]);
	scmirroring_debug("gst_param5 : %s\n", g_scmirroring_src_ini.gst_param[4]);

	scmirroring_debug("dump ts : %d\n", g_scmirroring_src_ini.dump_ts);

	scmirroring_debug("---------------------------------------------------\n");

	return ERROR_NONE;
}


static
void __scmirroring_ini_check_ini_status(void)
{
	struct stat ini_buff;

	if (g_stat(SCMIRRORING_SRC_INI_DEFAULT_PATH, &ini_buff) < 0) {
		scmirroring_debug("failed to get scmirroring ini status\n");
	} else {
		if (ini_buff.st_size < 5) {
			scmirroring_debug("scmirroring.ini file size=%d, Corrupted! So, Removed\n", (int)ini_buff.st_size);
			g_remove(SCMIRRORING_SRC_INI_DEFAULT_PATH);
		}
	}
}


scmirroring_src_ini_t *
scmirroring_src_ini_get_structure(void)
{
	return &g_scmirroring_src_ini;
}

#if 0
static
void __get_string_list(gchar **out_list, gchar *str)
{
	gchar **list = NULL;
	gchar **walk = NULL;
	gint i = 0;
	gchar *strtmp = NULL;

	if (!str)
		return;

	if (strlen(str) < 1)
		return;

	strtmp = g_strdup(str);

	/* trimming. it works inplace */
	g_strstrip(strtmp);

	/* split */
	list = g_strsplit(strtmp, ", ", 10);

	g_return_if_fail(list != NULL);

	g_strfreev(list);
	if (strtmp)
		g_free(strtmp);
}
#endif
