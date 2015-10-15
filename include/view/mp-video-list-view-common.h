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


#ifndef _VIDEO_LIST_VIEW_COMMON_
#define  _VIDEO_LIST_VIEW_COMMON_


#include <stdbool.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "mp-video-type-define.h"

typedef void (*UpdateListItemsCbFunc)(MpListUpdateType eUpdateType);

typedef struct
{
	void *pNaviFrameHandle;
	void *pNaviFrameItem;

	void *pVideosGenlist;				// DON'T CHANGE GENLIST HANDLE!!!

	Ecore_Timer *pDbUpdateTimer;	//db Update

	MpVideoListTabType nListTabType;

	int nCurrentViewType;

	Evas_Smart_Cb pGenlistItemtLanguageChangedCb;
	Evas_Smart_Cb pGenlistItemtRealizeCb;
	Evas_Smart_Cb pGenlistItemtPressCb;
	Evas_Smart_Cb pGenlistItemtLongPressCb;
}st_VideoListViewMainViewHandle;


typedef struct
{
	Evas_Object *pBox;					// DON'T CHANGE BOX !!!
	Evas_Object *pToolBar;				// DON'T CHANGE TOOLBAR !!!

	////////////////////////////////////////////////////////////
	// personal tab
	Evas_Object *pListViewBaselayout;
	Evas_Object *pListViewTitlelayout;
	Evas_Object *pListViewNocontentlayout;
	////////////////////////////////////////////////////////////
	Evas_Object *pScroller;

	Evas_Object *pVideoGenlistTitle;
	Elm_Genlist_Item_Class *pVideoGenlistTitleItc;
	Elm_Object_Item *pVideoGenlistItemForTitle;
	Elm_Object_Item *pVideoGenlistItemForNoContentLayout;

	Evas_Object *gesture;
	int zoom_level;
}st_VideoListViewMainViewWidget;

void mp_list_common_set_callabcks(void *viewHandle,
				    Evas_Smart_Cb pGenlistItemtLanguageChangedCb,
				    Evas_Smart_Cb pGenlistItemtRealizeCb,
				    Evas_Smart_Cb pGenlistItemtPressCb,
				    Evas_Smart_Cb pGenlistItemtLongPressCb);
void mp_list_common_unset_callabcks(void *viewHandle);

void mp_folder_view_set_is_item_view(bool isSelected);
bool mp_folder_view_is_item_view(void);
bool mp_folder_view_is_top_view(void);


#endif

