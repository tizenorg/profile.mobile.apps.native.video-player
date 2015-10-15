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


#ifndef _VIDEO_LIST_PERSONAL_VIEW_
#define  _VIDEO_LIST_PERSONAL_VIEW_

#include <stdbool.h>
#include "mp-video-type-define.h"


typedef void (*ChangePersonalViewCbFunc)(MpListUpdateType eUpdateType);

/*nViewStyle: 0->add to personal, 1->Remove from personal */
void mp_personal_view_push(void* pNaviFrame, ChangePersonalViewCbFunc pChangeViewCb, char *pFolderPath, int nViewStyle);
void mp_personal_view_pop(void);
bool mp_personal_view_is_top_view(void);
void mp_personal_view_update(void);


#endif

