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

#include <gio/gio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <netinet/tcp.h>

#include <scmirroring_src.h>
#include <scmirroring_private.h>

#define MAX_MSG_LEN 128
#define TIMEOUT_SEC 2
#define CONNECTED_TO_SERVER 1
#define NOT_CONNECTED_TO_SERVER 0

static gboolean __scmirroring_src_callback_call(gpointer data)
{
	scmirroring_src_s *scmirroring = (scmirroring_src_s *) data;

	if (scmirroring == NULL) {
		scmirroring_error("SCMIRRORING is NULL");
		return FALSE;
	}

	scmirroring_state_cb_s *cb_info = scmirroring->scmirroring_state_cb;

	if ((cb_info != NULL) && (cb_info->state_cb != NULL)) {
		scmirroring_debug("Calling user callback(error: %d, status: %d)", cb_info->error_code, cb_info->src_state);
		cb_info->state_cb(cb_info->error_code, cb_info->src_state, cb_info->user_data);
	}

	if (cb_info != NULL && cb_info->src_state == SCMIRRORING_STATE_NULL) {
		SCMIRRORING_SAFE_FREE(scmirroring->ip);
		SCMIRRORING_SAFE_FREE(scmirroring->port);
		SCMIRRORING_SAFE_FREE(scmirroring->scmirroring_state_cb);

		g_io_channel_shutdown(scmirroring->channel, FALSE, NULL);
		g_io_channel_unref(scmirroring->channel);

		SCMIRRORING_SAFE_FREE(scmirroring->sock_path);
		SCMIRRORING_SAFE_FREE(scmirroring);
	}

	return FALSE;
}

static int __scmirroring_src_send_cmd_to_server(scmirroring_src_s *scmirroring, const char *cmd)
{
	int ret = SCMIRRORING_ERROR_NONE;
	char *_cmd = NULL;

	scmirroring_retvm_if(scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "scmirroring is NULL");
	scmirroring_retvm_if(cmd == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "cmd is NULL");

	_cmd = g_strdup(cmd);
	_cmd[strlen(_cmd)] = '\0';

	if (write(scmirroring->sock, _cmd, strlen(_cmd) + 1) != (signed int) strlen(_cmd) + 1) {
		char buf[255] = {0, };
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("sendto failed [%s]", buf);
		ret = SCMIRRORING_ERROR_INVALID_OPERATION;
	} else {
		scmirroring_debug("Sent message [%s] successfully", _cmd);
	}

	SCMIRRORING_SAFE_FREE(_cmd);

	return ret;
}

static int __miracast_server_launch(scmirroring_src_s *scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;
	GDBusProxy *proxy = NULL;
	GDBusConnection *conn = NULL;
	GError *error = NULL;

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	scmirroring_debug("-----------socket connect failed it means server is not yet started------------");
	scmirroring_debug("going to start miracast server");

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (error) {
		scmirroring_error("Failed to get dbus connection: %s", error->message);
		g_error_free(error);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	gchar *name = NULL;
	gchar *if_name = NULL;
	gchar *obj_path = NULL;

	if (scmirroring->server_name) {
		name = g_strdup_printf("org.tizen.%s.server", scmirroring->server_name);
		if_name = g_strdup_printf("org.tizen.%s.server", scmirroring->server_name);
		obj_path = g_strdup_printf("/org/tizen/%s/server", scmirroring->server_name);
	} else {
		name = g_strdup("org.tizen.scmirroring.server");
		if_name = g_strdup("org.tizen.scmirroring.server");
		obj_path = g_strdup("/org/tizen/scmirroring/server");
	}

	scmirroring_debug("Server Name : %s", name);

	proxy = g_dbus_proxy_new_sync(conn,
								G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
								NULL,
								name,
								obj_path,
								if_name,
								NULL,
								&error);
	g_free(name);
	g_free(if_name);
	g_free(obj_path);
	g_object_unref(conn);
	if (error) {
		scmirroring_error("g_dbus_proxy_new_sync failed : %s", error->message);
		g_error_free(error);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	g_dbus_proxy_call_sync(proxy, "launch_method", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
	if (error) {
		scmirroring_error("g_dbus_proxy_call_sync failed : %s", error->message);
		g_error_free(error);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	scmirroring_debug("Miracast server is launched successfully");

	return ret;
}

/*
 * Application - miracast server message
 * 1. Response Message from miracast server
 * [Error:Status]
 * - OK:LISTENING
 * - OK:CONNECTED
 * - OK:STOP
 * - OK:DESTROY
 *
 * 2. Request Message from application to miracast server
 * - START [ip addr:port]
 * - SET [sth]
*/

static int __scmirroring_src_get_error(gchar *str)
{
	if (g_strrstr(str, "OK")) {
		return SCMIRRORING_ERROR_NONE;
	} else if (g_strrstr(str, "FAIL")) {
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	return SCMIRRORING_ERROR_NONE;
}

static int __scmirroring_src_get_status(gchar *str)
{
	if (g_strrstr(str, "LISTENING")) {
		return SCMIRRORING_STATE_CONNECTION_WAIT;
	} else if (g_strrstr(str, "CONNECTED")) {
		return SCMIRRORING_STATE_CONNECTED;
	} else if (g_strrstr(str, "PLAYING")) {
		return SCMIRRORING_STATE_PLAYING;
	} else if (g_strrstr(str, "SET")) {
		return SCMIRRORING_STATE_READY;
	} else if (g_strrstr(str, SCMIRRORING_STATE_CMD_PAUSE)) {
		return SCMIRRORING_STATE_PAUSED;
	} else if (g_strrstr(str, SCMIRRORING_STATE_CMD_RESUME)) {
		return SCMIRRORING_STATE_PLAYING;
	} else if (g_strrstr(str, SCMIRRORING_STATE_CMD_STOP)) {
		return SCMIRRORING_STATE_TEARDOWN;
	} else if (g_strrstr(str, SCMIRRORING_STATE_CMD_DESTROY)) {
		return SCMIRRORING_STATE_NULL;
	}

	return SCMIRRORING_STATE_NONE;
}

static void __scmirroring_src_set_callback_info(scmirroring_src_s *scmirroring, int error_code, int state)
{
	scmirroring_state_cb_s *cb_info = scmirroring->scmirroring_state_cb;
	if (cb_info) {
		cb_info->error_code = error_code;
		cb_info->src_state = state;

		GSource *src_user_cb = NULL;
		src_user_cb = g_idle_source_new();
		g_source_set_callback(src_user_cb, (GSourceFunc)__scmirroring_src_callback_call, scmirroring, NULL);
		g_source_attach(src_user_cb, g_main_context_get_thread_default());
	} else {
		scmirroring_error("There is no callback");
	}

	return;
}

static void __scmirroring_src_interpret(scmirroring_src_s *scmirroring, char *buf)
{
	scmirroring_debug("Received : %s", buf);

	int error_code = SCMIRRORING_ERROR_INVALID_OPERATION;
	int src_state = SCMIRRORING_STATE_NONE;
	gchar **response;
	response = g_strsplit(buf, ":", 0);

	scmirroring_debug("error: %s, status: %s", response[0], response[1]);

	error_code = __scmirroring_src_get_error(response[0]);
	src_state = __scmirroring_src_get_status(response[1]);;
	g_strfreev(response);

	if (scmirroring->current_state != src_state) {
		scmirroring->current_state = src_state;
		__scmirroring_src_set_callback_info(scmirroring, error_code, src_state);
	} else {
		scmirroring_debug("Current state is already %d", src_state);
	}

	return;
}

gboolean __scmirroring_src_read_cb(GIOChannel *src, GIOCondition condition, gpointer data)
{
	char buf[MAX_MSG_LEN + 1];
	gsize read;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)data;

	if (condition & G_IO_IN) {
		g_io_channel_read_chars(_scmirroring->channel, buf, MAX_MSG_LEN, &read, NULL);
		if (read == 0) {
			scmirroring_error("Read 0 bytes");
			return FALSE;
		} else {
			scmirroring_debug("Read %d bytes", read);
		}

		gsize i = 0;
		gsize idx = 0;

		/* Handling multiple response like "CMD1\0CMD2\0CMD3\0" */
		for (i = 0; i < read; i++) {
			gchar *str = NULL;
			if (buf[i] == '\0') {
				str = buf + idx;
				idx = i + 1;
			} else {
				continue;
			}
			scmirroring_debug("Handling %s", str);
			__scmirroring_src_interpret(_scmirroring, str);
			if (idx >= read) break;
		}
	} else if (condition & G_IO_ERR) {
		scmirroring_error("got G_IO_ERR");
		return FALSE;
	} else if (condition & G_IO_HUP) {
		scmirroring_error("got G_IO_HUP");
		return FALSE;
	}

	return TRUE;
}

static int __scmirroring_src_send_set_cm(scmirroring_src_h scmirroring)
{
	/* Set connection mode to miracast server */
	char *cmd = NULL;
	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	cmd = g_strdup_printf("SET CM %d", _scmirroring->connect_mode);
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if (ret != SCMIRRORING_ERROR_NONE) {
		SCMIRRORING_SAFE_FREE(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	SCMIRRORING_SAFE_FREE(cmd);

	return ret;
}

static int __scmirroring_src_send_set_ip(scmirroring_src_h scmirroring)
{
	/* Set IP and Port to server */
	char *cmd = NULL;
	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	cmd = g_strdup_printf("SET IP %s:%s", _scmirroring->ip, _scmirroring->port);
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if (ret != SCMIRRORING_ERROR_NONE) {
		SCMIRRORING_SAFE_FREE(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	SCMIRRORING_SAFE_FREE(cmd);

	return ret;
}

static int __scmirroring_src_send_set_reso(scmirroring_src_h scmirroring)
{
	/* Set resolution to miracast server */
	char *cmd = NULL;
	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	cmd = g_strdup_printf("SET RESO %d", _scmirroring->resolution);
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if (ret != SCMIRRORING_ERROR_NONE) {
		SCMIRRORING_SAFE_FREE(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	SCMIRRORING_SAFE_FREE(cmd);

	return ret;
}

static int __scmirroring_src_send_set_multisink(scmirroring_src_h scmirroring)
{
	/* Set resolution to miracast server */
	char *cmd = NULL;
	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	cmd = g_strdup_printf("SET MULTISINK %d", _scmirroring->resolution);
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if (ret != SCMIRRORING_ERROR_NONE) {
		SCMIRRORING_SAFE_FREE(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	SCMIRRORING_SAFE_FREE(cmd);

	return ret;
}

static int __scmirroring_src_send_set_direct_streaming(scmirroring_src_h scmirroring)
{
	/* Set resolution to miracast server */
	char *cmd = NULL;
	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	cmd = g_strdup_printf("SET STREAMING %d %s", _scmirroring->direct_streaming, _scmirroring->filesrc);
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if (ret != SCMIRRORING_ERROR_NONE) {
		SCMIRRORING_SAFE_FREE(cmd);
		scmirroring_error("Failed to enable direct streaming [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	SCMIRRORING_SAFE_FREE(cmd);

	return ret;
}

int scmirroring_src_create(scmirroring_src_h *scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = NULL;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	_scmirroring = (scmirroring_src_s *)calloc(1, sizeof(scmirroring_src_s));
	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

	_scmirroring->ip = NULL;
	_scmirroring->port = NULL;
	_scmirroring->filesrc = NULL;
	_scmirroring->connected = NOT_CONNECTED_TO_SERVER;
	_scmirroring->use_hdcp = TRUE;
	_scmirroring->resolution = 0;
	_scmirroring->connect_mode = SCMIRRORING_CONNECTION_WIFI_DIRECT;
	_scmirroring->scmirroring_state_cb = NULL;
	_scmirroring->sock = -1;
	_scmirroring->channel = NULL;
	_scmirroring->sock_path = NULL;
	_scmirroring->current_state = SCMIRRORING_STATE_NONE;
	_scmirroring->server_name = g_strdup("scmirroring");
	_scmirroring->multisink = SCMIRRORING_MULTISINK_DISABLE;

	*scmirroring = (scmirroring_src_h)_scmirroring;

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_connection_mode(scmirroring_src_h scmirroring, scmirroring_connection_mode_e connect_mode)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if ((connect_mode < SCMIRRORING_CONNECTION_WIFI_DIRECT) || (connect_mode >= SCMIRRORING_CONNECTION_MAX)) {
		scmirroring_error("INVALID Connection mode : %d", connect_mode);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	_scmirroring->connect_mode = connect_mode;

	if (_scmirroring->connected) {
		ret = __scmirroring_src_send_set_cm(_scmirroring);
	}

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_state_changed_cb(scmirroring_src_h scmirroring, scmirroring_state_cb callback, void *user_data)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");
	scmirroring_retvm_if(callback == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "callback is NULL");

	if (_scmirroring->scmirroring_state_cb == NULL) {
		_scmirroring->scmirroring_state_cb = (scmirroring_state_cb_s *)calloc(1, sizeof(scmirroring_state_cb_s));
		scmirroring_retvm_if(_scmirroring->scmirroring_state_cb == NULL, SCMIRRORING_ERROR_OUT_OF_MEMORY, "Error Set CB");
	} else {
		memset(_scmirroring->scmirroring_state_cb, 0, sizeof(scmirroring_state_cb_s));
	}

	_scmirroring->scmirroring_state_cb->user_data = user_data;
	_scmirroring->scmirroring_state_cb->state_cb = callback;

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_unset_state_changed_cb(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if (_scmirroring->scmirroring_state_cb != NULL) {
		_scmirroring->scmirroring_state_cb->user_data = NULL;
		_scmirroring->scmirroring_state_cb->state_cb = NULL;
	}

	SCMIRRORING_SAFE_FREE(_scmirroring->scmirroring_state_cb);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_ip_and_port(scmirroring_src_h scmirroring, const char *ip, const char *port)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");
	scmirroring_retvm_if(!STRING_VALID(ip), SCMIRRORING_ERROR_INVALID_PARAMETER, "INVALID IP");
	scmirroring_retvm_if(!STRING_VALID(port), SCMIRRORING_ERROR_INVALID_PARAMETER, "INVALID PORT");

	_scmirroring->ip = strdup(ip);
	_scmirroring->port = strdup(port);

	if ((_scmirroring->ip == NULL) || (_scmirroring->port == NULL)) {
		scmirroring_error("OUT_OF_MEMORY");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	if (_scmirroring->connected) {
		ret = __scmirroring_src_send_set_ip(_scmirroring);
	}

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_resolution(scmirroring_src_h scmirroring, scmirroring_resolution_e resolution)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if ((resolution < SCMIRRORING_RESOLUTION_1920x1080_P30) || (resolution >= SCMIRRORING_RESOLUTION_MAX)) {
		scmirroring_error("INVALID resolution : %d", resolution);
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	_scmirroring->resolution = resolution;

	if (_scmirroring->connected) {
		ret = __scmirroring_src_send_set_reso(_scmirroring);
	}

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_server_name(scmirroring_src_h scmirroring, const char *name)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if (_scmirroring->server_name) g_free(_scmirroring->server_name);
	_scmirroring->server_name = g_strdup(name);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_multisink_ability(scmirroring_src_h scmirroring, scmirroring_multisink_e multisink)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	_scmirroring->multisink = multisink;

	if (_scmirroring->connected) {
		ret = __scmirroring_src_send_set_multisink(_scmirroring);
	}

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_connect(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	int retry = 0;
	struct sockaddr_un serv_addr;
	int sock = -1;
	GIOChannel *channel = NULL;
	struct timeval tv_timeout = { TIMEOUT_SEC, 0 };
	char buf[255] = {0, };

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");
	scmirroring_retvm_if(_scmirroring->connected == CONNECTED_TO_SERVER, SCMIRRORING_ERROR_INVALID_OPERATION, "INVALID OPERATION, already connected to server.");

	/*Create TCP Socket*/
	if ((sock = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("socket failed: %s", buf);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout)) == -1) {
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("setsockopt failed: %s", buf);
		close(sock);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	channel = g_io_channel_unix_new(sock);
	if (channel == NULL) {
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("g_io_channel_unix_new failed: %s", buf);
	}

	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

	_scmirroring->sock = sock;
	_scmirroring->channel = channel;
	_scmirroring->sock_path = g_strdup("/tmp/.miracast_ipc_rtspserver");

	/* Connecting to the miracast server */
	memset(&serv_addr, 0, sizeof(struct sockaddr_un));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, _scmirroring->sock_path, sizeof(serv_addr.sun_path));
	serv_addr.sun_path[sizeof(serv_addr.sun_path) - 1] = '\0';

try:
	scmirroring_debug("Trying to connect to the miracast server");
	if (connect(_scmirroring->sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		/* Once failed to connect, try to launch miracast server */
		if (retry == 0) {
			ret = __miracast_server_launch(_scmirroring);
			if (ret != SCMIRRORING_ERROR_NONE) {
				scmirroring_error("__miracast_server_launch error : %d", ret);
				return ret;
			}
			retry++;
			usleep(10000);
			goto try;
		} else {
			scmirroring_debug("Trying to connect failed");
			if (retry < 5) {
				scmirroring_debug("Trying to connect again..");
				retry++;
				usleep(10000);
				goto try;
			}

			strerror_r(errno, buf, sizeof(buf));
			scmirroring_error("Connect error : %s", buf);
			close(_scmirroring->sock);
			_scmirroring->sock = -1;
			return SCMIRRORING_ERROR_INVALID_OPERATION;
		}
	} else {
		scmirroring_debug("Connected successfully");
	}

	/* Create new channel to watch tcp socket */
	GSource *source = NULL;
	source = g_io_create_watch(_scmirroring->channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
	int source_id = -1;

	/* Set callback to be called when socket is readable */
	g_source_set_callback(source, (GSourceFunc)__scmirroring_src_read_cb, _scmirroring, NULL);
	source_id = g_source_attach(source, g_main_context_get_thread_default());

	_scmirroring->source_id = source_id;
	_scmirroring->connected = CONNECTED_TO_SERVER;
	_scmirroring->current_state = SCMIRRORING_STATE_READY;

	__scmirroring_src_set_callback_info(_scmirroring, SCMIRRORING_ERROR_NONE, SCMIRRORING_STATE_READY);

	if ((_scmirroring->ip != NULL) || (_scmirroring->port != NULL)) {
		ret = __scmirroring_src_send_set_ip(_scmirroring);
		ret = __scmirroring_src_send_set_cm(_scmirroring);
		ret = __scmirroring_src_send_set_reso(_scmirroring);
	}

	if (_scmirroring->multisink == SCMIRRORING_MULTISINK_ENABLE) {
		ret = __scmirroring_src_send_set_multisink(_scmirroring);
	}

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_disconnect(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if (_scmirroring->channel != NULL) {
		g_io_channel_shutdown(_scmirroring->channel, FALSE, NULL);
		g_io_channel_unref(_scmirroring->channel);
		_scmirroring->channel = NULL;
	}

	if (_scmirroring->sock != -1) {
		close(_scmirroring->sock);
		_scmirroring->sock = -1;
	}

	SCMIRRORING_SAFE_FREE(_scmirroring->sock_path);
	_scmirroring->connected = NOT_CONNECTED_TO_SERVER;

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_start(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, SCMIRRORING_STATE_CMD_START);
	if (ret != SCMIRRORING_ERROR_NONE)
		scmirroring_error("Failed to start [%d]", ret);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_pause(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, SCMIRRORING_STATE_CMD_PAUSE);
	if (ret != SCMIRRORING_ERROR_NONE)
		scmirroring_error("Failed to pause [%d]", ret);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_resume(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, SCMIRRORING_STATE_CMD_RESUME);
	if (ret != SCMIRRORING_ERROR_NONE)
		scmirroring_error("Failed to resume [%d]", ret);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_direct_streaming(scmirroring_src_h scmirroring_src,
		scmirroring_direct_streaming_e enable, const char* uri_srcname)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
  int len = 0;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring_src;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");
	scmirroring_retvm_if(!STRING_VALID(uri_srcname), SCMIRRORING_ERROR_INVALID_PARAMETER, "INVALID URI_SRCNAME");
	_scmirroring->direct_streaming = enable;

	len = strlen (uri_srcname);
	if (_scmirroring->filesrc != NULL) {
		g_free (_scmirroring->filesrc);
		_scmirroring->filesrc = NULL;
	}
	_scmirroring->filesrc = g_strndup(uri_srcname, len);

	if ((_scmirroring->filesrc == NULL)) {
		scmirroring_error("OUT_OF_MEMORY");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	ret = __scmirroring_src_send_set_direct_streaming(_scmirroring);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_stop(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, SCMIRRORING_STATE_CMD_STOP);
	if (ret != SCMIRRORING_ERROR_NONE)
		scmirroring_error("Failed to be stop [%d]", ret);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_destroy(scmirroring_src_h scmirroring)
{
	CHECK_FEATURE_SUPPORTED(WIFIDIRECT_DISPLAY_FEATURE);

	int ret = SCMIRRORING_ERROR_NONE;
	scmirroring_src_s *_scmirroring = (scmirroring_src_s *)scmirroring;

	scmirroring_debug_fenter();

	scmirroring_retvm_if(_scmirroring == NULL, SCMIRRORING_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if (_scmirroring->connected == CONNECTED_TO_SERVER) {
		ret = __scmirroring_src_send_cmd_to_server(_scmirroring, SCMIRRORING_STATE_CMD_DESTROY);
		if (ret != SCMIRRORING_ERROR_NONE)
			scmirroring_error("Failed to destroy [%d]", ret);
	} else {
		SCMIRRORING_SAFE_FREE(_scmirroring->ip);
		SCMIRRORING_SAFE_FREE(_scmirroring->port);
		SCMIRRORING_SAFE_FREE(_scmirroring->filesrc);
		SCMIRRORING_SAFE_FREE(_scmirroring->scmirroring_state_cb);
		SCMIRRORING_SAFE_FREE(_scmirroring);
	}

	scmirroring_debug_fleave();

	return ret;
}
