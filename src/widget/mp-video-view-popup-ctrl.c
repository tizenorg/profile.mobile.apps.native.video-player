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

#include <app.h>
#include <glib.h>
#include <app_preference.h>

#include <Eina.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-main.h"
#include "mp-util-widget-ctrl.h"
#include "vp-util.h"

#define PREF_MP_VIDEO_CONTENT_DISPLAYER_TYPE_KEY "preference/org.tizen.videos/storage_type"


typedef struct __list_view_ctr {
	Evas_Object *pLVPopUpHandle;
	Evas_Object *pLVGroupRadio;
	Evas_Object *pLVGenList;
	Evas_Object *pBox;

	int nListCount;
	MpListItemType nCurrViewType;
	Elm_Genlist_Item_Class *lv_Itc;
	ListViewCtrlCbFunc pListViewCtrlCbFunc;
} st_list_view_ctr;

static st_list_view_ctr *g_list_view_data = NULL;

static void __mp_view_ctrl_btn_asc_cb(void *pUserData,
				      Evas_Object *pObject,
				      void *pEventInfo);

void mp_view_ctrl_set_type(MpListItemType nViewType)
{
	if (!g_list_view_data) {
		VideoLogError("");
		return;
	}

	if (MP_LIST_VIEW_NONE >= nViewType || nViewType >= MP_LIST_VIEW_MAX) {
		VideoLogWarning
		("[WARNING] Value of View as type is not matched.");
		nViewType = MP_LIST_VIEW_ALL;
	}

	g_list_view_data->nCurrViewType = nViewType;

	int nRet =
		preference_set_int(PREF_MP_VIDEO_CONTENT_DISPLAYER_TYPE_KEY,
			      g_list_view_data->nCurrViewType);
	if (nRet) {
		VideoLogWarning
		("[WARNING] Fail to set value about list view type.");
	}
}

void mp_view_ctrl_delete_handle(void *pUserData)
{
	VideoLogInfo("");

	if (!pUserData) {
		VideoLogInfo("pUserData is NULL");
		return;
	}
	st_list_view_ctr *pLVData = (st_list_view_ctr *) pUserData;
	if (pLVData->pLVPopUpHandle) {
		eext_object_event_callback_del(pLVData->pLVPopUpHandle,
					       EEXT_CALLBACK_BACK,
					       __mp_view_ctrl_btn_asc_cb);
		evas_object_smart_callback_del(pLVData->pLVPopUpHandle,
					       "block,clicked",
					       __mp_view_ctrl_btn_asc_cb);
	}
	MP_DEL_ITC(pLVData->lv_Itc);
	MP_DEL_OBJ(pLVData->pLVGroupRadio);
	MP_DEL_OBJ(pLVData->pLVGenList);
	MP_DEL_OBJ(pLVData->pBox);
	MP_DEL_OBJ(pLVData->pLVPopUpHandle);
	MP_FREE_STRING(pLVData);
	g_list_view_data = NULL;

	VideoLogInfo("end");
}

static void __mp_view_ctrl_btn_asc_cb(void *pUserData,
				      Evas_Object *pObject,
				      void *pEventInfo)
{
	VideoLogInfo("");

	mp_view_ctrl_hide();
}

/*//////////////////////////////////////////////////////////*/
/* External function*/
/*//////////////////////////////////////////////////////////*/
bool mp_view_ctrl_show(ListViewCtrlCbFunc pListViewCtrlCb)
{
	VideoLogInfo("disable cloud module.");
	return false;
}

void mp_view_ctrl_hide(void)
{
	VideoLogInfo("");

	if (!g_list_view_data) {
		VideoLogInfo("g_list_view_data is NULL.");
		return;
	}

	mp_view_ctrl_delete_handle(g_list_view_data);
}

void mp_view_ctrl_change_language(void)
{
	return;
}

int mp_view_ctrl_get_list_type(void)
{
	return MP_LIST_VIEW_ALL;
}
