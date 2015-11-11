/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
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

#include <bluetooth.h>

#include "vp-play-macro-define.h"

#include "vp-avrcp.h"

/* check temp */
#include "vp-play-log.h"


/* callback functions */

/* internal functions */
void _vp_avrcp_connection_state_changed_cb(bool connected,
        const char *remote_address,
        void *user_data)
{
	VideoLogInfo("");
}

/* external functions */
bool vp_avrcp_initialize()
{
	VideoLogInfo("vp_avrcp_initialize start");

	int nRet = bt_initialize();
	if (nRet != BT_ERROR_NONE) {
		VideoLogError("bt_initialize fail. nRet[%d]", nRet);
		return FALSE;
	}

	nRet =
	    bt_avrcp_target_initialize(_vp_avrcp_connection_state_changed_cb,
	                               NULL);
	if (nRet != BT_ERROR_NONE) {
		VideoLogError("bt_avrcp_target_initialize fail. nRet[%d]", nRet);
		bt_deinitialize();
		return FALSE;
	}

	VideoLogInfo("vp_avrcp_initialize end");

	return TRUE;
}

bool vp_avrcp_deinitialize()
{
	VideoLogInfo("vp_avrcp_deinitialize start");

	int nRet = BT_ERROR_NONE;

	nRet = bt_avrcp_target_deinitialize();
	if (nRet != BT_ERROR_NONE) {
		VideoLogError("bt_avrcp_target_deinitialize fail. nRet[%d]",
		              nRet);
		return FALSE;
	}

	nRet = bt_deinitialize();
	if (nRet != BT_ERROR_NONE) {
		VideoLogError("bt_deinitialize fail. nRet[%d]", nRet);
		return FALSE;
	}


	VideoLogInfo("vp_avrcp_deinitialize end");

	return TRUE;
}

bool vp_avrcp_noti_track(const char *title, const char *artist,
                         const char *album, const char *genre,
                         unsigned int duration)
{
	VideoLogInfo("vp_avrcp_noti_track start");

	int nRet = BT_ERROR_NONE;
	VideoLogWarning("set avrcp noti tack. title[%s]", title);
	VideoLogWarning("set avrcp noti tack. duration[%d]", duration);

	nRet =
	    bt_avrcp_target_notify_track(title, artist, album, genre, 1, 1,
	                                 duration);
	if (nRet != BT_ERROR_NONE) {
		VideoLogError("bt_avrcp_target_notify_track fail. nRet[%d]",
		              nRet);
		return FALSE;
	}

	VideoLogInfo("vp_avrcp_noti_track end");

	return TRUE;
}

bool vp_avrcp_noti_track_position(unsigned int position)
{
	VideoLogInfo("vp_avrcp_noti_track_position start");

	int nRet = BT_ERROR_NONE;
	VideoLogWarning("set avrcp noti track position. duration[%d]",
	                position);
	nRet = bt_avrcp_target_notify_position(position);
	if (nRet != BT_ERROR_NONE) {
		VideoLogError("bt_avrcp_target_notify_position fail. nRet[%d]",
		              nRet);
		return FALSE;
	}

	VideoLogInfo("vp_avrcp_noti_track_position end");

	return TRUE;
}

bool vp_avrcp_noti_player_state(vp_mm_player_state_t nState)
{
	VideoLogInfo("vp_avrcp_noti_player_state start");
	if (nState == VP_MM_PLAYER_STATE_NONE
	        || nState == VP_MM_PLAYER_STATE_IDLE) {
		VideoLogWarning
		("player state : [%d]. don't need to send player state",
		 nState);
		return TRUE;
	}

	bt_avrcp_player_state_e player_state = BT_AVRCP_PLAYER_STATE_STOPPED;
	switch (nState) {
	case VP_MM_PLAYER_STATE_PLAYING:
		player_state = BT_AVRCP_PLAYER_STATE_PLAYING;
		break;
	case VP_MM_PLAYER_STATE_READY:
	case VP_MM_PLAYER_STATE_PAUSED:
		player_state = BT_AVRCP_PLAYER_STATE_PAUSED;
		break;
	case VP_MM_PLAYER_STATE_STOP:
		player_state = BT_AVRCP_PLAYER_STATE_STOPPED;
		break;
	default:
		break;
	}
	VideoLogWarning("set avrcp noti player state. player_state[%d]",
	                player_state);

	int nRet = BT_ERROR_NONE;
	nRet = bt_avrcp_target_notify_player_state(player_state);
	if (nRet != BT_ERROR_NONE) {
		VideoLogError
		("bt_avrcp_target_notify_player_state fail. nRet[%d]", nRet);
		return FALSE;
	}

	VideoLogInfo("vp_avrcp_noti_player_state end");

	return TRUE;
}
