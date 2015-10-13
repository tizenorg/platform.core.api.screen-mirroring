/*
 * miracast-server
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

#include <glib.h>
#include "miracast_server.h"
#include "scmirroring_private.h"

int main(int argc, char *argv[])
{
	GMainLoop *mainloop = NULL;
	MiracastServer *server = NULL;

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	server = miracast_server_new();
	if (NULL == server) {
		scmirroring_error("Unable to create miracast server object! Aborting miracast server");
		exit(-1);
	}

	mainloop = g_main_loop_new(NULL, FALSE);
	if (NULL == mainloop) {
		scmirroring_error("Unable to create gmain loop! Aborting miracast server");
		exit(-1);
	}

	if (!miracast_server_setup(server, mainloop)) {
		scmirroring_error("Unable to initialize miracast server");
		exit(-1);
	}

	scmirroring_debug("miracast server run loop");
	g_main_loop_run(mainloop);

	scmirroring_debug("MIRACAST SERVER EXIT");
	exit(0);
}
