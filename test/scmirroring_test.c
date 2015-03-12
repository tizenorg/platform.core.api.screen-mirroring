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

#include <stdlib.h>
#include <string.h>
#include <glib/gprintf.h>
#include <scmirroring_src.h>

#define MAX_STRING_LEN    2048

scmirroring_src_h g_scmirroring = NULL;
GMainLoop *g_loop;

static void scmirroring_state_callback(scmirroring_error_e error_code, scmirroring_state_e state, void *user_data)
{
	g_print("\n\nReceived Callback error code[%d], state[%d]\n\n", error_code, state);

	return;
}

static void __quit_program(void)
{
	g_print ("Quit Program\n");

	g_scmirroring = 0;
	g_main_loop_quit(g_loop);
}

static void __displaymenu(void)
{
	g_print("\n");
	g_print("=====================================================================\n");
	g_print("                          SCMIRRORING Testsuite (press q to quit) \n");
	g_print("=====================================================================\n");
	g_print("a : a ip port (ex. a 192.168.49.1 2022)\n");
	g_print("c : set resopution (ex. c 0 (0 : 1920x1080_P30, 1 : 1280x720_P30, 2 : 960x540_P30, 3: 640x360_P30)\n");
	g_print("f : set connection mode (ex. f 0 (0 : wifi_direct, 1 : Other)\n");
	g_print("C : Connect\n");
	g_print("I : dIsconnect\n");
	g_print("S : Start  \n");
	g_print("P : Pause \n");
	g_print("R : Resume \n");
	g_print("T : sTop\n");
	g_print("D : Destroy\n");
	g_print("q : quit\n");
	g_print("-----------------------------------------------------------------------------------------\n");
}

gboolean __timeout_menu_display(void* data)
{
	__displaymenu();
	return FALSE;
}

static void __interpret (char *cmd)
{
	int ret = SCMIRRORING_ERROR_NONE;
	gchar **value;
	value = g_strsplit(cmd," ",0);

	if (strncmp(cmd, "a", 1) == 0)
	{
		ret = scmirroring_src_set_ip_and_port(g_scmirroring, value[1], value[2]);
		g_print ("Input server IP and port number IP[%s] Port[%s]\n", value[1], value[2]);
	}
	else if (strncmp(cmd, "c", 1) == 0)
	{
		ret = scmirroring_src_set_resolution(g_scmirroring, atoi(value[1]));
		g_print ("Set Resolution[%d]\n", atoi(value[1]));
	}
	else if (strncmp(cmd, "f", 1) == 0)
	{
		ret = scmirroring_src_set_connection_mode(g_scmirroring, atoi(value[1]));
		g_print ("Connection mode [%d]\n", atoi(value[1]));
	}
	else if (strncmp(cmd, "C", 1) == 0)
	{
		g_print ("Connect\n");
		ret = scmirroring_src_connect(g_scmirroring);
	}
	else if (strncmp(cmd, "I", 1) == 0)
	{
		g_print ("dIsconnect\n");
		ret = scmirroring_src_disconnect(g_scmirroring);
	}
	else if (strncmp(cmd, "S", 1) == 0)
	{
		g_print ("Start\n");
		ret = scmirroring_src_start(g_scmirroring);
	}
	else if (strncmp(cmd, "P", 1) == 0)
	{
		g_print ("Pause\n");
		ret = scmirroring_src_pause(g_scmirroring);
	}
	else if (strncmp(cmd, "R", 1) == 0)
	{
		g_print ("Resume\n");
		ret = scmirroring_src_resume(g_scmirroring);
	}
	else if (strncmp(cmd, "T", 1) == 0)
	{
		g_print ("Stop\n");
		ret = scmirroring_src_stop(g_scmirroring);
	}
	else if (strncmp(cmd, "D", 1) == 0)
	{
		g_print ("Destroy\n");
		ret = scmirroring_src_destroy(g_scmirroring);
	}
	else if (strncmp(cmd, "q", 1) == 0)
	{
		__quit_program();
	}
	else
	{
		g_print("unknown menu \n");
	}

	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_print("Error Occured [%d]\n", ret);
	}

	g_timeout_add(100, __timeout_menu_display, 0);

return;
}

gboolean __input (GIOChannel *channel)
{
	char buf[MAX_STRING_LEN + 3];
	gsize read;
	GError *error = NULL;

	g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);
	buf[read] = '\0';
	g_strstrip(buf);
	__interpret (buf);
	return TRUE;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	GIOChannel *stdin_channel;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags (stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)__input, NULL);

	ret = scmirroring_src_create(&g_scmirroring);
	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_print("scmirroring_src_create fail [%d]", ret);
		return 0;
	}

	ret = scmirroring_src_set_state_changed_cb(g_scmirroring, scmirroring_state_callback, NULL);
	if(ret != SCMIRRORING_ERROR_NONE)
	{
		g_print("scmirroring_src_create fail [%d]", ret);
		return 0;
	}

	__displaymenu();

	g_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(g_loop);

	return ret;
}
