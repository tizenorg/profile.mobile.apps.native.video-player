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

typedef void *brightness_handle;

brightness_handle vp_play_brightness_create(Evas_Object *pParent);
void vp_play_brightness_destroy(brightness_handle pBrightnessHandle);
bool vp_play_brightness_realize(brightness_handle pBrightnessHandlee);
bool vp_play_brightness_unrealize(brightness_handle pBrightnessHandlee);
bool vp_play_brightness_set_landscape_mode(brightness_handle pBrightnessHandle, bool bLandscape);
bool vp_play_brightness_is_realize(brightness_handle pBrightnessHandle, bool *bIsRealize);
bool vp_play_brightness_set_value(brightness_handle pBrightnessHandle, int nCurVal);
bool vp_play_brightness_get_value(int *nCurVal);
bool vp_play_brightness_get_max_value(int *nMaxVal);
bool vp_play_brightness_get_min_value(int *nMinVal);


