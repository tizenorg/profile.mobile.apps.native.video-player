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

typedef void *sound_alive_handle;

sound_alive_handle vp_sound_alive_create(Evas_Object *pParent, char *pMediaUrl, PopupCloseCbFunc pCloseCb, video_sound_alive_t nType);
void vp_sound_alive_destroy(sound_alive_handle pSAHandle);
bool vp_sound_alive_realize(sound_alive_handle pSAHandle);
bool vp_sound_alive_unrealize(sound_alive_handle pSAHandle);
bool vp_sound_alive_set_user_data(sound_alive_handle pSAHandle, void *pUserData);

