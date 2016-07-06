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
#include <system_settings.h>
#include <runtime_info.h>
#include <app_preference.h>
#include <telephony.h>
#include <wifi-direct.h>
#include <device/display.h>
#include "vp-play-config.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-play-type-define.h"

#define PREF_VP_VIDEO_PREVIEW_URL_VIDEOS "preference/org.tizen.videos/preview_url_videos"
#define PREF_VP_SORT_TYPE  "preference/org.tizen.videos/sort_type"
#define PREF_VP_MULTI_PLAY_FLAG  "preference/org.tizen.videos/multi_play"


bool vp_play_config_set_multi_play_status(bool bMultiPlay)
{
	int nErr = 0;

	nErr = preference_set_boolean(PREF_VP_MULTI_PLAY_FLAG, bMultiPlay);
	if (nErr != 0) {
		VideoLogError("failed to set MULTI PLAY FLAG [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_config_get_multi_play_status(bool *bMultiPlay)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(PREF_VP_MULTI_PLAY_FLAG, &nVal);
	if (nErr != 0) {
		VideoLogError("failed to get MULTI PLAY FLAG [0x%x]", nErr);
		return FALSE;
	}

	*bMultiPlay = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_config_get_rotate_lock_status(bool *bStatus)
{
	bool bLocked = FALSE;
	int nErr = -1;
	nErr = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_DISPLAY_SCREEN_ROTATION_AUTO, &bLocked);
	if (nErr != SYSTEM_SETTINGS_ERROR_NONE) {
		VideoLogError("failed to get screen rotation status [0x%x]", nErr);
		return FALSE;
	}

	*bStatus = bLocked;

	return TRUE;
}

bool vp_play_config_get_sort_type_key(int *nType)
{

	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(PREF_VP_SORT_TYPE, &nVal);
	if (nErr != 0) {
		VideoLogError("failed to get sort type details [0x%x]", nErr);
		return FALSE;
	}

	*nType = nVal;

	return TRUE;
}

bool vp_play_config_set_preview_url_videos(const char *szMediaURL)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is null");
		return FALSE;
	}

	int 		nErr 		= 0;
	const char	*pStorePath = ".video-store";

	if (strstr(szMediaURL, pStorePath) == NULL) {
		nErr = preference_set_string(PREF_VP_VIDEO_PREVIEW_URL_VIDEOS, szMediaURL);
		if (nErr != 0) {
			VideoLogError("failed to set preview url [0x%x]", nErr);
			return FALSE;
		}
	}

	return TRUE;
}

bool vp_play_config_get_preview_url_videos(char **szMediaURL)
{
	int nErr = preference_get_string(PREF_VP_VIDEO_PREVIEW_URL_VIDEOS, szMediaURL);
	if (nErr != 0 || *szMediaURL == NULL) {
		VideoLogError("failed to get preview url [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_config_get_wifi_direct_connect_status(bool *bConnect)
{
	int nErr = -1;
	wifi_connection_state_e nwifistate = WIFI_CONNECTION_STATE_FAILURE;
	wifi_direct_state_e nwifidirectstate = WIFI_DIRECT_STATE_DEACTIVATED;
	bool bWifiState = FALSE;

	nErr = wifi_get_connection_state(&nwifistate);
	if (nErr != WIFI_ERROR_NONE) {
		VideoLogError("failed to get connection details [0x%x]", nErr);
		return FALSE;
	}

	if (nwifistate == WIFI_CONNECTION_STATE_CONNECTED) {
		bWifiState = TRUE;
	}

	nErr = wifi_direct_get_state(&nwifidirectstate);
	if (nErr != WIFI_DIRECT_ERROR_NONE) {
		VideoLogError("failed to get wifi-direct state [0x%x]", nErr);
		return FALSE;
	}

	if (nwifidirectstate >= WIFI_DIRECT_STATE_CONNECTED) {
		bWifiState = TRUE;
	}
	VideoLogError(" == %d == ", nwifidirectstate);

	*bConnect = bWifiState;

	return TRUE;
}

bool vp_play_config_get_wifi_direct_active_status(bool *bConnect)
{
	int nErr = 0;
	wifi_direct_state_e nVal = 0;
	bool bWifiState = FALSE;

	nErr = wifi_direct_get_state(&nVal);
	if (nErr != WIFI_DIRECT_ERROR_NONE) {
		VideoLogError("failed to get wifi direct state [0x%x]", nErr);
		return FALSE;
	}

	if (nVal >= WIFI_DIRECT_STATE_ACTIVATED) {
		bWifiState = TRUE;
	}
	VideoLogError(" == %d == ", nVal);

	*bConnect = bWifiState;

	return TRUE;

}

bool vp_play_config_get_call_state(bool *bCallOn)
{
	VideoLogInfo("start");
	telephony_call_h *call_list_sim1, *call_list_sim2;
	unsigned int count_sim1 = 0, count_sim2 = 0;
	telephony_handle_list_s tel_list;
	telephony_error_e ret_sim1, ret_sim2;

	int tel_valid = telephony_init(&tel_list);
	if (tel_valid != 0) {
		VideoLogError("telephony is not initialized. ERROR Code is %d", tel_valid);
		return false;
	}

	ret_sim1 = telephony_call_get_call_list(tel_list.handle[0], &count_sim1, &call_list_sim1);
	if (ret_sim1 != TELEPHONY_ERROR_NONE) {
		VideoLogError("Cannot get call list information for primary sim");
	}

	telephony_call_release_call_list(count_sim1, &call_list_sim1);

	if(tel_list.count > 1) {
		ret_sim2 = telephony_call_get_call_list(tel_list.handle[1], &count_sim2, &call_list_sim2);
		if (ret_sim2 != TELEPHONY_ERROR_NONE) {
			VideoLogError("Cannot get call list information for secondey sim");
		}
		telephony_call_release_call_list(count_sim2, &call_list_sim2);
	}

	telephony_deinit(&tel_list);

	if (count_sim1 == 0 && count_sim2 == 0) {
		*bCallOn = FALSE;
		return false;
	} else {
		*bCallOn = TRUE;
		return true;
	}

	return false;
}

bool vp_play_config_get_battery_charge_state(bool *bChargeState)
{
	int nErr = -1;
	int nVal = 0;
	nErr = runtime_info_get_value_int(RUNTIME_INFO_KEY_BATTERY_IS_CHARGING, &nVal);
	if (nErr != RUNTIME_INFO_ERROR_NONE) {
		VideoLogError("failed to get charging status [0x%x]", nErr);
		return FALSE;
	}

	*bChargeState = (nVal == RUNTIME_INFO_KEY_CHARGER_CONNECTED) ? TRUE : FALSE;
	VideoLogInfo("bChargeState: %d", *bChargeState);
	return TRUE;
}


bool vp_play_config_get_lcd_off_state(bool *bLCDOff)
{
	int nErr = 0;
	display_state_e state = DISPLAY_STATE_NORMAL;
	nErr = device_display_get_state(&state);
	if (nErr != DEVICE_ERROR_NONE) {
		VideoLogError("device_display_get_state is fail [0x%x]", nErr);
		return FALSE;
	}

	if (state >= DISPLAY_STATE_SCREEN_DIM) {
		*bLCDOff = TRUE;
	} else {
		*bLCDOff = FALSE;
	}

	return TRUE;
}

bool vp_play_config_set_rotate_state_key_changed_cb(system_settings_changed_cb pFunc, void *pUserData)
{
	int nErr = -1;
	nErr = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_DISPLAY_SCREEN_ROTATION_AUTO, pFunc, pUserData);
	if (nErr != SYSTEM_SETTINGS_ERROR_NONE) {
		VideoLogError("failed to register rotation callback [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_config_ignore_rotate_state_key_changed_cb()
{
	int nErr = -1;
	nErr = system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_DISPLAY_SCREEN_ROTATION_AUTO);
	if (nErr != SYSTEM_SETTINGS_ERROR_NONE) {
		VideoLogError("failed to un-register rotation callback [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_config_set_battery_charge_state_key_changed_cb(runtime_info_changed_cb pFunc, void *pUserData)
{
	int nErr = -1;
	nErr = runtime_info_set_changed_cb(RUNTIME_INFO_KEY_BATTERY_IS_CHARGING, pFunc, pUserData);
	if (nErr != RUNTIME_INFO_ERROR_NONE) {
		VideoLogError("failed to register battery charging callback [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_wifi_initialize()
{
	int nErr = -1;
	nErr = wifi_initialize();
	if (nErr != WIFI_ERROR_NONE) {
		VideoLogError("wifi_initialize is fail [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_config_set_wifi_state_key_changed_cb(wifi_connection_state_changed_cb pFunc, void *pUserData)
{
	int nErr = -1;
	nErr =  wifi_set_connection_state_changed_cb(pFunc, pUserData);
	if (nErr != WIFI_ERROR_NONE) {
		VideoLogError("failed to register wifi state changed callback [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_config_ignore_battery_charge_state_key_changed_cb()
{
	int nErr = -1;
	nErr = runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_BATTERY_IS_CHARGING);
	if (nErr != RUNTIME_INFO_ERROR_NONE) {
		VideoLogError("failed to un-register battery charging callback [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_config_ignore_wifi_state_key_changed_cb()
{
	int nErr = -1;
	nErr = wifi_unset_connection_state_changed_cb();
	if (nErr != WIFI_ERROR_NONE) {
		VideoLogError("failed to un-register wifi state changed callback [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}

void vp_play_wifi_deinitialize()
{
	int nErr = -1;
	nErr = wifi_deinitialize();
	if (nErr != WIFI_ERROR_NONE) {
		VideoLogError("wifi_deinitialize is fail [0x%x]", nErr);
	}
}
