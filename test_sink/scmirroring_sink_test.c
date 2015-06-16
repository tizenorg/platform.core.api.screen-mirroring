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
#include <scmirroring_sink.h>
#include <wifi-direct.h>

#define MAX_STRING_LEN    2048
#define SINKTEST_EXECUTE_DELAY	5000
#define MAIN_MENU 0
#define SUBMENU_RESOLUTION 1

/*#define TEST_WITH_WIFI_DIRECT */

scmirroring_sink_h g_scmirroring = NULL;
gint g_resolution = 0;
gint g_menu = MAIN_MENU;

GMainLoop *g_loop;
#ifdef TEST_WITH_WIFI_DIRECT
static int g_peer_cnt = 0;
static char g_peer_ip[32];
static char g_peer_port[32];
#define DEFAULT_SCREEN_MIRRORING_PORT 2022
#endif

gboolean __scmirroring_sink_start(gpointer data);

#ifndef TEST_WITH_WIFI_DIRECT
static int __scmirroring_sink_create(gpointer data);
#endif
#ifdef TEST_WITH_WIFI_DIRECT
static gboolean __start_p2p_connection(gpointer data);
static gboolean __disconnect_p2p_connection(void);
#endif
static void __quit_program(void);
gboolean 	__timeout_menu_display(void *data);
static void __display_resolution_submenu(void);
gboolean 	__timeout_resolution_submenu_display(void *data);
static void __interpret_resolution_submenu(char *cmd);


gboolean __timeout_resolution_submenu_display(void *data)
{
	__display_resolution_submenu();
	return FALSE;
}

static void __display_resolution_submenu(void)
{
	g_print("\n");
	g_print("**********************************************************************\n");
	g_print("     Setting resolution \n");
	g_print("**********************************************************************\n");
	g_print("1 : SCMIRRORING_RESOLUTION_1920x1080_P30 [%d]\n", SCMIRRORING_RESOLUTION_1920x1080_P30);
	g_print("2 : SCMIRRORING_RESOLUTION_1280x720_P30  [%d]\n", SCMIRRORING_RESOLUTION_1280x720_P30);
	g_print("3 : SCMIRRORING_RESOLUTION_960x540_P30   [%d]\n", SCMIRRORING_RESOLUTION_960x540_P30);
	g_print("4 : SCMIRRORING_RESOLUTION_864x480_P30   [%d]\n", SCMIRRORING_RESOLUTION_864x480_P30);
	g_print("5 : SCMIRRORING_RESOLUTION_720x480_P60   [%d]\n", SCMIRRORING_RESOLUTION_720x480_P60);
	g_print("6 : SCMIRRORING_RESOLUTION_640x480_P60   [%d]\n", SCMIRRORING_RESOLUTION_640x480_P60);
	g_print("7 : SCMIRRORING_RESOLUTION_640x360_P30   [%d]\n", SCMIRRORING_RESOLUTION_640x360_P30);
	g_print("r : Reset resolution \n");
	g_print("g : Go back to main menu \n");
	g_print("**********************************************************************\n");

}

static void __interpret_resolution_submenu(char *cmd)
{
	if (strncmp(cmd, "1", 1) == 0) {
		g_print("resolution |= SCMIRRORING_RESOLUTION_1920x1080_P30[%d]\n", SCMIRRORING_RESOLUTION_1920x1080_P30);
		g_resolution |= SCMIRRORING_RESOLUTION_1920x1080_P30;
	} else if (strncmp(cmd, "2", 1) == 0) {
		g_print("resolution |= SCMIRRORING_RESOLUTION_1280x720_P30[%d]\n", SCMIRRORING_RESOLUTION_1280x720_P30);
		g_resolution |= SCMIRRORING_RESOLUTION_1280x720_P30;
	} else if (strncmp(cmd, "3", 1) == 0) {
		g_print("resolution |= SCMIRRORING_RESOLUTION_960x540_P30[%d]\n", SCMIRRORING_RESOLUTION_960x540_P30);
		g_resolution |= SCMIRRORING_RESOLUTION_960x540_P30;
	} else if (strncmp(cmd, "4", 1) == 0) {
		g_print("resolution |= SCMIRRORING_RESOLUTION_864x480_P30[%d]\n", SCMIRRORING_RESOLUTION_864x480_P30);
		g_resolution |= SCMIRRORING_RESOLUTION_864x480_P30;
	} else if (strncmp(cmd, "5", 1) == 0) {
		g_print("resolution |= SCMIRRORING_RESOLUTION_720x480_P60[%d]\n", SCMIRRORING_RESOLUTION_720x480_P60);
		g_resolution |= SCMIRRORING_RESOLUTION_720x480_P60;
	} else if (strncmp(cmd, "6", 1) == 0) {
		g_print("resolution |= SCMIRRORING_RESOLUTION_640x480_P60[%d]\n", SCMIRRORING_RESOLUTION_640x480_P60);
		g_resolution |= SCMIRRORING_RESOLUTION_640x480_P60;
	} else if (strncmp(cmd, "7", 1) == 0) {
		g_print("resolution |= SCMIRRORING_RESOLUTION_640x360_P30[%d]\n", SCMIRRORING_RESOLUTION_640x360_P30);
		g_resolution |= SCMIRRORING_RESOLUTION_640x360_P30;
	} else if (strncmp(cmd, "r", 1) == 0) {
		g_resolution = 0;
	} else if (strncmp(cmd, "g", 1) == 0) {
		g_print("go back to main menu\n");
		g_menu = MAIN_MENU;
		g_timeout_add(100, __timeout_menu_display, 0);
		return;
	}

	g_print("resolution : %d\n", g_resolution);

	g_timeout_add(100, __timeout_resolution_submenu_display, 0);

	return;
}

static void scmirroring_sink_state_callback(scmirroring_error_e error_code, scmirroring_sink_state_e state, void *user_data)
{
	g_print("Received Callback error code[%d] state[%d]", error_code, state);

	if (state == SCMIRRORING_SINK_STATE_NONE)
		g_print(" state[%d] SCMIRRORING_SINK_STATE_NONE", state);
	else if (state == SCMIRRORING_SINK_STATE_NULL)
		g_print(" st ate[%d] (state == SCMIRRORING_SINK_STATE_NULL)", state);
	else if (state == SCMIRRORING_SINK_STATE_PREPARED)
		g_print(" state[%d] SCMIRRORING_SINK_STATE_PREPARED", state);
	else if (state == SCMIRRORING_SINK_STATE_CONNECTED) {
		g_print(" state[%d] SCMIRRORING_SINK_STATE_CONNECTED", state);
		if (scmirroring_sink_start(g_scmirroring) != SCMIRRORING_ERROR_NONE)
			g_print("scmirroring_sink_start fail");
	} else if (state == SCMIRRORING_SINK_STATE_PLAYING)
		g_print(" state[%d] SCMIRRORING_SINK_STATE_PLAYING", state);
	else if (state == SCMIRRORING_SINK_STATE_PAUSED)
		g_print(" state[%d] SCMIRRORING_SINK_STATE_PAUSED", state);
	else if (state == SCMIRRORING_SINK_STATE_DISCONNECTED) {
		g_print(" state[%d] SCMIRRORING_SINK_STATE_DISCONNECTED", state);
		if (scmirroring_sink_unprepare(g_scmirroring) != SCMIRRORING_ERROR_NONE) {
			g_print("scmirroring_sink_unprepare fail");
		}
		if (scmirroring_sink_destroy(g_scmirroring) != SCMIRRORING_ERROR_NONE) {
			g_print("scmirroring_sink_destroy fail");
		}
		__quit_program();
	} else
		g_print(" state[%d] Invalid State", state);

	return;
}

static void __quit_program(void)
{
	g_print("Quit Program\n");

#ifdef TEST_WITH_WIFI_DIRECT
	__disconnect_p2p_connection();
#endif
	g_scmirroring = 0;
	g_main_loop_quit(g_loop);
}

static void __displaymenu(void)
{
	g_print("\n");
	g_print("=====================================================================\n");
	g_print("                          SCMIRRORING Sink Testsuite(press q to quit) \n");
	g_print("=====================================================================\n");
#ifndef TEST_WITH_WIFI_DIRECT
	g_print("a : a ip port(ex. a 192.168.49.1 2022)\n");
	g_print("s : start  \n");
#endif
	g_print("P : Pause\n");
	g_print("R : Resume\n");
	g_print("D : Disconnect\n");
	g_print("T : desTroy\n");
	g_print("L : Setting resolution\n");
	g_print("q : quit\n");
	g_print("-----------------------------------------------------------------------------------------\n");
}

gboolean __timeout_menu_display(void *data)
{
	__displaymenu();

	return FALSE;
}

#ifdef TEST_WITH_WIFI_DIRECT
bool _connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *user_data)
{
	int peer_port = 0;
	if (wifi_direct_get_peer_display_port(peer->mac_address, &peer_port) != WIFI_DIRECT_ERROR_NONE) {
		g_print("Can not get port info\n Use default(2022)\n");
		peer_port = DEFAULT_SCREEN_MIRRORING_PORT;
	}
	if (peer_port == 0) {
		g_print("Can not get port info\n Use default(2022)\n");
		peer_port = DEFAULT_SCREEN_MIRRORING_PORT;
	}

	g_print("[_connected_peer_cb] Connected to IP [%s]\n", peer->ip_address);
	g_print("[_connected_peer_cb] Connected to Port [%d]\n", peer_port);
	g_print("[_connected_peer_cb] Connected device_name [%s]\n", peer->device_name);
	g_print("[_connected_peer_cb] Connected to mac_address [%s]\n", peer->mac_address);
	g_print("[_connected_peer_cb] Connected to interface_address [%s]\n", peer->interface_address);

	memset(g_peer_ip, 0x00, sizeof(g_peer_ip));
	memset(g_peer_port, 0x00, sizeof(g_peer_port));

	snprintf(g_peer_ip, sizeof(g_peer_port), "%s", peer->ip_address);
	snprintf(g_peer_port, sizeof(g_peer_port), "%d", peer_port);

	g_timeout_add(SINKTEST_EXECUTE_DELAY, __scmirroring_sink_start, NULL);

	return TRUE;
}

void _activation_cb(int error_code, wifi_direct_device_state_e device_state, void *user_data)
{
	switch (device_state) {
		case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
			g_print("device_state : WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
			break;
		case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
			g_print("device_state : WIFI_DIRECT_DEVICE_STATE_DEACTIVATED\n");
			break;
		default:
			g_print("device_state : ERROR\n");
			break;
	}

	return;
}

bool _discovered_peer_cb(wifi_direct_discovered_peer_info_s *peer, void *user_data)
{
	g_print("[%d] discovered device peer : %s, %s, %d\n", g_peer_cnt, peer->device_name, peer->mac_address, peer->is_connected);

	g_peer_cnt++;

	return TRUE;
}

void _discover_cb(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data)
{
	int ret = WIFI_DIRECT_ERROR_NONE;
	/*g_print("Discovered [ error : %d discovery state : %d ]\n", error_code, discovery_state); */

	switch (discovery_state) {
		case WIFI_DIRECT_ONLY_LISTEN_STARTED:
			g_print("discovery_state : WIFI_DIRECT_ONLY_LISTEN_STARTED \n");
			break;
		case WIFI_DIRECT_DISCOVERY_STARTED:
			g_print("discovery_state : WIFI_DIRECT_DISCOVERY_STARTED \n");
			break;
		case WIFI_DIRECT_DISCOVERY_FOUND:
			g_print("discovery_state : WIFI_DIRECT_DISCOVERY_FOUND \n");
			ret = wifi_direct_foreach_discovered_peers(_discovered_peer_cb, (void *)NULL);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				g_print("Error : wifi_direct_foreach_discovered_peers failed : %d\n", ret);
			}
			break;
		case WIFI_DIRECT_DISCOVERY_FINISHED:
			g_print("discovery_state : WIFI_DIRECT_DISCOVERY_FINISHED \n");
			break;
		default:
			g_print("discovery_state : ERROR\n");
			break;
	}

	return;
}

void _ip_assigned_cb(const char *mac_address, const char *ip_address, const char *interface_address, void *user_data)
{
	g_print("[_ip_assigned_cb] IP assigned [ ip addr : %s if addr : %s mac_addr:%s ]\n", ip_address, interface_address, mac_address);
}

void _connection_cb(int error_code, wifi_direct_connection_state_e connection_state, const char *mac_address, void *user_data)
{
	int ret = WIFI_DIRECT_ERROR_NONE;

	g_print("Connected [ error : %d connection state : %d mac_addr:%s ]\n", error_code, connection_state, mac_address);

	if (connection_state == WIFI_DIRECT_CONNECTION_REQ) {
		ret = wifi_direct_accept_connection(mac_address);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			g_print("Error : wifi_direct_accept_connection failed : %d\n", ret);
		}
	} else if (connection_state == WIFI_DIRECT_CONNECTION_RSP) {
		bool is_go = FALSE;
		ret = wifi_direct_is_group_owner(&is_go);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			g_print("Error : wifi_direct_is_group_owner failed : %d\n", ret);
		}

		if (is_go) {
			g_print("Connected as Group Owner\n");
		} else {
			ret = wifi_direct_foreach_connected_peers(_connected_peer_cb, (void *)NULL);
			if (ret != WIFI_DIRECT_ERROR_NONE) {
				g_print("Error : wifi_direct_foreach_connected_peers failed : %d\n", ret);
				return;
			}
			g_print("Connected as Group Client\n");
		}
	}

	return;
}
#endif

static void __interpret(char *cmd)
{
	int ret = SCMIRRORING_ERROR_NONE;
	gchar **value;
	value = g_strsplit(cmd, " ", 0);
	if (strncmp(cmd, "D", 1) == 0) {
		g_print("Disconnect\n");
		ret = scmirroring_sink_disconnect(g_scmirroring);
	} else if (strncmp(cmd, "P", 1) == 0) {
		g_print("Pause\n");
		ret = scmirroring_sink_pause(g_scmirroring);
	} else if (strncmp(cmd, "R", 1) == 0) {
		g_print("Resume\n");
		ret = scmirroring_sink_resume(g_scmirroring);
	} else if (strncmp(cmd, "T", 1) == 0) {
		g_print("Destroy\n");
		ret = scmirroring_sink_unprepare(g_scmirroring);
		ret = scmirroring_sink_destroy(g_scmirroring);
	} else if (strncmp(cmd, "q", 1) == 0) {
		__quit_program();
	} else if (strncmp(cmd, "L", 1) == 0) {
		g_menu = SUBMENU_RESOLUTION;
		g_timeout_add(100, __timeout_resolution_submenu_display, 0);
		return;
	}
#ifndef TEST_WITH_WIFI_DIRECT
	else if (strncmp(cmd, "a", 1) == 0) {
		ret = __scmirroring_sink_create(NULL);
		if (ret == SCMIRRORING_ERROR_NONE) {
			ret = scmirroring_sink_set_ip_and_port(g_scmirroring, value[1], value[2]);
			g_print("Input server IP and port number IP[%s] Port[%s]\n", value[1], value[2]);
		}
	} else if (strncmp(cmd, "s", 1) == 0) {
		g_print("Start\n");
		ret = __scmirroring_sink_start(NULL);
	}
#endif
	else {
		g_print("unknown menu \n");
	}

	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("Error Occured [%d]", ret);
	}

	g_timeout_add(100, __timeout_menu_display, 0);

	return;
}

gboolean __input(GIOChannel *channel)
{
	char buf[MAX_STRING_LEN + 3];
	gsize read;
	GError *error = NULL;

	g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);
	buf[read] = '\0';
	g_strstrip(buf);

	if (g_menu == MAIN_MENU)
		__interpret(buf);
	else if (g_menu == SUBMENU_RESOLUTION)
		__interpret_resolution_submenu(buf);

	return TRUE;
}

#ifdef TEST_WITH_WIFI_DIRECT
static gboolean __start_p2p_connection(gpointer data)
{
	int ret = WIFI_DIRECT_ERROR_NONE;
	int go_intent = 0;
	wifi_direct_state_e direct_state = WIFI_DIRECT_STATE_DEACTIVATED;

	g_print("====== Start p2p connection ======\n");

	ret = wifi_direct_initialize();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_initialize failed : %d\n", ret);
		return FALSE;
	}

	struct ug_data *ugd = (struct ug_data *)data;

	/* Activation / Deactivation state Callback */
	ret =  wifi_direct_set_device_state_changed_cb(_activation_cb, (void *)ugd);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_set_device_state_changed_cb failed : %d\n", ret);
		goto error;
	}

	/* Discovery state Callback */
	ret =  wifi_direct_set_discovery_state_changed_cb(_discover_cb, (void *)ugd);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_set_discovery_state_changed_cb failed : %d\n", ret);
		goto error;
	}

	/* Connection state Callback */
	ret =  wifi_direct_set_connection_state_changed_cb(_connection_cb, (void *)ugd);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_set_connection_state_changed_cb failed : %d\n", ret);
		goto error;
	}

	/* IP address assigning state callback */
	ret =  wifi_direct_set_client_ip_address_assigned_cb(_ip_assigned_cb, (void *)ugd);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_set_client_ip_address_assigned_cb failed : %d\n", ret);
		goto error;
	}

	ret = wifi_direct_get_state(&direct_state);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_get_state failed : %d\n", ret);
		goto error;
	}

	if (direct_state < WIFI_DIRECT_STATE_ACTIVATED) {
		g_print("wifi direct status < WIFI_DIRECT_STATE_ACTIVATED\n");
		g_print("\n------Starting to activate scmirroring------\n");
		ret = wifi_direct_activate();
		if (ret < WIFI_DIRECT_ERROR_NONE) {
			g_print("Error : wifi_direct_activate failed : %d\n", ret);
			return FALSE;
		}
	} else {
		g_print("wifi direct status >= WIFI_DIRECT_STATE_ACTIVATED.. Disconnect all first\n");
		ret = wifi_direct_disconnect_all();
		if (!ret) {
			g_print("wifi_direct_disconnect_all success\n");
		} else {
			g_print("wifi_direct_disconnect_all fail\n");
		}
	}

	/*Enable Screen Mirroring*/
	ret = wifi_direct_init_display();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_display_init failed : %d\n", ret);
		goto error;
	}

	/*Enable Wifi Direct - You can set this as true if you want to see it from wifi-direct list*/
	ret = wifi_direct_set_display_availability(TRUE);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_display_init failed : %d\n", ret);
		goto error;
	}

	ret = wifi_direct_set_display(WIFI_DISPLAY_TYPE_SINK, 2022, 0);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_display_set_device failed : %d\n", ret);
		goto error;
	}

	ret = wifi_direct_get_group_owner_intent(&go_intent);
	g_print("go_intent = [%d]\n", go_intent);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_get_group_owner_intent failed : %d\n", ret);
		goto error;
	}

	go_intent = 1;
	ret = wifi_direct_set_group_owner_intent(go_intent);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_get_group_owner_intent failed : %d\n", ret);
		goto error;
	}
	g_print("wifi_direct_set_group_owner_intent() result=[%d] go_intent[%d]\n", ret, go_intent);

	ret = wifi_direct_set_max_clients(1);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_set_max_clients failed : %d\n", ret);
		goto error;
	}

	ret = wifi_direct_get_state(&direct_state);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_get_state failed : %d\n", ret);
		goto error;
	}

	if (direct_state > WIFI_DIRECT_STATE_ACTIVATING) {
		char *device_name = NULL;

		ret = wifi_direct_start_discovery(1, 0);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			g_print("Error : wifi_direct_start_discovery failed : %d\n", ret);
			goto error;
		}

		ret = wifi_direct_get_device_name(&device_name);
		if (ret != WIFI_DIRECT_ERROR_NONE) {
			g_print("Error : wifi_direct_get_device_name failed : %d\n", ret);
			goto error;
		}

		g_print("Device Name : [%s]\n", device_name);
		if (device_name)
			free(device_name);

	} else {
		g_print("Error : Direct not activated yet\n");
	}

	g_print("======  p2p connection established ======\n");

	return TRUE;

error:
	ret = wifi_direct_deinitialize();

	return FALSE;
}

static gboolean __disconnect_p2p_connection(void)
{
	int ret = WIFI_DIRECT_ERROR_NONE;

	ret = wifi_direct_deactivate();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_deactivate failed : %d\n", ret);
		return FALSE;
	}

	ret = wifi_direct_deinitialize();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_deinitialize failed : %d\n", ret);
		return FALSE;
	}

	g_print("------p2p connection disconnected------\n");

	return TRUE;
}
#endif

#ifndef TEST_WITH_WIFI_DIRECT
static int __scmirroring_sink_create(gpointer data)
{
	int ret = SCMIRRORING_ERROR_NONE;

	ret = scmirroring_sink_create(&g_scmirroring);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_create fail [%d]", ret);
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	ret = scmirroring_sink_prepare(g_scmirroring);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_prepare fail [%d]", ret);
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}
	return ret;
}
#endif

gboolean __scmirroring_sink_start(gpointer data)
{
	int ret = SCMIRRORING_ERROR_NONE;

#ifdef TEST_WITH_WIFI_DIRECT
	ret = scmirroring_sink_create(&g_scmirroring);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_create fail [%d]", ret);
		return FALSE;
	}

	if (g_resolution != 0) {
		ret = scmirroring_sink_set_resolution(g_scmirroring, g_resolution);
		if (ret != SCMIRRORING_ERROR_NONE) {
			g_print("Failed to set resolution, error[%d]\n", ret);
		}
	}

	ret = scmirroring_sink_prepare(g_scmirroring);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_prepare fail [%d]", ret);
		return SCMIRRORING_ERROR_OUT_OF_MEMORY;
	}

	ret = scmirroring_sink_set_ip_and_port(g_scmirroring, g_peer_ip, g_peer_port);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_set_ip_and_port fail [%d]", ret);
		return FALSE;
	}
#endif

	ret = scmirroring_sink_set_state_changed_cb(g_scmirroring, scmirroring_sink_state_callback, NULL);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_set_state_changed_cb fail [%d]", ret);
		return FALSE;
	}

	ret = scmirroring_sink_connect(g_scmirroring);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_connect fail [%d]", ret);
		return FALSE;
	}

#if 0
	ret = scmirroring_sink_set_display(g_scmirroring, SCMIRRORING_DISPLAY_TYPE_X11, NULL);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_sink_set_display fail [%d]", ret);
		return FALSE;
	}
#endif
	return FALSE;
}

gboolean _scmirroring_start_jobs(gpointer data)
{
#ifdef TEST_WITH_WIFI_DIRECT
	int ret = WIFI_DIRECT_ERROR_NONE;

	ret = __start_p2p_connection(data);
	if (ret == FALSE)
		return FALSE;
#endif

	return FALSE;
}

int main(int argc, char *argv[])
{
	GIOChannel *stdin_channel;
	GMainContext *context = NULL;
	GSource *start_job_src = NULL;

	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags(stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)__input, NULL);

	__displaymenu();

	g_loop = g_main_loop_new(NULL, FALSE);
	context = g_main_loop_get_context(g_loop);

	start_job_src = g_idle_source_new();
	g_source_set_callback(start_job_src, _scmirroring_start_jobs, NULL, NULL);
	g_source_attach(start_job_src, context);

	g_main_loop_run(g_loop);

	g_print("Exit Program");

	return 0;
}
