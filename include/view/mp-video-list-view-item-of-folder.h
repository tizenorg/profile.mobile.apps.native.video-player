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

#ifndef  _VIDEO_ITEM_VIEW_OF_FOLDER_
#define  _VIDEO_ITEM_VIEW_OF_FOLDER_

#include "mp-video-list-view-common.h"
#include "mp-video-type-define.h"

void mp_folder_item_view_push(void* pNaviFrame, int folder_index, UpdateListItemsCbFunc pChangeViewCb);
bool mp_folder_item_view_is_current_view();
void mp_folder_item_view_check_more_button_of_toolbar(void);
void mp_folder_item_view_change_language(void);
void mp_folder_item_view_update_played_item(void);
void mp_folder_item_view_update_video_list(MpListUpdateType eUpdateType);

#endif
