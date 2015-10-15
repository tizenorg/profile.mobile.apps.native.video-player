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

#include <glib.h>

#include <stdbool.h>
#include <Elementary.h>

#include <vp-play-popup.h>

typedef void *subtitle_select_handle;

typedef void (*SubtitleCloseCbFunc)(char *szSelectURL, void* pUserData);

subtitle_select_handle vp_subtitle_select_create(Evas_Object *pParent, SubtitleCloseCbFunc pCloseCb, SubtitleCloseCbFunc pButtonCb, GList *pFileList, char *szCurrentPath, bool isSubtitleOff);
void vp_subtitle_select_destroy(subtitle_select_handle pSubtitleSelectHandle);
bool vp_subtitle_select_realize(subtitle_select_handle pSubtitleSelectHandle);
bool vp_subtitle_select_unrealize(subtitle_select_handle pSubtitleSelectHandle);
bool vp_subtitle_select_set_user_data(subtitle_select_handle pSubtitleSelectHandle, void *pUserData);
