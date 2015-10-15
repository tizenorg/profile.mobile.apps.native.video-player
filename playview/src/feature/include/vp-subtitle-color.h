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

typedef void *subtitle_color_handle;

typedef void (*SubtitleColorUpdateCbFunc)(double fValue, void* pUserData);

#ifndef SUBTITLE_K_FEATURE
typedef void (*SubtitlePopupCloseCbFunc)(int nType, bool bPause, void* pUserData);
#else
typedef void (*SubtitlePopupCloseCbFunc)(char *pColorHex, bool bSetValue, void* pUserData);
#endif

#ifndef SUBTITLE_K_FEATURE
subtitle_color_handle vp_subtitle_color_create(Evas_Object *pParent, SubtitlePopupCloseCbFunc pCloseCb, int nDefaultColor);
#else
subtitle_color_handle vp_subtitle_color_hex_create(Evas_Object *pParent, SubtitlePopupCloseCbFunc pCloseCb, char *pDefaultColor);
#endif
void vp_subtitle_color_destroy(subtitle_color_handle pSubtitleFontColorHandle);
bool vp_subtitle_color_realize(subtitle_color_handle pSubtitleFontColorHandle);
bool vp_subtitle_color_unrealize(subtitle_color_handle pSubtitleFontColorHandle);
bool vp_subtitle_color_set_user_data(subtitle_color_handle pSubtitleFontColorHandle, void *pUserData);

