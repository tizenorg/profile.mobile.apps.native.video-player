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


#ifndef _MP_VIDEO_LIST_AUTO_PLAY_CTRL_
#define _MP_VIDEO_LIST_AUTO_PLAY_CTRL_


typedef enum
{
	MP_LIST_AUTO_PLAY_NONE				= 0,
	MP_LIST_AUTO_PLAY_ON,
	MP_LIST_AUTO_PLAY_OFF,
	MP_LIST_AUTO_PLAY_MAX,
}MpListAutoPlaySelect;


typedef void(*ListViewAsCtrlCbFunc)(void);


bool mp_auto_play_ctrl_show();
void mp_auto_play_ctrl_hide(void);
void mp_auto_play_ctrl_change_language(void);
int mp_auto_play_ctrl_get_type(void);


#endif // _MP_VIDEO_LIST_AUTO_PLAY_CTRL_
