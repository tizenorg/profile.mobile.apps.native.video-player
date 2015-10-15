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

typedef void *play_speed_popup;

typedef void (*PlaySpeedUpdateCbFunc)(double fValue, void* pUserData);
typedef void (*PlaySpeedDoneCbFunc)(double fValue, bool bSetValue, void* pUserData);


play_speed_popup vp_play_speed_popup_create(Evas_Object *pParent, double fSpeedVal, PlaySpeedUpdateCbFunc pUpdateCb,  PlaySpeedDoneCbFunc pDoneCb);
void vp_play_speed_popup_destroy(play_speed_popup pPlaySpeedHandle);
bool vp_play_speed_popup_realize(play_speed_popup pPlaySpeedHandle);
bool vp_play_speed_popup_unrealize(play_speed_popup pPlaySpeedHandle);
bool vp_play_speed_popup_set_user_data(play_speed_popup pPlaySpeedHandle, void *pUserData);
