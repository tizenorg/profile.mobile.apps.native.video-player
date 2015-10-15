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

typedef void *subtitle_font_handle;

typedef void (*SubtitleFontCbFunc)(char *szSelectURL, void* pUserData);

subtitle_font_handle vp_subtitle_font_create(Evas_Object *pParent, SubtitleFontCbFunc pCloseCb, char *szCurrentFont);
void vp_subtitle_font_destroy(subtitle_font_handle pSubtitleFontHandle);
bool vp_subtitle_font_realize(subtitle_font_handle pSubtitleFontHandle);
bool vp_subtitle_font_unrealize(subtitle_font_handle pSubtitleFontHandle);
bool vp_subtitle_font_set_user_data(subtitle_font_handle pSubtitleFontHandle, void *pUserData);

