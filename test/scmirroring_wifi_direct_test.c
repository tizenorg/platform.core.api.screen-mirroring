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
#include <wifi-direct.h>

#define MAX_STRING_LEN    2048

scmirroring_src_h g_scmirroring = NULL;
GMainLoop *g_loop;

bool _connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *user_data);
void _activation_cb(int error_code, wifi_direct_device_state_e device_state, void *user_data);
bool _discovered_peer_cb(wifi_direct_discovered_peer_info_s *peer, void *user_data);
void _discover_cb(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data);
void _ip_assigned_cb(const char *mac_address, const char *ip_address, const char *interface_address, void *user_data);
void _connection_cb(int error_code, wifi_direct_connection_state_e connection_state, const char *mac_address, void *user_data);
static gboolean __start_wifi_display_connection();
static gboolean __start_p2p_connection(gpointer data);
static gboolean __disconnect_p2p_connection(void);
static int g_peer_cnt = 0;
static char g_peer_ip[32];
static char g_peer_port[32];
static char g_sink_mac_addr[18] = {0, };

static void scmirroring_state_callback(scmirroring_error_e error_code, scmirroring_state_e state, void *user_data)
{
	g_print("\n\nReceived Callback error code[%d], state[%d]\n\n", error_code, state);

	return;
}

static void __quit_program(void)
{
	g_print("Quit Program\n");

	__disconnect_p2p_connection();

	g_scmirroring = 0;
	g_main_loop_quit(g_loop);
}

static void __displaymenu(void)
{
	g_print("\n");
	g_print("=====================================================================\n");
	g_print("                          SCMIRRORING Testsuite(press q to quit) \n");
	g_print("=====================================================================\n");
	g_print("a : a ip port(ex. a 192.168.49.1 2022)\n");
	g_print("b : set sink device mac address(ex. b f8:d0:bd:7f:e9:7c)\n");
	g_print("c : set resolution(ex. c 1 (1 : 1920x1080_P30, 2 : 1280x720_P30, 3 : (1 +2), 4. 960x540_P30, 64: 640x360_P30, 67: 1+2+64, 71: 1+2+4+64)\n");
	g_print("f : set connection mode(ex. f 0 (0 : wifi_direct, 1 : Other)\n");
	g_print("g : set multisink mode(ex. g 1 (0 : disable, 1 : enable)\n");
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

gboolean __timeout_menu_display(void *data)
{
	__displaymenu();
	return FALSE;
}

static int __wifi_direct_device_connect()
{
	if (strlen(g_sink_mac_addr) > 17 || strlen(g_sink_mac_addr) <= 0) {
		g_print("\nWrong Mac_address");
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}

	int err =  wifi_direct_connect(g_sink_mac_addr);
	if (err != WIFI_DIRECT_ERROR_NONE) {
		g_print("Failed to connect  [%d]\n", err);
		return SCMIRRORING_ERROR_INVALID_OPERATION;
	}
	return SCMIRRORING_ERROR_NONE;
}

static void __interpret(char *cmd)
{
	int ret = SCMIRRORING_ERROR_NONE;
	gchar **value;
	value = g_strsplit(cmd, " ", 0);

	if (strncmp(cmd, "a", 1) == 0) {
		ret = scmirroring_src_set_ip_and_port(g_scmirroring, value[1], value[2]);
		g_print("Input server IP and port number IP[%s] Port[%s]\n", value[1], value[2]);
	}
	if (strncmp(cmd, "b", 1) == 0) {
		strncpy(g_sink_mac_addr, value[1], sizeof(g_sink_mac_addr));
		g_sink_mac_addr[17] = '\0';
		g_print("Sink mac address : %s\n", g_sink_mac_addr);
		ret = __wifi_direct_device_connect();
	} else if (strncmp(cmd, "c", 1) == 0) {
		ret = scmirroring_src_set_resolution(g_scmirroring, atoi(value[1]));
		g_print("Set Resolution[%d]\n", atoi(value[1]));
	} else if (strncmp(cmd, "f", 1) == 0) {
		ret = scmirroring_src_set_connection_mode(g_scmirroring, atoi(value[1]));
		g_print("Connection mode [%d]\n", atoi(value[1]));
	} else if (strncmp(cmd, "g", 1) == 0) {
		ret = scmirroring_src_set_multisink_ability(g_scmirroring, atoi(value[1]));
		g_print("Multisink mode [%d]\n", atoi(value[1]));
	} else if (strncmp(cmd, "C", 1) == 0) {
		g_print("Connect\n");
		ret = scmirroring_src_connect(g_scmirroring);
	} else if (strncmp(cmd, "I", 1) == 0) {
		g_print("dIsconnect\n");
		ret = scmirroring_src_disconnect(g_scmirroring);
	} else if (strncmp(cmd, "S", 1) == 0) {
		g_print("Start\n");
		ret = scmirroring_src_start(g_scmirroring);
	} else if (strncmp(cmd, "P", 1) == 0) {
		g_print("Pause\n");
		ret = scmirroring_src_pause(g_scmirroring);
	} else if (strncmp(cmd, "R", 1) == 0) {
		g_print("Resume\n");
		ret = scmirroring_src_resume(g_scmirroring);
	} else if (strncmp(cmd, "T", 1) == 0) {
		g_print("Stop\n");
		ret = scmirroring_src_stop(g_scmirroring);
		g_print("Stop end\n");
	} else if (strncmp(cmd, "D", 1) == 0) {
		g_print("Destroy\n");
		ret = scmirroring_src_destroy(g_scmirroring);
	} else if (strncmp(cmd, "q", 1) == 0) {
		__quit_program();
	} else {
		g_print("unknown menu \n");
	}

	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("Error Occured [%d]\n", ret);
	}

	g_timeout_add(100, __timeout_menu_display, 0);

	g_print("Return interpret\n");
	return;
}

gboolean __input(GIOChannel *channel)
{
	char buf[MAX_STRING_LEN + 3];
	gsize read;
	GError *error = NULL;

	g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);
	buf[read] = '\0';
	g_print("Read %"G_GSSIZE_FORMAT" bytes: %s\n", read, buf);
	g_strstrip(buf);
	__interpret(buf);
	return TRUE;
}


static gboolean __disconnect_p2p_connection(void)
{
	int ret = WIFI_DIRECT_ERROR_NONE;

	ret = wifi_direct_disconnect_all();
	if (!ret) {
		g_print("wifi_direct_disconnect_all success\n");
	} else {
		g_print("wifi_direct_disconnect_all fail\n");
	}

	g_print("------p2p connection disconnected------\n");

	return TRUE;
}

bool _connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *user_data)
{
	g_print("[_connected_peer_cb] Connected to IP [%s]\n", peer->ip_address);
	/*g_print("[_connected_peer_cb] Connected to Port [%d]\n", peer->scmirroring_ctrl_port); */
	g_print("[_connected_peer_cb] Connected device_name [%s]\n", peer->device_name);
	g_print("[_connected_peer_cb] Connected to mac_address [%s]\n", peer->mac_address);
	g_print("[_connected_peer_cb] Connected to interface_address [%s]\n", peer->interface_address);

	memset(g_peer_ip, 0x00, sizeof(g_peer_ip));
	memset(g_peer_port, 0x00, sizeof(g_peer_port));

	snprintf(g_peer_ip, sizeof(g_peer_ip), "%s", peer->ip_address);
	/*snprintf(g_peer_port, sizeof(g_peer_port), "%d", peer->scmirroring_ctrl_port); */

	/*g_timeout_add(SINKTEST_EXECUTE_DELAY, __scmirroring_sink_start, NULL); */

	return TRUE;
}

void _activation_cb(int error_code, wifi_direct_device_state_e device_state, void *user_data)
{
	int ret = 0;

	switch (device_state) {
		case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
			g_print("device_state : WIFI_DIRECT_DEVICE_STATE_ACTIVATED\n");
			ret = __start_wifi_display_connection();
			if (ret == TRUE) {
				g_print("__start_wifi_display_connection success\n");
			} else {
				g_print("__start_wifi_display_connection fail\n");
				g_print("Quit Program\n");
				ret = wifi_direct_deinitialize();
				if (ret != WIFI_DIRECT_ERROR_NONE)
					g_print("wifi_direct_deinitialize is failed\n");

				g_scmirroring = 0;
				g_main_loop_quit(g_loop);
			}
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
	char *local_ip = NULL;

	wifi_direct_get_ip_address(&local_ip);

	if (!local_ip) {
		g_print("Failed to get local ip\n");
		return;
	}

	g_print("[_ip_assigned_cb] IP assigned [ ip addr : %s if addr : %s mac_addr:%s ]\n", ip_address, interface_address, mac_address);

	scmirroring_src_set_ip_and_port(g_scmirroring, local_ip, "2022");
	g_print("Input server IP and port number IP[%s] Port[%s]\n", local_ip, "2022");
	g_free(local_ip);
}

void _connection_cb(int error_code, wifi_direct_connection_state_e connection_state, const char *mac_address, void *user_data)
{
	g_print("Connected [ error : %d connection state : %d mac_addr:%s ]\n", error_code, connection_state, mac_address);

	switch (connection_state) {
		case WIFI_DIRECT_CONNECTION_IN_PROGRESS: {
				g_print(" WIFI_DIRECT_CONNECTION_IN_PROGRESS\n");
				break;
			}
		case WIFI_DIRECT_CONNECTION_RSP: {
				g_print(" WIFI_DIRECT_CONNECTION_RSP\n");
				bool is_go = FALSE;
				wifi_direct_is_group_owner(&is_go);
				if (!is_go) {
					wifi_direct_foreach_connected_peers(_connected_peer_cb, (void *)NULL);
					g_print("WIFI_DIRECT_CONNECTION_RSP : Source is not group owner\n");
				} else {
					g_print("WIFI_DIRECT_CONNECTION_RSP : Source is group owner\n ");
				}
				break;
			}
		case WIFI_DIRECT_CONNECTION_REQ:
		case WIFI_DIRECT_CONNECTION_WPS_REQ:
		case WIFI_DIRECT_DISCONNECTION_RSP:
		case WIFI_DIRECT_DISCONNECTION_IND:
		case WIFI_DIRECT_GROUP_CREATED:
		case WIFI_DIRECT_GROUP_DESTROYED:
		case WIFI_DIRECT_DISASSOCIATION_IND:
			break;
		default:
			break;
	}

	return;
}

static gboolean __start_wifi_display_connection()
{
	int go_intent = 0;
	static int is_initialized = FALSE;
	gint ret = FALSE;

	if (is_initialized == TRUE)
		return TRUE;
	is_initialized = TRUE;

	/*Enable Screen Mirroring*/
	ret = wifi_direct_init_display();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_init_display failed : %d\n", ret);
		return FALSE;
	}

	ret = wifi_direct_set_display_availability(TRUE);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_set_display_availability failed : %d\n", ret);
		return FALSE;
	}

	ret = wifi_direct_set_display(WIFI_DISPLAY_TYPE_SRC, 2022, 0);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_display_set_device failed : %d\n", ret);
		return FALSE;
	}

	ret = wifi_direct_get_group_owner_intent(&go_intent);
	g_print("go_intent = [%d]\n", go_intent);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_get_group_owner_intent failed : %d\n", ret);
		return FALSE;
	}

	go_intent = 14;
	ret = wifi_direct_set_group_owner_intent(go_intent);
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_get_group_owner_intent failed : %d\n", ret);
		return FALSE;
	}
	g_print("wifi_direct_set_group_owner_intent() result=[%d] go_intent[%d]\n", ret, go_intent);

	/* 10 sec discovery in cycle mode */
	int err =  wifi_direct_start_discovery(0, 10);
	if (err < WIFI_DIRECT_ERROR_NONE) {
		g_print("wifi_direct_discovery fail\n");
		return FALSE;
	} else {
		g_print("wifi_direct_discovery start\n");
	}
	g_print("======  p2p connection established ======\n");

	return TRUE;
}

static gboolean __start_p2p_connection(gpointer data)
{
	int ret = WIFI_DIRECT_ERROR_NONE;
	wifi_direct_state_e direct_state = WIFI_DIRECT_STATE_DEACTIVATED;
	struct ug_data *ugd = (struct ug_data *)data;

	g_print("====== Start p2p connection ======\n");

	ret = wifi_direct_initialize();
	if (ret != WIFI_DIRECT_ERROR_NONE) {
		g_print("Error : wifi_direct_initialize failed : %d\n", ret);
		return FALSE;
	}

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

		ret = __start_wifi_display_connection();
		if (ret == TRUE) {
			g_print("__start_wifi_display_connection success\n");
		} else {
			g_print("__start_wifi_display_connection fail\n");
			goto error;
		}
	}

	return TRUE;

error:
	ret = wifi_direct_deinitialize();

	return FALSE;
}

gboolean _scmirroring_start_jobs(gpointer data)
{
	__start_p2p_connection(data);
	return FALSE;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	GIOChannel *stdin_channel;
	GMainContext *context = NULL;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags(stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)__input, NULL);

	ret = scmirroring_src_create(&g_scmirroring);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_src_create fail [%d]", ret);
		return 0;
	}

	ret = scmirroring_src_set_state_changed_cb(g_scmirroring, scmirroring_state_callback, NULL);
	if (ret != SCMIRRORING_ERROR_NONE) {
		g_print("scmirroring_src_create fail [%d]", ret);
		return 0;
	}

	__displaymenu();

	g_loop = g_main_loop_new(NULL, FALSE);
	context = g_main_loop_get_context(g_loop);

	GSource *start_job_src = NULL;
	start_job_src = g_idle_source_new();
	g_source_set_callback(start_job_src, _scmirroring_start_jobs, NULL, NULL);
	g_source_attach(start_job_src, context);

	g_main_loop_run(g_loop);

	return ret;
}
