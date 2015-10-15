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
#ifndef  __VIDEO_MP_UTIL_CONFIG__
#define  __VIDEO_MP_UTIL_CONFIG__

#include <stdbool.h>
#include <Elementary.h>
#include <vconf.h>
#include <app_preference.h>

bool mp_util_config_set_multi_play_status(bool bMultiPlay);
bool mp_util_config_get_multi_play_status(bool *bMultiPlay);

bool mp_util_config_set_preview(char* szPreviewUrl);
char* mp_util_config_get_preview(void);
bool mp_util_config_preview_key_changed_cb(preference_changed_cb callback, void *pUserData);
bool mp_util_config_ignore_preview_key_changed_cb();

#endif

