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

#include <scmirroring_src.h>
#include <scmirroring_private.h>

#define MAX_MSG_LEN 128
#define TIMEOUT_SEC 2

static gboolean __scmirroring_src_callback_call(gpointer data)
{
	scmirroring_src_s *scmirroring = (scmirroring_src_s *) data;

	if (scmirroring == NULL) {
		scmirroring_error ("SCMIRRORING is NULL");
		return FALSE;
	}

	scmirroring_state_cb_s *cb_info = scmirroring->scmirroring_state_cb;

	if((cb_info != NULL) && (cb_info->state_cb != NULL)) {
		scmirroring_debug("Calling user callback (error: %d, status: %d)", cb_info->error_code, cb_info->src_state);
		cb_info->state_cb(cb_info->error_code, cb_info->src_state, cb_info->user_data);
	}

	if (cb_info->src_state == SCMIRRORING_STATE_NULL) {
		SCMIRRORING_SAFE_FREE(scmirroring->ip);
		SCMIRRORING_SAFE_FREE(scmirroring->port);
		SCMIRRORING_SAFE_FREE(scmirroring->scmirroring_state_cb);

		g_io_channel_shutdown(scmirroring->channel,  FALSE, NULL);
		g_io_channel_unref(scmirroring->channel);

		SCMIRRORING_SAFE_FREE(scmirroring->sock_path);
		SCMIRRORING_SAFE_FREE(scmirroring);
	}

	return FALSE;
}

int __scmirroring_src_send_cmd_to_server(scmirroring_src_s *scmirroring, char *cmd)
{
	int ret = SCMIRRORING_ERROR_NONE;
	char *_cmd = NULL;

	if (scmirroring == NULL) {
		scmirroring_error("OUT_OF_MEMORY");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	_cmd = g_strdup(cmd);

	if (write(scmirroring->sock, _cmd, strlen(_cmd)) != (signed int) strlen(_cmd)) {
		scmirroring_error("sendto failed [%s]", strerror(errno));
		ret = SCMIRRORING_ERROR_INVALID_OPERATION;
	} else {
		scmirroring_debug("Sent message [%s] successfully", _cmd);
	}

	g_free(_cmd);
	return ret;
}

int __miracast_server_launch(scmirroring_src_s *scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif

	GError *error = NULL;

	scmirroring_debug("-----------  socket connect failed it means server is not yet started ------------\n");
	scmirroring_debug("going to start miracast server \n");

    GDBusProxy *proxy = NULL;
    GDBusConnection *conn = NULL;

    conn = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, &error);
    if (error) {
      scmirroring_error("Failed to get dbus connection: %s", error->message);
      return -1;
    }    

    proxy = g_dbus_proxy_new_sync (conn,
                                            G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                            NULL,
                                            "org.tizen.scmirroring.server",
                                            "/org/tizen/scmirroring/server",
                                            "org.tizen.scmirroring.server",
                                            NULL,
                                            &error);
    if (error) {
        scmirroring_error("g_dbus_proxy_new_sync failed : %s\n", error->message);
        return -1;
    }

    g_dbus_proxy_call_sync (proxy, "launch_method", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
    if (error) {
        scmirroring_error("g_dbus_proxy_call_sync failed : %s\n", error->message);
        return -1;
    }

    scmirroring_debug("scmirroring_proxy_initialize trying for connect\n");


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

int __scmirroring_src_get_error(gchar *str)
{
	if (g_strrstr(str, "OK")) {
		return SCMIRRORING_ERROR_NONE;
	} else if (g_strrstr(str, "FAIL")) {
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	return SCMIRRORING_ERROR_NONE;
}

int __scmirroring_src_get_status(gchar *str)
{
	if (g_strrstr(str, "LISTENING")) {
		return SCMIRRORING_STATE_CONNECTION_WAIT;
	} else if (g_strrstr(str, "CONNECTED")) {
		return SCMIRRORING_STATE_CONNECTED;
	} else if (g_strrstr(str, "STOP")) {
		return SCMIRRORING_STATE_TEARDOWN;
	} else if (g_strrstr(str, "DESTROY")) {
		return SCMIRRORING_STATE_NULL;
	}

	return SCMIRRORING_STATE_NONE;
}

void __scmirroring_src_set_callback_info (scmirroring_src_s *scmirroring, int error_code, int state)
{
	scmirroring_state_cb_s *cb_info = scmirroring->scmirroring_state_cb;
	if (cb_info) {
		cb_info->error_code = error_code;
		cb_info->src_state = state;

		GSource *src_user_cb = NULL;
		src_user_cb = g_idle_source_new ();
		g_source_set_callback(src_user_cb, (GSourceFunc)__scmirroring_src_callback_call, scmirroring, NULL);
		g_source_attach(src_user_cb, g_main_context_get_thread_default());
	} else {
		scmirroring_error("There is no callback");
	}

	return;
}

void __scmirroring_src_interpret(scmirroring_src_s *scmirroring, char *buf)
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

	__scmirroring_src_set_callback_info (scmirroring, error_code, src_state);

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
		}
		buf[read] = '\0';
		g_strstrip(buf);
		__scmirroring_src_interpret(_scmirroring, buf);
	} else if (condition & G_IO_ERR) {
		scmirroring_error("got G_IO_ERR");
		return FALSE;
	} else if (condition & G_IO_HUP) {
		scmirroring_error("got G_IO_HUP");
		return FALSE;
	}

	return TRUE;
}

int scmirroring_src_create(scmirroring_src_h *scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_debug_fenter();

	if(scmirroring == NULL)
	{
		scmirroring_error("INVALID_PARAMETER");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)calloc(1,sizeof(scmirroring_src_s));
	if(_scmirroring == NULL)
	{
		scmirroring_error("OUT_OF_MEMORY");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	_scmirroring->ip= NULL;
	_scmirroring->port = NULL;
	_scmirroring->use_hdcp = TRUE;
	_scmirroring->resolution = 0;
	_scmirroring->connect_mode = SCMIRRORING_CONNECTION_WIFI_DIRECT;

	/*Create TCP Socket*/
	int sock;
	struct timeval tv_timeout = { TIMEOUT_SEC, 0 };

	if ((sock = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
		scmirroring_error("socket failed: %s", strerror(errno));
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout)) == -1) {
		scmirroring_error("setsockopt failed: %s", strerror(errno));
		close(sock);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	GIOChannel *channel = NULL;
	channel = g_io_channel_unix_new(sock);
	if (channel == NULL) {
		scmirroring_error("g_io_channel_unix_new failed: %s", strerror(errno));
	}

	g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, NULL);

	_scmirroring->sock = sock;
	_scmirroring->channel = channel;
	_scmirroring->sock_path = g_strdup("/tmp/.miracast_ipc_rtspserver");

	*scmirroring = (scmirroring_src_h)_scmirroring;

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_connection_mode(scmirroring_src_h scmirroring, scmirroring_connection_mode_e connect_mode)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if((connect_mode < SCMIRRORING_CONNECTION_WIFI_DIRECT) ||(connect_mode > SCMIRRORING_CONNECTION_MAX))
	{
		scmirroring_error("INVALID Connection mode");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	_scmirroring->connect_mode = connect_mode;

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_state_changed_cb(scmirroring_src_h scmirroring, scmirroring_state_cb callback, void *user_data)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if(callback != NULL)
	{
		_scmirroring->scmirroring_state_cb = (scmirroring_state_cb_s*)calloc(1, sizeof(scmirroring_state_cb_s));
		if(!_scmirroring->scmirroring_state_cb) {
			scmirroring_error("Error Set CB");
			return SCMIRRORING_ERROR_OUT_OF_MEMORY;
		}
		_scmirroring->scmirroring_state_cb->user_data = user_data;
		_scmirroring->scmirroring_state_cb->state_cb = callback;
	}

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_set_ip_and_port(scmirroring_src_h scmirroring, const char *ip, const char *port)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if(!STRING_VALID(ip))
	{
		scmirroring_error("INVALID IP");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if(!STRING_VALID(port))
	{
		scmirroring_error("INVALID PORT");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	_scmirroring->ip = strdup(ip);
	_scmirroring->port = strdup(port);

	if((_scmirroring->ip == NULL) || (_scmirroring->port == NULL))
	{
		scmirroring_error("OUT_OF_MEMORY");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	scmirroring_debug("");

	return ret;
}

int scmirroring_src_set_resolution(scmirroring_src_h scmirroring, scmirroring_resolution_e resolution)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if((resolution < SCMIRRORING_RESOLUTION_1920x1080_P30) ||(resolution > SCMIRRORING_RESOLUTION_MAX))
	{
		scmirroring_error("INVALID resolution");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	_scmirroring->resolution = resolution;

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_connect(scmirroring_src_h scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	if((_scmirroring->ip == NULL) ||(_scmirroring->port == NULL))
	{
		scmirroring_error("INVALID IP and Port");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	/* Connecting to the miracast server */
	int retry = 0;
	struct sockaddr_un serv_addr;

	memset(&serv_addr, 0, sizeof(struct sockaddr_un));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, _scmirroring->sock_path);

try:
	scmirroring_debug("Trying to connect to the miracast server\n");
	if (connect(_scmirroring->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
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
			scmirroring_debug("Trying to connect failed\n");
			if (retry < 5) {
				scmirroring_debug("Trying to connect again..\n");
				retry++;
				usleep(10000);
				goto try;
			}

			scmirroring_error("Connect error : %s", strerror(errno));
			close(_scmirroring->sock);
			_scmirroring->sock = 0;
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

	__scmirroring_src_set_callback_info (_scmirroring, SCMIRRORING_ERROR_NONE, SCMIRRORING_STATE_READY);
	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_disconnect(scmirroring_src_h scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	g_io_channel_shutdown (_scmirroring->channel, FALSE, NULL);
	g_io_channel_unref(_scmirroring->channel);
	close (_scmirroring->sock);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_start(scmirroring_src_h scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	/* Set IP and Port to server and to be ready for miracast server */
	char *cmd = NULL;
	cmd = g_strdup_printf("START %s:%s", _scmirroring->ip, _scmirroring->port);
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_free(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	g_free(cmd);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_pause(scmirroring_src_h scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	char *cmd = NULL;
	cmd = g_strdup_printf("PAUSE");
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_free(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	g_free(cmd);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_resume(scmirroring_src_h scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	char *cmd = NULL;
	cmd = g_strdup_printf("RESUME");
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_free(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	g_free(cmd);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_stop(scmirroring_src_h scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	char *cmd = NULL;
	cmd = g_strdup_printf("STOP");
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_free(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	g_free(cmd);

	scmirroring_debug_fleave();

	return ret;
}

int scmirroring_src_destroy(scmirroring_src_h scmirroring)
{
	int ret = SCMIRRORING_ERROR_NONE;

	scmirroring_src_s *_scmirroring = (scmirroring_src_s*)scmirroring;

	scmirroring_debug_fenter();

	if(_scmirroring == NULL)
	{
		scmirroring_error("INVALID Handle");
		return SCMIRRORING_ERROR_INVALID_PARAMETER;
	}

	char *cmd = NULL;
	cmd = g_strdup_printf("DESTROY");
	ret = __scmirroring_src_send_cmd_to_server(_scmirroring, cmd);
	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_free(cmd);
		scmirroring_error("Failed to be ready [%d]", ret);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}
	g_free(cmd);

	scmirroring_debug_fleave();

	return ret;
}
