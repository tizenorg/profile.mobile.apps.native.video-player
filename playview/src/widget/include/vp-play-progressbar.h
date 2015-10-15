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

typedef void *progressbar_handle;


typedef enum {
	VIDEO_PROGRESSBAR_TYPE_NORMAL = 0x00,
	VIDEO_PROGRESSBAR_TYPE_TRIM ,
}video_progressbar_type_t;

progressbar_handle vp_play_progressbar_create(Evas_Object *pParent, video_progressbar_type_t nType);
void vp_play_progressbar_destroy(progressbar_handle pWidgetHandle);
bool vp_play_progressbar_realize(progressbar_handle pWidgetHandle);
bool vp_play_progressbar_unrealize(progressbar_handle pWidgetHandle);
Evas_Object *vp_play_progressbar_get_object(progressbar_handle pWidgetHandle);
bool vp_play_progressbar_set_duration(progressbar_handle pWidgetHandle, int nDuration);
bool vp_play_progressbar_set_position(progressbar_handle pWidgetHandle, int nPosition);
bool vp_play_progressbar_set_buffering_position(progressbar_handle pWidgetHandle, int nBufferinPosition);
bool vp_play_progressbar_set_landscape_mode(progressbar_handle pWidgetHandle, bool bLandscape);
bool vp_play_progressbar_set_opacity(progressbar_handle pWidgetHandle, bool bDefault);
Evas_Object *vp_play_progressbar_get_focus_object(progressbar_handle pWidgetHandle);

