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
#ifndef  __VIDEO_MP_UTIL_PREFERENCE__
#define  __VIDEO_MP_UTIL_PREFERENCE__

#include <Elementary.h>
#include <stdbool.h>
#include <app_preference.h>

/*Currently, for different process, we could not use preference. Only save and read key in the same process, we could use preference.*/


bool mp_util_preference_set_multi_play_status(bool set);
bool mp_util_preference_get_multi_play_status(bool *bMultiPlay);

char* mp_util_preference_get_preview(void);
bool mp_util_preference_set_preview(char* szPreview);
bool mp_util_preference_set_preview_change(preference_changed_cb callback, void *pUserData);
bool mp_util_preference_unset_preview_change(void);

void mp_util_preference_init();


#endif

