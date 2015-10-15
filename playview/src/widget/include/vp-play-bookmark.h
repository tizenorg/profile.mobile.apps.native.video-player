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

typedef void *bookmark_handle;

typedef void (*BookmarkCaptureStartCbFunc)(void *pUserData);
typedef void (*BookmarkItemSelectCbFunc)(int nPos, const char *szPath, void* pUserData);


bookmark_handle vp_play_bookmark_create(Evas_Object *pParent);
void vp_play_bookmark_destroy(bookmark_handle pWidgetHandle);
bool vp_play_bookmark_realize(bookmark_handle pWidgetHandle);
bool vp_play_bookmark_unrealize(bookmark_handle pWidgetHandle);
bool vp_play_bookmark_is_realize(bookmark_handle pWidgetHandle, bool *bIsRealize);
Evas_Object *vp_play_bookmark_get_object(bookmark_handle pWidgetHandle);
bool vp_play_bookmark_set_capture_callback(bookmark_handle pWidgetHandle, BookmarkCaptureStartCbFunc func);
bool vp_play_bookmark_set_item_select_callback(bookmark_handle pWidgetHandle, BookmarkItemSelectCbFunc func);
bool vp_play_bookmark_set_user_param(bookmark_handle pWidgetHandle, void *pUserData);
bool vp_play_bookmark_set_media_url(bookmark_handle pWidgetHandle, const char *szMediaURL);
bool vp_play_bookmark_set_edit_mode(bookmark_handle pWidgetHandle, bool bEditMode);
bool vp_play_bookmark_get_edit_mode(bookmark_handle pWidgetHandle, bool *bEditMode);
bool vp_play_bookmark_get_pressed_status(bookmark_handle pWidgetHandle, bool *bPressed);

bool vp_play_bookmark_insert_item(bookmark_handle pWidgetHandle, const char *szBookmarkURL, int nPos);
bool vp_play_bookmark_get_item_count(bookmark_handle pWidgetHandle, int *nCount);

