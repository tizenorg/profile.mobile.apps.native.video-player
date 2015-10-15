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


#ifndef _VIDEO_FOOTER_TOOLBAR_
#define _VIDEO_FOOTER_TOOLBAR_


typedef enum
{
	FOOTER_TAB_TYPE_PERSONAL			= 0,
	FOOTER_TAB_TYPE_DOWNLOAD,
	FOOTER_LIST_ITEM_TYPE_MAX,
}MpFooterLibraryMainListTabType;


typedef struct
{
	void* DownloadCb;
	void* SearchCb;
	void* ShareViaCb;
	void* DeleteCb;
	void* ViewAsCb;
	void* SortItemCb;
	void* RenameCb;
	void* DetailsCb;
	void* SignInCb;
#ifdef	ENABLE_PRIVATE_MODE
	void* AddToPersonalCB;
	void* RemoveFromPersonalCB;
#endif
}st_RegisteCbFunc;


void mp_ft_ctrl_create(void* pNaviFrameHandle, void* pNaviFrameItem, MpFooterLibraryMainListTabType nFooterListTabType, st_RegisteCbFunc* pRegisteCbFunc);
void mp_ft_ctrl_destroy(void);
void mp_ft_ctrl_hide_more_popup(void);
void mp_ft_ctrl_create_center_button(void* pUserData, void* pToolbarCenterButtonCb);
void mp_ft_ctrl_create_center_toolbar(void *pParent, void* pToolbarCenterButtonCb);
void mp_ft_ctrl_delete_center_button(void);
void mp_ft_ctrl_disable(void);
void mp_ft_ctrl_delete_center_toolbar();
void mp_ft_ctrl_rotate_update();
bool mp_ft_ctrl_is_show(void);
#endif
