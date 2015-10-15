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

#ifndef  _VIDEO_PLAYER_SEARCH_VIEW_
#define  _VIDEO_PLAYER_SEARCH_VIEW_

#include "mp-video-type-define.h"
#include "mp-video-list-view-folder.h"


typedef void (*ListViewUpdateListItemCbFunc)(void);
typedef void (*ViewUpdateListItemCbFunc)(MpListUpdateType eUpdateType);

void mp_search_view_push(void* pNaviFrame, char *pFolderPath);
void mp_search_view_register_update_list_func(ViewUpdateListItemCbFunc pUpdateViewCb, ListViewUpdateListItemCbFunc pChangeViewCb);
void mp_search_view_change_language(void);
void mp_search_view_update_played_item(int nIndex);
char* mp_search_view_search_markup_keyword(char *szOriginStr, char *szSearchWord, bool *bResult);
void mp_search_view_update_genlist();
bool mp_search_view_get_active_view();

#endif
