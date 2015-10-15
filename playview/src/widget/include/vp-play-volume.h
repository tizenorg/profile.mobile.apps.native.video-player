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

typedef void *volume_handle;

typedef void (*NormalVolumeChangeCbFunc)(int nValue, void* pUserData);

volume_handle vp_play_volume_create(Evas_Object *pParent, void *pPlayerHandle, NormalVolumeChangeCbFunc pFunc);
void vp_play_volume_destroy(volume_handle pVolumeHandle);
bool vp_play_volume_realize(volume_handle pVolumeHandle, Evas_Object *pWin);
bool vp_play_volume_unrealize(volume_handle pVolumeHandle);
bool vp_play_volume_set_landscape_mode(volume_handle pVolumeHandle, bool bLandscape, Evas_Object *pWin);
bool vp_play_volume_set_user_data(volume_handle pVolumeHandle, void *pUserData);
bool vp_play_volume_is_realize(volume_handle pVolumeHandle, bool *bIsRealize);
bool vp_play_volume_increase_value(volume_handle pVolumeHandle);
bool vp_play_volume_decrease_value(volume_handle pVolumeHandle);
bool vp_play_volume_update_value(volume_handle pVolumeHandle);
bool vp_play_volume_is_mute(volume_handle pVolumeHandle, bool *bIsMute);
int vp_play_volume_get_max_value(volume_handle pVolumeHandle);
int vp_play_volume_get_value(volume_handle pVolumeHandle);
bool vp_play_volume_set_value(volume_handle pVolumeHandle, int nVolumeVal);
bool vp_play_volume_set_mute_value(volume_handle pVolumeHandle);
int vp_play_volume_get_mute_value(volume_handle pVolumeHandle);
bool vp_play_volume_set_focus_in(volume_handle pVolumeHandle);
bool vp_play_volume_set_mouse_callbacks(volume_handle pVolumeHandle, Evas_Object *pWin);
bool vp_play_volume_unset_mouse_callbacks(volume_handle pVolumeHandle, Evas_Object *pWin);
