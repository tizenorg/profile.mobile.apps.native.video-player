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


#ifndef _MP_VIDEO_LIST_SORT_CTRL_
#define _MP_VIDEO_LIST_SORT_CTRL_


typedef enum
{
	MP_LIST_SORT_BY_NONE			= 0,
	//MP_LIST_SORT_BY_RECENTLY_VIEWED,
	MP_LIST_SORT_BY_RECENTLY_ADDED =2,
	MP_LIST_SORT_BY_NAME =3,
	//MP_LIST_SORT_BY_SIZE,
	//MP_LIST_SORT_BY_TYPE,
	MP_LIST_SORT_BY_MAX,
}MpListSortType;


typedef void (*ListSortCtrlCbFunc)(void);


bool mp_sort_ctrl_show(ListSortCtrlCbFunc pListSortCtrlCb);
void mp_sort_ctrl_hide(void);
void mp_sort_ctrl_change_language(void);
int mp_sort_ctrl_get_sort_state(void);


#endif // _MP_VIDEO_LIST_SORT_CTRL_

