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


#ifndef __MP_VIDEO_RENAME_CTRL__
#define __MP_VIDEO_RENAME_CTRL__

typedef void(*mpRenameCtrlCbFunc)(void);

Evas_Object* mp_rename_ctrl_show(void *pNaviFrameHandlet, void *pUserData, mpRenameCtrlCbFunc pRenameCtrlCb, int nVideoItemIndex);
void mp_rename_ctrl_delete_handle(void);
void mp_rename_ctrl_focus_out_cb(void);
bool mp_rename_ctrl_is_top_view();
void mp_rename_ctrl_set_focus();
#endif // __MP_VIDEO_RENAME_CTRL__


