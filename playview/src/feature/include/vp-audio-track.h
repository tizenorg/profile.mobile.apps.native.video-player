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

typedef void *audio_track_handle;


audio_track_handle vp_audio_track_create(Evas_Object *pParent, PopupCloseCbFunc pCloseCb, int nDefaultIndex);
void vp_audio_track_destroy(audio_track_handle pAudioTrackHandle);
bool vp_audio_track_realize(audio_track_handle pAudioTrackHandle);
bool vp_audio_track_unrealize(audio_track_handle pAudioTrackHandle);
bool vp_audio_track_add_Item(audio_track_handle pAudioTrackHandle, char *szCode, int nIndex);
bool vp_audio_track_set_user_data(audio_track_handle pAudioTrackHandle, void *pUserData);
bool vp_audio_track_get_name(char *szCode, char **szName);
