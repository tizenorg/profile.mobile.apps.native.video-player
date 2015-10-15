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
#include <glib.h>
#include <Elementary.h>

#include <vp-play-popup.h>

typedef void *sound_path_handle;

sound_path_handle vp_sound_path_create(Evas_Object *pParent, PopupCloseCbFunc pCloseCb);
void vp_sound_path_destroy(sound_path_handle pSoundPathHandle);
bool vp_sound_path_realize(sound_path_handle pSoundPathHandle);
bool vp_sound_path_unrealize(sound_path_handle pSoundPathHandle);
bool vp_sound_path_set_user_data(sound_path_handle pSoundPathHandle, void *pUserData);

