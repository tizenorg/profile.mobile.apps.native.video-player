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

typedef void *subtitle_sync_popup;

typedef void (*SubtitleSyncUpdateCbFunc)(float fValue, void* pUserData);
typedef void (*SubtitleSyncDoneCbFunc)(float fValue, bool bSetValue, void* pUserData);


subtitle_sync_popup vp_subtitle_sync_popup_create(Evas_Object *pParent, float fSyncVal, SubtitleSyncUpdateCbFunc pUpdateCb,  SubtitleSyncDoneCbFunc pDoneCb);
void vp_subtitle_sync_popup_destroy(subtitle_sync_popup pSubtitleSync);
bool vp_subtitle_sync_popup_realize(subtitle_sync_popup pSubtitleSync);
bool vp_subtitle_sync_popup_unrealize(subtitle_sync_popup pSubtitleSync);
bool vp_subtitle_sync_popup_set_user_data(subtitle_sync_popup pSubtitleSync, void *pUserData);

