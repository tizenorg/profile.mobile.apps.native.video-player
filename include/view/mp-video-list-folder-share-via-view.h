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


#ifndef _VIDEO_LIST_FOLDER_SHARE_VIA_VIEW_
#define  _VIDEO_LIST_FOLDER_SHARE_VIA_VIEW_

#include <stdbool.h>
#include "mp-video-type-define.h"

typedef void (*ChangeListFolderShareViewCbFunc)(void);

void mp_folder_share_view_push(void* pNaviFrame, ChangeListFolderShareViewCbFunc pChangeViewCb, MpVideoListTabType eTabType);
void mp_folder_share_view_pop(void);
void mp_folder_share_view_change_language(void);

#endif
