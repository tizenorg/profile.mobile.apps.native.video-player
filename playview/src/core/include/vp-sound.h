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


typedef enum {
	VP_SOUND_DEVICE_NONE = 0x00,
	VP_SOUND_DEVICE_SPEAKER,
	VP_SOUND_DEVICE_EARJACK,
	VP_SOUND_DEVICE_BLUETOOTH,
	VP_SOUND_DEVICE_HDMI,
	VP_SOUND_DEVICE_MIRRORING,
	VP_SOUND_DEVICE_USB_AUDIO,
	VP_SOUND_DEVICE_MAX,
}video_sound_device_type_t;

typedef void(*vp_sound_route_change_cb)(int nRoute, bool bAvailable, void *pUserData);
typedef void(*vp_sound_volume_change_cb)(int nType, int nVolume, void *pUserData);

bool vp_sound_init_session();
bool vp_sound_deinit_session();
bool vp_sound_get_volume(int *nValue);
bool vp_sound_set_volume(int nValue);
bool vp_sound_get_max_volume(int *nMaxValue);
bool vp_sound_get_active_device(video_sound_device_type_t *nDeviceType);
bool vp_sound_is_sound_path_enable(bool *bIsSoundPath);
bool vp_sound_get_a2dp_name(char **szBTName);
bool vp_sound_set_active_route(video_sound_device_type_t nDeviceType);
bool vp_sound_set_route_change_cb(vp_sound_route_change_cb func, void *pUserData);
bool vp_sound_set_volume_change_cb(vp_sound_volume_change_cb func, void *pUserData);
bool vp_sound_device_is_enable(video_sound_device_type_t nDeviceType, void *pUserData);

