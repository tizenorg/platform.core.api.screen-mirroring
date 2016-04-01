/*
 * capi-media-streaming
 *
 * Copyright (c) 2011 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __MIRACAST_SERVER_H__
#define __MIRACAST_SERVER_H__

#include <gst/gst.h>

typedef struct _MiracastServer MiracastServer;
typedef struct _MiracastServerClass MiracastServerClass;

#define MIRACAST_SERVER_TYPE_OBJECT       (miracast_server_get_type())
#define MIRACAST_SERVER(object)           (G_TYPE_CHECK_INSTANCE_CAST \
                                          ((object), MIRACAST_SERVER_TYPE_OBJECT, MiracastServer))
#define MIRACAST_SERVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), MIRACAST_SERVER_TYPE_OBJECT, MiracastServerClass))
#define MIRACAST_SERVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), MIRACAST_SERVER_TYPE_OBJECT, MiracastServerClass))
#define MIRACAST_SERVER_CAST(obj)         ((MiracastServer*)(obj))
#define MIRACAST_SERVER_CLASS_CAST(klass) ((MiracastServerClass*)(klass))
#define IS_MIRACAST_SERVER_OBJECT(obj)    (G_TYPE_CHECK_INSTANCE_TYPE((obj), MIRACAST_SERVER_TYPE_OBJECT))
#define IS_MIRACAST_SERVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), MIRACAST_SERVER_TYPE_OBJECT))

struct _MiracastServer {
	GObject parent;

	GIOChannel *client_channel;
	GMainLoop *mainloop;
	gchar *ip;
	gchar *port;
	void *server;
	void *client;
	void *factory;
	gint resolution;
	gint connection_mode;
	gint multisink;

	gpointer _gst_reserved[GST_PADDING];
};

struct _MiracastServerClass {
	GObjectClass parent;

	void     (*interpret)     (MiracastServer *server, gchar *buf);
	int      (*send_response) (MiracastServer *server, const gchar *cmd);
	int      (*server_start)  (MiracastServer *server);
	void     (*quit_server)   (MiracastServer *server);
	gboolean (*server_setup)  (MiracastServer *server);

	gpointer _gst_reserved[GST_PADDING_LARGE - 1];
};

GType            miracast_server_get_type(void);
MiracastServer*  miracast_server_new(void);
gboolean         miracast_server_setup(MiracastServer *server, GMainLoop *mainloop);
gint             miracast_server_gst_init();

#endif
