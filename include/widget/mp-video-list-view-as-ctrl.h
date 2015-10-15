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


#ifndef _MP_VIDEO_LIST_VIEW_AS_CTRL_
#define _MP_VIDEO_LIST_VIEW_AS_CTRL_


typedef enum
{
	MP_LIST_VIEW_AS_NONE				= 0,
#ifdef VS_FEATURE_LIST_VIEW
	MP_LIST_VIEW_AS_NORMAL_LIST,
#endif
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	MP_LIST_VIEW_AS_THUMBNAIL_LIST,
#endif
	MP_LIST_VIEW_AS_FOLDER_LIST,
	MP_LIST_VIEW_AS_MAX,
}MpListViewAsType;


typedef void(*ListViewAsCtrlCbFunc)(void);


bool mp_view_as_ctrl_show(int tabType, ListViewAsCtrlCbFunc pListViewAsCtrlCb);
void mp_view_as_ctrl_hide(void);
void mp_view_as_ctrl_change_language(void);
int mp_view_as_ctrl_get_type(void);


#endif // _MP_VIDEO_LIST_VIEW_AS_CTRL_
