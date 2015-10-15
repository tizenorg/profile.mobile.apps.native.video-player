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
#pragma once

#include <stdbool.h>
#include <Elementary.h>

#include <vp-play-popup.h>

typedef void *setting_popup_handle;

typedef enum {
	VP_SETTING_MODE_NONE = 0x00,
	VP_SETTING_MODE_PLAY_SPEED,
	VP_SETTING_MODE_SOUND_ALIVE,
	VP_SETTING_MODE_AUDIO_TRACK,
	VP_SETTING_MODE_REPEAT_MODE,
	VP_SETTING_MODE_CAPTURE,
}vp_setting_mode_t;

typedef struct _SettingInfo{
	float	fSpeed;
	int	nSoundAlive;
	bool	bSelectSubtitle;
	char	*szAudioTrack;
	int 	nRepeatMode;
	bool	bCapture;
	bool	bTagBuddy;
	bool	bEditWeatherTag;
	bool 	bStreaming;
	bool 	bRepeat;
	bool	bDrm;
	bool	bExternalMode;
	bool	bShowEditWeather;
}SettingInfo;

setting_popup_handle vp_setting_create(Evas_Object *pParent, SettingInfo *pSettingInfo, PopupCloseCbFunc pCloseCb);
void vp_setting_destroy(setting_popup_handle pSettingHandle);
bool vp_setting_realize(setting_popup_handle pSettingHandle);
bool vp_setting_unrealize(setting_popup_handle pSettingHandle);
bool vp_setting_set_user_data(setting_popup_handle pSettingHandle, void *pUserData);

