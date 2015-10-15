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


#ifndef _VIDEO_LIST_VIEW_
#define  _VIDEO_LIST_VIEW_

#include <stdbool.h>
#include "mp-video-type-define.h"


void mp_list_view_push(void* pNaviFrame);
void mp_list_view_destroy(void);
void mp_list_view_update_widget(void);
void mp_list_view_update_widget_resume(void);
void mp_list_view_change_language(void);
void mp_list_view_change_color(void);
void mp_list_view_destroy_player(void);
void mp_list_view_update_whole_genlist(MpListUpdateType eUpdateType);
void mp_list_view_rotate_view(MpListUpdateType eUpdateType);
void mp_list_view_update_count_layout();
void mp_list_view_update_base_layout();
void mp_list_view_search_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);

#endif

