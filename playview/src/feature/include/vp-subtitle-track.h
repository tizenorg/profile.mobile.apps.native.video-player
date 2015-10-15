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
#ifdef _SUBTITLE_MULTI_LANGUAGE
typedef void (*SubtitleLanguageChangeCbFunc)(int index, void* pUserData);
#endif
typedef void *subtitle_track_handle;

#ifdef _SUBTITLE_MULTI_LANGUAGE
subtitle_track_handle vp_subtitle_track_create(Evas_Object *pParent, PopupCloseCbFunc pCloseCb);
#else
subtitle_track_handle vp_subtitle_track_create(Evas_Object *pParent, PopupCloseCbFunc pCloseCb, int nDefaultIndex);
#endif
void vp_subtitle_track_destroy(subtitle_track_handle pSubtitleTrackHandle);
bool vp_subtitle_track_realize(subtitle_track_handle pSubtitleTrackHandle);
bool vp_subtitle_track_unrealize(subtitle_track_handle pSubtitleTrackHandle);
#ifdef _SUBTITLE_MULTI_LANGUAGE
bool vp_subtitle_track_add_Item(subtitle_track_handle pSubtitleTrackHandle, char *szCode, int nIndex, bool bCheck);
#else
bool vp_subtitle_track_add_Item(subtitle_track_handle pSubtitleTrackHandle, char *szCode, int nIndex);
#endif
bool vp_subtitle_track_set_user_data(subtitle_track_handle pSubtitleTrackHandle, void *pUserData);
#ifdef _SUBTITLE_MULTI_LANGUAGE
bool vp_subtitle_track_set_add_language_cb(subtitle_track_handle pSubtitleTrackHandle, SubtitleLanguageChangeCbFunc pCallback);
bool vp_subtitle_track_set_remove_language_cb(subtitle_track_handle pSubtitleTrackHandle, SubtitleLanguageChangeCbFunc pCallback);
#endif
bool vp_subtitle_track_get_name(char *szCode, char **szName);

