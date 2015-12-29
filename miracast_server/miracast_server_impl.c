/*
 * miracast-server
 *
 * Copyright (c) 2011 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: JongHyuk Choi <jhchoi.choi@samsung.com>, ByungWook Jang <bw.jang@samsung.com>,
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
 *
 */

#include <glib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <gio/gio.h>

#include "miracast_server.h"
#include "scmirroring_private.h"
#include "scmirroring_src_ini.h"

#include <gst/rtsp-server/rtsp-server-wfd.h>
#include <gst/rtsp-server/rtsp-media-factory-wfd.h>

#define MAX_CLIENT_CNT 1
#define MAX_MSG_LEN 128
#define MEDIA_IPC_PATH "/tmp/.miracast_ipc_rtspserver"

#define PROC_DBUS_OBJECT "/org/tizen/scmirroring/server"
#define PROC_DBUS_INTERFACE "org.tizen.scmirroring.server"
#define PROC_DBUS_STATUS_CHANGE_SIGNAL "miracast_wfd_source_status_changed"

#define MIRACAST_WFD_SOURCE_ON 1
#define MIRACAST_WFD_SOURCE_OFF 0

#define TEST_MOUNT_POINT  "/wfd1.0/streamid=0"
#define WFD_REQUIREMENT "org.wfa.wfd1.0"


static const GDBusMethodInfo scmirroring_server_method_info_method = {
	-1,
	(char *)"launch_method",
	NULL,
	NULL,
	NULL
};
static const GDBusMethodInfo *const scmirroring_server_method_info_pointers[] = {&scmirroring_server_method_info_method, NULL};

static const GDBusInterfaceInfo scmirroring_server_interface_info = {
	-1,
	(char *)PROC_DBUS_INTERFACE,
	(GDBusMethodInfo **) &scmirroring_server_method_info_pointers,
	(GDBusSignalInfo **) NULL,
	(GDBusPropertyInfo **) NULL,
	NULL,
};

static gint g_server_status = MIRACAST_WFD_SOURCE_OFF;

static GDBusNodeInfo *introspection_data = NULL;

/* Introspection data for the service we are exporting */
static const gchar introspection_xml[] =
	"<node>"
	"  <interface name='"PROC_DBUS_INTERFACE"'>"
	"    <method name='launch_method'>"
	"    </method>"
	"    <method name='get_miracast_wfd_source_status'>"
	"      <arg type='i' name='status' direction='out'/>"
	"    </method>"
	"  </interface>"
	"</node>";


static void
handle_method_call(GDBusConnection       *connection,
						const gchar           *sender,
						const gchar           *object_path,
						const gchar           *interface_name,
						const gchar           *method_name,
						GVariant              *parameters,
						GDBusMethodInvocation *invocation,
						gpointer               user_data)
{
	scmirroring_debug("handle_method_call is called\n");

	if (method_name != NULL) {
		scmirroring_debug("method_name is %s\n", method_name);

		if (g_strcmp0(method_name, "launch_method") == 0) {
			g_dbus_method_invocation_return_value(invocation, NULL);
		} else if (g_strcmp0(method_name, "get_miracast_wfd_source_status") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", g_server_status));
		}
	}
}


/* for now */
static const GDBusInterfaceVTable interface_vtable = {
	handle_method_call,
	NULL,
	NULL,
	{NULL}
};

static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	guint registration_id;

	scmirroring_debug("on_bus_acquired called\n");
	registration_id = g_dbus_connection_register_object(connection,
												PROC_DBUS_OBJECT,
												introspection_data->interfaces[0],
												&interface_vtable,
												NULL,
												NULL,  /* user_data_free_func */
												NULL); /* GError** */
	g_assert(registration_id > 0);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	scmirroring_debug("on_name_acquired called\n");
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	scmirroring_debug("on_name_lost called\n");
}

G_DEFINE_TYPE(MiracastServer, miracast_server, G_TYPE_OBJECT);

static int __miracast_server_emit_status_signal(int status);
static void __miracast_server_interpret(MiracastServer *server, gchar *buf);
static int __miracast_server_send_resp(MiracastServer *server, const gchar *cmd);
static void __miracast_server_quit_program(MiracastServer *server);
static int __miracast_server_start(MiracastServer *server_obj);
static gboolean __miracast_server_setup(MiracastServer *server_obj);

static void miracast_server_init(MiracastServer *obj)
{
	scmirroring_debug("miracast_server_init\n");

	obj->client_channel = NULL;
	obj->ip = NULL;
	obj->port = NULL;
	obj->server = NULL;
	obj->client = NULL;
	obj->factory = NULL;
	obj->resolution = 0;
	obj->multisink = SCMIRRORING_MULTISINK_DISABLE;
}

static void miracast_server_class_init(MiracastServerClass *klass)
{
	scmirroring_debug("miracast_server_class_init\n");
	//GObjectClass *gobject_class;

	//gobject_class = G_OBJECT_CLASS (klass);
	//gobject_class->finalize = gst_rtsp_server_finalize;

	klass->interpret = __miracast_server_interpret;
	klass->send_response = __miracast_server_send_resp;
	klass->quit_server = __miracast_server_quit_program;
	klass->server_start = __miracast_server_start;
	klass->server_setup = __miracast_server_setup;
}

int __miracast_server_send_resp(MiracastServer *server, const gchar *cmd)
{
	int ret = SCMIRRORING_ERROR_NONE;
	char *_cmd = NULL;
	int client_sock = -1;

	if (server == NULL) {
		scmirroring_error("OUT_OF_MEMORY");
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	client_sock = g_io_channel_unix_get_fd(server->client_channel);
	if (client_sock < 0) {
		scmirroring_error("client sock fd is invalid!");
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	_cmd = g_strdup(cmd);
	_cmd[strlen(_cmd)] = '\0';

	if (write(client_sock, _cmd, strlen(_cmd) + 1) != ((int)(strlen(_cmd) + 1))) {
		char buf[255] = {0, };
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("sendto failed [%s]", buf);
		ret = SCMIRRORING_ERROR_INVALID_OPERATION;
	} else {
		scmirroring_debug("Sent response [%s] successfully", _cmd);
	}

	g_free(_cmd);
	return ret;
}

void __miracast_server_quit_program(MiracastServer *server)
{
	scmirroring_error("Quit program is called");

	MiracastServerClass *klass;
	klass = MIRACAST_SERVER_GET_CLASS(server);

	void *pool = NULL;
	int i;
	int ret = 0;

	if (server->server == NULL) {
		scmirroring_error("server is already NULL");
		goto done;
	}

	ret = __miracast_server_emit_status_signal(MIRACAST_WFD_SOURCE_OFF);
	if (ret != SCMIRRORING_ERROR_NONE) {
		scmirroring_error("Failed to emit miracast server off signal");
	}

	pool = (void *)gst_rtsp_server_get_session_pool(server->server);
	if (pool) {
		gst_rtsp_session_pool_cleanup(pool);
		g_object_unref(pool);
	}

	int serv_ref_cnt = GST_OBJECT_REFCOUNT_VALUE(server->server);
	scmirroring_debug("serv ref cnt:%d", serv_ref_cnt);

	for (i = 0; i < serv_ref_cnt; i++)
		g_object_unref(server->server);

done:
	klass->send_response(server, "OK:DESTROY");
	g_main_loop_quit(server->mainloop);
}

static void __miracast_server_signal_handler(int signo)
{
	scmirroring_error("__miracast_server_signal_handler call quit_program() %d", signo);
	int ret = 0;

	switch (signo) {
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			ret = __miracast_server_emit_status_signal(MIRACAST_WFD_SOURCE_OFF);
			if (ret != SCMIRRORING_ERROR_NONE) {
				scmirroring_error("Failed to emit miracast server off signal");
			}
			break;
		default:
			break;
	}
	exit(1);
}

static void __miracast_server_set_signal()
{
	/*SIGABRT  A  Process abort signal.
	   SIGALRM   T  Alarm clock.
	   SIGBUS    A  Access to an undefined portion of a memory object.
	   SIGCHLD  I  Child process terminated, stopped,
	   SIGCONT  C  Continue executing, if stopped.
	   SIGFPE  A  Erroneous arithmetic operation.
	   SIGHUP  T  Hangup.
	   SIGILL  A  Illegal instruction.
	   SIGINT  T  Terminal interrupt signal.
	   SIGKILL  T  Kill(cannot be caught or ignored).
	   SIGPIPE  T  Write on a pipe with no one to read it.
	   SIGQUIT  A  Terminal quit signal.
	   SIGSEGV  A  Invalid memory reference.
	   SIGSTOP  S  Stop executing(cannot be caught or ignored).
	   SIGTERM  T  Termination signal.
	   SIGTSTP  S  Terminal stop signal.
	   SIGTTIN  S  Background process attempting read.
	   SIGTTOU  S  Background process attempting write.
	   SIGUSR1  T  User-defined signal 1.
	   SIGUSR2  T  User-defined signal 2.
	   SIGPOLL  T  Pollable event.
	   SIGPROF  T  Profiling timer expired.
	   SIGSYS  A  Bad system call.
	   SIGTRAP  A  Trace/breakpoint trap.
	   SIGURG  I  High bandwidth data is available at a socket.
	   SIGVTALRM  T  Virtual timer expired.
	   SIGXCPU  A  CPU time limit exceeded.
	   SIGXFSZ  A  File size limit exceeded.

	The default actions are as follows:
	T : Abnormal termination of the process. The process is terminated with all the consequences of _exit()
	A : Abnormal termination of the process.
	I  : Ignore the signal.
	S : Stop the process
	*/
	struct sigaction act_new;
	memset(&act_new, 0, sizeof(struct sigaction));

	act_new.sa_handler = __miracast_server_signal_handler;

	sigaction(SIGABRT, &act_new, NULL);
	sigaction(SIGBUS, &act_new, NULL);
	sigaction(SIGINT, &act_new, NULL);
	sigaction(SIGKILL, &act_new, NULL);
	sigaction(SIGPIPE, &act_new, NULL);
	sigaction(SIGQUIT, &act_new, NULL);
	sigaction(SIGSEGV, &act_new, NULL);
	sigaction(SIGTERM, &act_new, NULL);
	sigaction(SIGSYS, &act_new, NULL);
}

static gboolean __miracast_server_setup(MiracastServer* server)
{
	scmirroring_debug("__miracast_server_setup start\n");

	introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	if (introspection_data == NULL) {
		scmirroring_error("Introspection data is NULL");
		return false;
	}

	g_bus_own_name(G_BUS_TYPE_SYSTEM,
					PROC_DBUS_INTERFACE,
					G_BUS_NAME_OWNER_FLAGS_NONE,
					on_bus_acquired,
					on_name_acquired,
					on_name_lost,
					G_OBJECT(server),
					NULL);

	__miracast_server_set_signal();
	scmirroring_debug("__miracast_server_setup end\n");

	return true;
}

static gboolean __miracast_server_ready_channel(int *sockfd)
{
	int sock;
	int i;
	gboolean bind_success = FALSE;
	struct sockaddr_un serv_addr;
	char buf[255] = {0, };

	/* Create a TCP socket */
	if ((sock = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("socket failed: %s", buf);
		return FALSE;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;
	unlink(MEDIA_IPC_PATH);
	strncpy(serv_addr.sun_path, MEDIA_IPC_PATH, sizeof(serv_addr.sun_path));

	/* Bind to the local address */
	for (i = 0; i < 20; i++) {
		if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
			bind_success = true;
			break;
		}
		usleep(250000);
	}

	if (bind_success == false) {
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("bind failed : %s %d_", buf, errno);
		close(sock);
		return FALSE;
	}

	scmirroring_debug("bind success");

	/* Listening */
	if (listen(sock, SOMAXCONN) < 0) {
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("listen failed : %s", buf);
		close(sock);
		return FALSE;
	}

	/*change permission of sock file*/
	if (chmod(MEDIA_IPC_PATH, 0666) < 0) {
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("chmod failed [%s]", buf);
	}

	scmirroring_debug("Listening...");
	*sockfd = sock;

	return TRUE;
}

void __miracast_server_msg_cb(gboolean is_error, void *userdata)
{
	/*MiracastServer *server = (MiracastServer *)userdata; */

	if (is_error) {
		scmirroring_error("Error occured");
	}
}

gint miracast_server_gst_init()
{
	gint *argc = NULL;
	gchar **argv = NULL;
	static const int max_argc = 50;
	GError *err = NULL;
	int i = 0;

	/* alloc */
	argc = calloc(1, sizeof(int));
	if (!argc) {
		scmirroring_error("Cannot allocate memory for scmirroringsink\n");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	argv = calloc(max_argc, sizeof(gchar *));
	if (!argv) {
		scmirroring_error("Cannot allocate memory for scmirroringsink\n");
		SCMIRRORING_SAFE_FREE(argc);
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	/* we would not do fork for scanning plugins */
	argv[*argc] = g_strdup("--gst-disable-registry-fork");
	(*argc)++;

	/* check disable registry scan */
	if (scmirroring_src_ini_get_structure()->skip_rescan) {
		argv[*argc] = g_strdup("--gst-disable-registry-update");
		(*argc)++;
	}

	/* check disable segtrap */
	if (scmirroring_src_ini_get_structure()->disable_segtrap) {
		argv[*argc] = g_strdup("--gst-disable-segtrap");
		(*argc)++;
	}

	scmirroring_debug("initializing gstreamer with following parameter\n");
	scmirroring_debug("argc : %d\n", *argc);

	for (i = 0; i < SCMIRRORING_SRC_INI_GST_START_PARAMS_NUM; i++) {
		if (g_strcmp0(scmirroring_src_ini_get_structure()->gst_param[i], "") != 0) {
			argv[*argc] = g_strdup(scmirroring_src_ini_get_structure()->gst_param[i]);
			(*argc)++;
		}
	}

	for (i = 0; i < *argc; i++) {
		scmirroring_debug("argv[%d] : %s\n", i, argv[i]);
	}

	/* initializing gstreamer */
	if (!gst_init_check(argc, &argv, &err)) {
		scmirroring_error("Could not initialize GStreamer: %s\n", err ? err->message : "unknown error occurred");
		if (err) {
			g_error_free(err);
		}
		goto ERROR;
	}

	/* release */
	for (i = 0; i < *argc; i++) {
		SCMIRRORING_SAFE_FREE(argv[i]);
	}
	SCMIRRORING_SAFE_FREE(argv);
	SCMIRRORING_SAFE_FREE(argc);

	return SCMIRRORING_ERROR_NONE;

ERROR:
	for (i = 0; i < *argc; i++) {
		SCMIRRORING_SAFE_FREE(argv[i]);
	}
	SCMIRRORING_SAFE_FREE(argv);
	SCMIRRORING_SAFE_FREE(argc);
	return SCMIRRORING_ERROR_INVALID_OPERATION;
}


static void
__client_closed(GstRTSPClient *client)
{
	if (client == NULL) return;

	scmirroring_debug("client %p: connection closed", client);

	return;
}

static void
__new_session(GstRTSPClient *client, GstRTSPSession *session)
{
	if (client == NULL) return;

	scmirroring_debug("New session(%p) is made for client %p", session, client);

	return;
}

static void
__teardown_req(GstRTSPClient *client, GstRTSPContext *ctx)
{
	if (client == NULL) return;

	scmirroring_debug("Got TEARDOWN request for client %p", client);

	return;
}

static gchar* __check_requirements_cb(GstRTSPClient * client, GstRTSPContext * ctx, gchar ** req, gpointer user_data)
{
	int index = 0;
	GString *result = g_string_new("");

	while (req[index] != NULL) {
		if (g_strcmp0(req[index], WFD_REQUIREMENT)) {
			if (result->len > 0)
				g_string_append(result, ", ");
			g_string_append(result, req[index]);
		}
		index++;
	}

	return  g_string_free(result, FALSE);
}

static void __miracast_server_client_connected_cb(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data)
{
	MiracastServer *server_obj = (MiracastServer *)user_data;
	MiracastServerClass *klass;
	klass = MIRACAST_SERVER_GET_CLASS(server_obj);

	scmirroring_debug("There is a client, connected");
	server_obj->client = (void *)client;

	g_signal_connect(G_OBJECT(client), "teardown-request", G_CALLBACK(__teardown_req), NULL);
	g_signal_connect(G_OBJECT(client), "closed", G_CALLBACK(__client_closed), NULL);
	g_signal_connect(G_OBJECT(client), "new-session", G_CALLBACK(__new_session), NULL);
	g_signal_connect(G_OBJECT(client), "check-requirements", G_CALLBACK(__check_requirements_cb), NULL);

	/* Sending connected response to client */
	klass->send_response(server_obj, "OK:CONNECTED");
}

static void __media_constructed(GstRTSPMediaFactory * factory, GstRTSPMedia * media, gpointer user_data)
{
	guint i, n_streams;
	MiracastServer *server_obj = (MiracastServer *)user_data;

	if (!server_obj || !server_obj->ip)
		return;

	n_streams = gst_rtsp_media_n_streams(media);

	for (i = 0; i < n_streams; i++) {
		GstRTSPAddressPool *pool;
		GstRTSPStream *stream;

		stream = gst_rtsp_media_get_stream(media, i);

		/* make a new address pool */
		pool = gst_rtsp_address_pool_new();

		scmirroring_debug("Setting IP:%s... ", server_obj->ip);
		gst_rtsp_address_pool_add_range(pool, server_obj->ip, server_obj->ip, 19000, 19001, 0);

		gst_rtsp_stream_set_address_pool(stream, pool);
		g_object_unref(pool);
	}
}

int __miracast_server_start(MiracastServer *server_obj)
{
	int ret = SCMIRRORING_ERROR_NONE;

	if (scmirroring_src_ini_load() != ERROR_NONE) {
		scmirroring_error("Error occurred during parsing config file");
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	ret = miracast_server_gst_init();
	if (ret < SCMIRRORING_ERROR_NONE) {
		scmirroring_error("Gstreamer init failed");
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	guint id;
	guint native_reso = 0;
	guint supported_reso = 0;
	GstRTSPWFDServer *server = NULL;
	GstRTSPMediaFactoryWFD *factory = NULL;
	GstRTSPMountPoints *mounts = NULL;

	server = (void *)gst_rtsp_wfd_server_new();
	if (NULL == server) {
		scmirroring_error("Failed to create server...");
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	gst_rtsp_server_set_address(GST_RTSP_SERVER(server), server_obj->ip);
	gst_rtsp_server_set_service(GST_RTSP_SERVER(server), server_obj->port);
	mounts = gst_rtsp_server_get_mount_points(GST_RTSP_SERVER(server));
	if (NULL == mounts) {
		scmirroring_error("Failed to get mount point...");
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	factory = gst_rtsp_media_factory_wfd_new();

	gst_rtsp_media_factory_wfd_set(factory,
							scmirroring_src_ini_get_structure()->videosrc_element,
							scmirroring_src_ini_get_structure()->name_of_audio_device,
							scmirroring_src_ini_get_structure()->audio_latency_time,
							scmirroring_src_ini_get_structure()->audio_buffer_time,
							scmirroring_src_ini_get_structure()->audio_do_timestamp,
							scmirroring_src_ini_get_structure()->mtu_size
	);

	gst_rtsp_media_factory_wfd_set_encoders(factory,
			scmirroring_src_ini_get_structure()->name_of_video_encoder,
			scmirroring_src_ini_get_structure()->name_of_audio_encoder_aac,
			scmirroring_src_ini_get_structure()->name_of_audio_encoder_ac3);

	gst_rtsp_wfd_server_set_audio_codec(server,
			scmirroring_src_ini_get_structure()->audio_codec);

	/* CEA Resolution */
	if (server_obj->resolution & SCMIRRORING_RESOLUTION_1920x1080_P30) {
		native_reso = GST_WFD_VIDEO_CEA_RESOLUTION;
		supported_reso |= GST_WFD_CEA_1920x1080P30;
	}
	if (server_obj->resolution & SCMIRRORING_RESOLUTION_1280x720_P30) {
		native_reso = GST_WFD_VIDEO_CEA_RESOLUTION;
		supported_reso |= GST_WFD_CEA_1280x720P30;
	}
	if (server_obj->resolution & SCMIRRORING_RESOLUTION_720x480_P60) {
		native_reso = GST_WFD_VIDEO_CEA_RESOLUTION;
		supported_reso |= GST_WFD_CEA_720x480P60;
	}
	if (server_obj->resolution & SCMIRRORING_RESOLUTION_640x480_P60) {
		native_reso = GST_WFD_VIDEO_CEA_RESOLUTION;
		supported_reso |= GST_WFD_CEA_640x480P60;
	}

	/* HH Resolution */
	if (server_obj->resolution & SCMIRRORING_RESOLUTION_960x540_P30) {
		native_reso = GST_WFD_VIDEO_HH_RESOLUTION;
		supported_reso |= GST_WFD_HH_960x540P30;
	}
	if (server_obj->resolution & SCMIRRORING_RESOLUTION_864x480_P30) {
		native_reso = GST_WFD_VIDEO_HH_RESOLUTION;
		supported_reso |= GST_WFD_HH_864x480P30;
	}
	if (server_obj->resolution & SCMIRRORING_RESOLUTION_640x360_P30) {
		native_reso = GST_WFD_VIDEO_HH_RESOLUTION;
		supported_reso |= GST_WFD_HH_640x360P30;
	}

	if (server_obj->resolution == 0) {
		/* Use default */
		gst_rtsp_wfd_server_set_video_native_reso(server, GST_WFD_VIDEO_CEA_RESOLUTION);
		gst_rtsp_wfd_server_set_supported_reso(server, scmirroring_src_ini_get_structure()->video_reso_supported);
	} else {
		gst_rtsp_wfd_server_set_video_native_reso(server, native_reso);
		gst_rtsp_wfd_server_set_supported_reso(server, supported_reso);
	}

	gst_rtsp_media_factory_wfd_set_dump_ts(factory, scmirroring_src_ini_get_structure()->dump_ts);
	if (server_obj->multisink == SCMIRRORING_MULTISINK_ENABLE)
		gst_rtsp_media_factory_set_shared(GST_RTSP_MEDIA_FACTORY_CAST(factory), TRUE);

	g_signal_connect(GST_RTSP_MEDIA_FACTORY(factory), "media-constructed", (GCallback) __media_constructed, server_obj);

	g_object_ref(factory);
	gst_rtsp_mount_points_add_factory(mounts, TEST_MOUNT_POINT, GST_RTSP_MEDIA_FACTORY(factory));
	g_object_unref(mounts);

	/* attach the server to the default maincontext */
	if ((id = gst_rtsp_server_attach(GST_RTSP_SERVER_CAST(server), NULL)) == 0)
		goto failed;

	g_signal_connect(server, "client-connected", G_CALLBACK(__miracast_server_client_connected_cb), server_obj);

	server_obj->server = (void *) server;
	server_obj->factory = (void *) factory;

	return SCMIRRORING_ERROR_NONE;

failed:
	scmirroring_error("Failed to start miracast server");
	return SCMIRRORING_ERROR_INVALID_OPERATION;
}

void __miracast_server_interpret(MiracastServer *server, gchar *buf)
{
	int ret = SCMIRRORING_ERROR_NONE;
	MiracastServerClass *klass;
	klass = MIRACAST_SERVER_GET_CLASS(server);

	scmirroring_debug("Received : %s", buf);

	if (g_strrstr(buf, SCMIRRORING_STATE_CMD_START)) {
		ret = klass->server_start(server);
		if (ret == SCMIRRORING_ERROR_NONE) {
			klass->send_response(server, "OK:LISTENING");

			ret = __miracast_server_emit_status_signal(MIRACAST_WFD_SOURCE_ON);
			if (ret != SCMIRRORING_ERROR_NONE) {
				scmirroring_error("Failed to emit miracast server on signal");
			}

		} else {
			klass->send_response(server, "FAIL:LISTENING");
		}
	} else if (g_strrstr(buf, "SET IP")) {
		gchar **addr_info;
		gchar **IP_Port;

		addr_info = g_strsplit(buf, " ", 0);
		IP_Port = g_strsplit(addr_info[2], ":", 0);

		scmirroring_debug("IP: %s, Port: %s", IP_Port[0], IP_Port[1]);
		server->ip = g_strdup(IP_Port[0]);
		server->port = g_strdup(IP_Port[1]);

		g_strfreev(IP_Port);
		g_strfreev(addr_info);

		klass->send_response(server, "OK:SET");
	} else if (g_strrstr(buf, "SET CM")) {
		gchar **conn_mode_info;
		gint connection_mode = 0;

		conn_mode_info = g_strsplit(buf, " ", 0);

		connection_mode = atoi(conn_mode_info[2]);
		scmirroring_debug("Connection mode %d", connection_mode);

		server->connection_mode = connection_mode;

		g_strfreev(conn_mode_info);

		klass->send_response(server, "OK:SET");
	} else if (g_strrstr(buf, "SET RESO")) {
		gchar **resolution_info;
		gint resolution = 0;

		resolution_info = g_strsplit(buf, " ", 0);

		resolution = atoi(resolution_info[2]);
		scmirroring_debug("Resolution %d", resolution);

		server->resolution = resolution;

		g_strfreev(resolution_info);

		klass->send_response(server, "OK:SET");
	} else if (g_strrstr(buf, "SET MULTISINK")) {
		gchar **multisink_info;
		gint multisink = 0;

		multisink_info = g_strsplit(buf, " ", 0);

		multisink = atoi(multisink_info[2]);
		scmirroring_debug("Multisink %d", multisink);

		server->multisink = multisink;

		g_strfreev(multisink_info);

		klass->send_response(server, "OK:SET");
	} else if (g_strrstr(buf, SCMIRRORING_STATE_CMD_PAUSE)) {
		gst_rtsp_wfd_server_trigger_request(GST_RTSP_SERVER(server->server), WFD_TRIGGER_PAUSE);
		klass->send_response(server, "OK:PAUSE");
	} else if (g_strrstr(buf, SCMIRRORING_STATE_CMD_RESUME)) {
		gst_rtsp_wfd_server_trigger_request(GST_RTSP_SERVER(server->server), WFD_TRIGGER_PLAY);
		klass->send_response(server, "OK:RESUME");
	} else if (g_strrstr(buf, SCMIRRORING_STATE_CMD_STOP)) {
		gst_rtsp_wfd_server_trigger_request(GST_RTSP_SERVER(server->server), WFD_TRIGGER_TEARDOWN);
		klass->send_response(server, "OK:STOP");
	} else if (g_strrstr(buf, SCMIRRORING_STATE_CMD_DESTROY)) {
		klass->quit_server(server);
	}
}

int __miracast_server_accept(int serv_sock, int *client_sock)
{
	int sockfd = -1;
	struct sockaddr_un client_addr;
	socklen_t client_addr_len;

	if (client_sock == NULL)
		return SCMIRRORING_ERROR_INVALID_PARAMETER;

	client_addr_len = sizeof(client_addr);
	if ((sockfd = accept(serv_sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
		char buf[255] = {0, };
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("accept failed : %s", buf);
		*client_sock = -1;
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	*client_sock = sockfd;

	return SCMIRRORING_ERROR_NONE;
}

gboolean __miracast_server_client_read_cb(GIOChannel *src, GIOCondition condition, gpointer data)
{
	char buf[MAX_MSG_LEN + 1];
	gsize read;
	MiracastServer *server = (MiracastServer *)data;
	MiracastServerClass *klass;
	klass = MIRACAST_SERVER_GET_CLASS(server);

	if (condition & G_IO_IN) {
		g_io_channel_read_chars(server->client_channel, buf, MAX_MSG_LEN, &read, NULL);
		if (read == 0) {
			scmirroring_error("Read 0 bytes");
			return FALSE;
		} else {
			scmirroring_debug("Read %d bytes", read);
		}

		gsize i = 0;
		gsize idx = 0;

		/* Handling multiple commands like "CMD1\0CMD2\0CMD3\0" */
		for (i = 0; i < read; i++) {
			gchar *str = NULL;
			if (buf[i] == '\0') {
				str = buf + idx;
				idx = i + 1;
			} else {
				continue;
			}
			scmirroring_debug("Handling %s", str);
			//__miracast_server_interpret(server, str);
			klass->interpret(server, str);
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


gboolean __miracast_server_read_cb(GIOChannel *src, GIOCondition condition, gpointer data)
{
	MiracastServer *server = (MiracastServer *)data;

	int server_sock = -1;
	int client_sock = -1;

	server_sock = g_io_channel_unix_get_fd(src);
	if (server_sock < 0) {
		scmirroring_error("server sock fd is invalid!");
		return TRUE;
	}

	__miracast_server_accept(server_sock, &client_sock);

	scmirroring_debug("New client accepted : %d", client_sock);

	GIOChannel *channel = NULL;
	channel = g_io_channel_unix_new(client_sock);
	if (channel == NULL) {
		char buf[255] = {0, };
		strerror_r(errno, buf, sizeof(buf));
		scmirroring_error("g_io_channel_unix_new failed: %s", buf);
	}

	/* Create new channel to watch udp socket */
	GSource *source = NULL;
	source = g_io_create_watch(channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
	/*int source_id = -1; */

	/* To avoid blocking in g_io_channel_read_chars */
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);
	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

	server->client_channel = channel;

	/* Set callback to be called when socket is readable */
	g_source_set_callback(source, (GSourceFunc)__miracast_server_client_read_cb, server, NULL);
	g_source_attach(source, g_main_context_get_thread_default());

	return TRUE;
}

static int __miracast_server_emit_status_signal(int status)
{
	GError *err = NULL;
	GDBusConnection *conn = NULL;
	gboolean ret;

	if (status != MIRACAST_WFD_SOURCE_ON && status != MIRACAST_WFD_SOURCE_OFF) {
		scmirroring_error("invalid arguments [%d]", status);
		return -1;
	}

	if (g_server_status == status) {
		scmirroring_debug("The server status is not changed, status [%d] ", status);
		return 0;
	}

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		if (err) {
			scmirroring_error("g_bus_get_sync() error (%s) ", err->message);
			g_error_free(err);
		} else {
			scmirroring_error("g_bus_get_sync() unknown error ");
		}
		return -1;
	}

	ret = g_dbus_connection_emit_signal(conn,
									NULL, PROC_DBUS_OBJECT, PROC_DBUS_INTERFACE, PROC_DBUS_STATUS_CHANGE_SIGNAL,
									g_variant_new("(i)", status), &err);
	if (!ret && err) {
		scmirroring_error("g_dbus_connection_emit_signal() error(%s) ", err->message);
		goto error;
	}

	ret = g_dbus_connection_flush_sync(conn, NULL, &err);
	if (!ret && err) {
		scmirroring_error("g_dbus_connection_flush_sync() error(%s) ", err->message);
		goto error;
	}

	g_object_unref(conn);
	scmirroring_debug("sending miracast server status [%s] success",
					(status == MIRACAST_WFD_SOURCE_ON) ? "On" : "Off");

	g_server_status = status;

	return 0;

error:
	g_error_free(err);
	if (conn) g_object_unref(conn);
	return -1;
}

MiracastServer* miracast_server_new(void)
{
	return g_object_new(MIRACAST_SERVER_TYPE_OBJECT, NULL);
}

gboolean miracast_server_setup(MiracastServer *server, GMainLoop *mainloop)
{
	int sockfd = -1;

	GSource *source = NULL;
	GIOChannel *channel = NULL;
	GMainContext *context = NULL;
	MiracastServerClass *klass = NULL;

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	klass = MIRACAST_SERVER_GET_CLASS(server);
	if (klass == NULL) {
		scmirroring_error("Unable to get miracast server class");
		return FALSE;
	}

	if (!klass->server_setup(server)) {
		scmirroring_error("Unable to initialize test server\n");
		return FALSE;
	}

	if (!__miracast_server_ready_channel(&sockfd)) {
		scmirroring_error("Unable to create channel");
		return FALSE;
	}

	context = g_main_loop_get_context(mainloop);
	server->mainloop = mainloop;

	/* Create new channel to watch udp socket */
	channel = g_io_channel_unix_new(sockfd);
	source = g_io_create_watch(channel, G_IO_IN);

	/* Set callback to be called when socket is readable */
	g_source_set_callback(source, (GSourceFunc)__miracast_server_read_cb, server, NULL);
	g_source_attach(source, context);

	return TRUE;
}
