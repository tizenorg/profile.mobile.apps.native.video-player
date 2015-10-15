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

typedef enum {
	VIDEO_LOADING_SIZE_SMALL = 0x00,
	VIDEO_LOADING_SIZE_MIDUM,
	VIDEO_LOADING_SIZE_LARGE,
	VIDEO_LOADING_SIZE_XLARGE,
}video_loading_size_t;

Evas_Object *vp_play_loading_ani_create(Evas_Object *pParent, video_loading_size_t nSize);
void vp_play_loading_ani_destroy(Evas_Object *pProgress);

