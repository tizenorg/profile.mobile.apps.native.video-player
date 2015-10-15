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


#ifndef _VIDEO_LIST_REMOVE_VIEW_
#define  _VIDEO_LIST_REMOVE_VIEW_

#include <stdbool.h>
#include "mp-video-type-define.h"


typedef void (*ChangeListRemoveViewCbFunc)(void);
typedef void (*ListRemoveViewCbFunc)(MpListUpdateType eUpdateType);

void mp_remove_view_push(void* pNaviFrame, ListRemoveViewCbFunc pUpdateViewCb, ChangeListRemoveViewCbFunc pChangeViewCb, char *pFolderPath, MpVideoListTabType eTabType);
void mp_remove_view_pop(void);
bool mp_remove_view_is_top_view(void);
void mp_remove_view_change_language(void);

#endif

