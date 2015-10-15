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


#ifndef _MP_VIDEO_LIST_OPTION_CTRL_
#define _MP_VIDEO_LIST_OPTION_CTRL_


typedef enum
{
	MP_LIST_OPTION_NONE				= 0,
	MP_LIST_OPTION_SHARE_VIA_ITEM,
	MP_LIST_OPTION_ADD_HOME_ITEM,
	MP_LIST_OPTION_RENAME_ITEM,
	MP_LIST_OPTION_DETAILS_ITEM,
	MP_LIST_OPTION_DELETE_ITEM,
	MP_LIST_OPTION_DOWNLOAD_ITEM,
	MP_LIST_OPTION_MAX,
}MpListOptionType;

typedef enum
{
	MP_LIST_OPTION_DOWNLOAD_NONE				= 0,
	MP_LIST_OPTION_CLOUD_DOWNLOAD_ITEM,
	MP_LIST_OPTION_ASF_DOWNLOAD_ITEM,
	MP_LIST_OPTION_DOWNLOAD_OPTION_MAX,
}MpDownloadType;


typedef struct
{
	void* ShareViaItemCb;
	void* AddToHomeItemCb;
	void* DeleteItemCb;
	void* DownloadCb;  //CLOUD, ALLSHARE
}st_RegisteOptionCbFunc;

typedef void (* MpListOptionCbFunc)(void *pUserData);

bool mp_option_ctrl_show(void *pNaviHandle, const char *title, int nVideoIndex,
	st_RegisteOptionCbFunc *pCallbackFunc, MpListOptionCbFunc pCloseFun, void*userData, MpDownloadType euType);
void mp_option_ctrl_change_language(void);
void mp_option_ctrl_delete_handle(void);
void *mp_option_ctrl_get_handle(void);


#endif // _MP_VIDEO_LIST_OPTION_CTRL_
