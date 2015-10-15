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

typedef void *volume_popup_handle;

volume_popup_handle vp_play_volume_popup_create(Evas_Object *pParent);
void vp_play_volume_popup_destroy(volume_popup_handle pVolumeHandle);
bool vp_play_volume_popup_realize(volume_popup_handle pVolumeHandle);
bool vp_play_volume_popup_unrealize(volume_popup_handle pVolumeHandle);
bool vp_play_volume_popup_set_landscape_mode(volume_popup_handle pVolumeHandle, bool bLandscape);
bool vp_play_volume_popup_is_realize(volume_popup_handle pVolumeHandle, bool *bIsRealize);
bool vp_play_volume_popup_set_value(volume_popup_handle pVolumeHandle, int nCurVal);
bool vp_play_volume_popup_get_value(int *nCurVal);
bool vp_play_volume_popup_get_max_value(int *nMaxVal);
bool vp_play_volume_popup_is_mute(volume_popup_handle pVolumeHandle, bool *bIsMute);



