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

#include <Eina.h>
#include <Ecore_Evas.h>
#include <Elementary.h>
#include <glib.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-external-ug.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-util-media-service.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-list-remove-ctrl.h"
#include "mp-video-list-remove-view.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-video-list-view-item-of-folder.h"
#include "mp-video-util-db-controller.h"
#include "mp-util-widget-ctrl.h"
#include "vp-file-util.h"
#include "mp-rotate-ctrl.h"
#include "videos-view-mgr.h"

#define VIDEO_SELECTED_INFO_TIMER		4.0

typedef struct
{
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	void *pObjVideosList;		//used for list view ,folder view,thumbnail view

	Elm_Genlist_Item_Class *pGenListItc;
	Elm_Genlist_Item_Class *pGenListLandscapeItc;
	Elm_Gengrid_Item_Class *pGenGridItc_1;
	Elm_Gengrid_Item_Class *pGenGridItc_2;
	bool *pCheckedItemsEditListForRemoved;
	char **pSelectedList; //save selected video id
	bool bSelectallCheckBoxState;
	int nSelectCounterForEditList;
	MpMediaSvcSortType nSortType;
	ChangeListRemoveViewCbFunc pChangeViewUserCbFunc; //view pop
	ListRemoveViewCbFunc pChangeViewUpdateFunc; //update main list
	char *pFolderPath;
	MpVideoListTabType eTabType;
	MpDbUpdateViewLev euLev;
	bool bUpdateList;
	bool bFolderDel;
#ifdef VS_FEATURE_THUMBNAIL_VIEW	//
	bool bLandscape;
	bool bThumbnailView;
#endif
}st_VideoListRemoveViewHandle;

typedef struct
{
	//Evas_Object *pSelectionInfoLayout;
	Evas_Object *pVideoListBox;
	Evas_Object *pRemoveViewBaselayout;
	Evas_Object *pSelectallbtn;
	Evas_Object *pSelAllLayout;
	Evas_Object *pSavebtn;
	Evas_Object *pCancelbtn;

	Ecore_Timer *pSelectedInfoLayoutTimer;
}st_VideoListRemoveViewWidget;

static st_VideoListRemoveViewHandle *g_pListRemoveViewHandle = NULL;
static st_VideoListRemoveViewWidget *g_pListRemoveViewWidget = NULL;

// Pre define function
int mp_remove_view_get_sort_type(void);
static bool __mp_remove_view_arrange_video_list(void *pObjVideosList, MpVideoListTabType eTabType, bool bNeedSvc);
static bool __mp_remove_view_arrange_folder_list(void *pObjVideosList, MpVideoListTabType eTabType, bool bNeedSvc);
void mp_remove_view_naviframe_transition_effect_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
void mp_remove_view_change_title_info(void);
static void __mp_remove_view_base_layout_del_cb(void *pUserData, Evas *e, Evas_Object *pObject, void *pEventInfo);
static void __mp_remove_view_rotate_cb(void *data, Evas_Object *obj, void *event_info);

static bool __mp_remove_view_get_selected_video_list(int nSelectedSize);
//static void __mp_remove_view_update_selected_video_list(void);

static void __mp_remove_view_free_selected_list(void)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewHandle->pSelectedList)
	{
		VideoLogInfo("Selected_list is already freed.");
		return;
	}
	int nIndex = 0;
	while (g_pListRemoveViewHandle->pSelectedList[nIndex])
	{
		nIndex++;
		MP_FREE_STRING(g_pListRemoveViewHandle->pSelectedList[nIndex]);
	}
	MP_FREE_STRING(g_pListRemoveViewHandle->pSelectedList);
}

void mp_remove_view_update_all_check_box(void)
{
	VideoLogInfo("");

	int nVideoListSize = 0;
	if (LIST_TAB_TYPE_PERSONAL == g_pListRemoveViewHandle->eTabType)
	{
		if (g_pListRemoveViewHandle->bFolderDel)
		{
			nVideoListSize = mp_util_svc_get_video_folder_size();
		}
		else
		{
			nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		}
	}
	else
	{
		VideoLogError("Can't handle this eTabType!");
	}

	if (!g_pListRemoveViewHandle->pObjVideosList) {
		VideoLogInfo("genlist is NULL");
		return;
	}
	if (nVideoListSize < 1) {
		VideoLogInfo("nVideoListSize < 1:%d", nVideoListSize);
		return;
	}

	Evas_Object *check_box = NULL;
	Evas_Object *check_box_layout = NULL;
	Elm_Object_Item *currentItem = NULL;
	Elm_Object_Item *currentItemGengrid = NULL;
	int nVideoItemIndex = 0;
#ifdef VS_FEATURE_THUMBNAIL_VIEW	//
	if (g_pListRemoveViewHandle->bThumbnailView)
	{
		currentItem = elm_genlist_first_item_get(g_pListRemoveViewHandle->pObjVideosList);
		nVideoItemIndex = 0;
		while (currentItem && (nVideoItemIndex < nVideoListSize)) {
			Evas_Object *gengrid= elm_object_item_part_content_get(currentItem, "elm.icon");
			currentItemGengrid = elm_gengrid_first_item_get(gengrid);
			while (currentItemGengrid && (nVideoItemIndex < nVideoListSize)) {
				Evas_Object *check_box= elm_object_item_part_content_get(currentItemGengrid, "elm.swallow.end");
				if (check_box) {
					elm_check_state_set(check_box, g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex]);
				}
				currentItemGengrid = elm_gengrid_item_next_get(currentItemGengrid);
				nVideoItemIndex ++;
			}
			currentItem = elm_genlist_item_next_get(currentItem);
		}

	}
	else
#endif
	{
		currentItem = elm_genlist_first_item_get(g_pListRemoveViewHandle->pObjVideosList);
		nVideoItemIndex = 0;
		while (currentItem && (nVideoItemIndex < nVideoListSize)) {
			check_box_layout = elm_object_item_part_content_get(currentItem, "elm.icon.2");
			check_box = elm_object_part_content_get(check_box_layout, "elm.swallow.content");
			if (check_box) {
				elm_check_state_set(check_box, g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex]);
			}
			currentItem = elm_genlist_item_next_get(currentItem);
			nVideoItemIndex ++;
		}
	}

}

void mp_remove_view_destroy(void)
{
	VideoLogInfo("");

	if (g_pListRemoveViewHandle)
	{
		evas_object_smart_callback_del(g_pListRemoveViewHandle->pNaviFrameHandle, "transition,finished", mp_remove_view_naviframe_transition_effect_cb);
		evas_object_smart_callback_del((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_remove_view_rotate_cb);
		__mp_remove_view_free_selected_list();
		MP_DEL_ITC(g_pListRemoveViewHandle->pGenListItc);
		MP_DEL_GRID_ITC(g_pListRemoveViewHandle->pGenGridItc_1);
		MP_DEL_GRID_ITC(g_pListRemoveViewHandle->pGenGridItc_2);
		MP_DEL_OBJ(g_pListRemoveViewHandle->pObjVideosList);
		MP_FREE_STRING(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved);
		g_pListRemoveViewHandle->pChangeViewUserCbFunc = NULL;
		g_pListRemoveViewHandle->pChangeViewUpdateFunc = NULL;
		MP_FREE_STRING(g_pListRemoveViewHandle->pFolderPath);

		free(g_pListRemoveViewHandle);
		g_pListRemoveViewHandle = NULL;
	}

	if (g_pListRemoveViewWidget)
	{
		MP_DEL_OBJ(g_pListRemoveViewWidget->pSelectallbtn);
		MP_DEL_OBJ(g_pListRemoveViewWidget->pSavebtn);
		MP_DEL_OBJ(g_pListRemoveViewWidget->pCancelbtn);
		MP_DEL_OBJ(g_pListRemoveViewWidget->pSelAllLayout);
		MP_DEL_OBJ(g_pListRemoveViewWidget->pVideoListBox);
		MP_DEL_OBJ(g_pListRemoveViewWidget->pRemoveViewBaselayout);
		MP_DEL_TIMER(g_pListRemoveViewWidget->pSelectedInfoLayoutTimer);
		free(g_pListRemoveViewWidget);
		g_pListRemoveViewWidget = NULL;
	}
}

static void __mp_remove_view_reset(void)
{
	VideoLogInfo("");

	if (g_pListRemoveViewHandle)
	{
		mp_util_db_set_update_fun(g_pListRemoveViewHandle->euLev, NULL);
		mp_util_db_set_backup_fun(g_pListRemoveViewHandle->euLev, NULL);
		evas_object_smart_callback_del(g_pListRemoveViewHandle->pNaviFrameHandle, "transition,finished", mp_remove_view_naviframe_transition_effect_cb);
		evas_object_smart_callback_del((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_remove_view_rotate_cb);
		__mp_remove_view_free_selected_list();
		g_pListRemoveViewHandle->pObjVideosList = NULL;
		MP_DEL_ITC(g_pListRemoveViewHandle->pGenListItc);
		MP_DEL_GRID_ITC(g_pListRemoveViewHandle->pGenGridItc_1);
		MP_DEL_GRID_ITC(g_pListRemoveViewHandle->pGenGridItc_2);
		MP_FREE_STRING(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved);
		g_pListRemoveViewHandle->pChangeViewUserCbFunc = NULL;
		g_pListRemoveViewHandle->pChangeViewUpdateFunc = NULL;
		MP_FREE_STRING(g_pListRemoveViewHandle->pFolderPath);

		free(g_pListRemoveViewHandle);
		g_pListRemoveViewHandle = NULL;
	}

	if (g_pListRemoveViewWidget)
	{

		MP_DEL_OBJ(g_pListRemoveViewWidget->pSelectallbtn);
		MP_DEL_OBJ(g_pListRemoveViewWidget->pCancelbtn);
		MP_DEL_OBJ(g_pListRemoveViewWidget->pSavebtn);
		g_pListRemoveViewWidget->pSelAllLayout = NULL;
		if (g_pListRemoveViewWidget->pVideoListBox)
		{
			g_pListRemoveViewWidget->pVideoListBox = NULL;
		}

		if (g_pListRemoveViewWidget->pRemoveViewBaselayout)
		{
			g_pListRemoveViewWidget->pRemoveViewBaselayout = NULL;
		}

		MP_DEL_TIMER(g_pListRemoveViewWidget->pSelectedInfoLayoutTimer);

		free(g_pListRemoveViewWidget);
		g_pListRemoveViewWidget = NULL;
	}
}

void mp_remove_view_reset_value(void)
{
	VideoLogInfo("");

	if (!g_pListRemoveViewHandle)
	{
		VideoLogError("[ERR] g_pListRemoveViewHandle is not existed.");
		return;
	}

	g_pListRemoveViewHandle->pNaviFrameHandle = NULL;
	g_pListRemoveViewHandle->pNaviFrameItem = NULL;
	g_pListRemoveViewHandle->pChangeViewUserCbFunc = NULL;

	g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
	g_pListRemoveViewHandle->nSelectCounterForEditList = 0;
	g_pListRemoveViewHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;

	if (g_pListRemoveViewHandle->pNaviFrameItem)
	{
		elm_object_item_part_content_unset(g_pListRemoveViewHandle->pNaviFrameItem, "toolbar_more_btn");
		elm_object_item_part_content_unset(g_pListRemoveViewHandle->pNaviFrameItem, "toolbar");
		elm_object_item_part_content_unset(g_pListRemoveViewHandle->pNaviFrameItem, "toolbar_button2");
	}
}


////////////////////////////////////////////////////
// Remove items. (Below functions are about the item deletion to call by button of toolbar.)
void mp_remove_view_done_delete_complete(void)
{
	if (!g_pListRemoveViewHandle) {
		VideoLogError("[ERR] g_pListRemoveViewHandle is NULL.");
		return;
	}
	g_pListRemoveViewHandle->bUpdateList = TRUE;
	VideoLogInfo("");
	if (g_pListRemoveViewHandle->pNaviFrameHandle) {
		elm_naviframe_item_pop(g_pListRemoveViewHandle->pNaviFrameHandle);
	}
}

void mp_remove_view_response_done_delete_items_popup_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pListRemoveViewHandle)
	{
		VideoLogError("[ERR] g_pListRemoveViewHandle is NULL.");
		return;
	}

	mp_util_delete_popup_handle();
	g_pListRemoveViewHandle->bUpdateList = TRUE;
	int nVideoListSize = 0;

	if (LIST_TAB_TYPE_PERSONAL == g_pListRemoveViewHandle->eTabType)
	{
		if (g_pListRemoveViewHandle->bFolderDel)
		{
			nVideoListSize = mp_util_svc_get_video_folder_size();
		}
		else
		{
			nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		}
		mp_remove_ctrl_show(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved,
					nVideoListSize,
					mp_remove_view_done_delete_complete,
					g_pListRemoveViewHandle->bFolderDel,
					NULL);
	}
	else
	{
		VideoLogError("Can't handle this eTabType!");
	}
}

void mp_remove_view_response_cancel_delete_items_popup_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
}

void mp_remove_view_HW_mouse_back_popup_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo)
{
	Evas_Event_Mouse_Up *pEvent = pEventInfo;
	if (!pEvent) {
		VideoLogWarning("[ERR] pEvent is NULL.");
		return ;
	}

	if (pEvent->button == 3)
	{
		mp_util_delete_popup_handle();
	}
}

void mp_remove_view_HW_key_back_popup_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo)
{
	Evas_Event_Key_Down *pEvent = pEventInfo;
	if (!pEvent) {
		VideoLogWarning("[ERR] pEvent is NULL.");
		return ;
	}
	if (!g_strcmp0(pEvent->keyname, "XF86Back"))
	{
		mp_util_delete_popup_handle();
	}
}

static void __mp_remove_view_run_cancel_items_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	if (!g_pListRemoveViewHandle->pNaviFrameHandle) {
		VideoLogError("g_pListRemoveViewHandle->pNaviFrameHandle is NULL");
		return;
	}

	VideoLogInfo("");

	elm_naviframe_item_pop(g_pListRemoveViewHandle->pNaviFrameHandle);
}


static void __mp_remove_view_run_delete_items_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	if (!g_pListRemoveViewHandle) {
		VideoLogError("[ERR] g_pMainViewHandle is NULL.");
		return;
	}

	VideoLogInfo("");

	if (!g_pListRemoveViewHandle->pObjVideosList) {
		VideoLogError("No exist genlist object.");
		return;
	}

	int nSelectCounter = g_pListRemoveViewHandle->nSelectCounterForEditList;

	if (nSelectCounter > 0) {
		MpListViewAsType type = mp_view_as_ctrl_get_type();
		bool bItemFolderView = mp_folder_item_view_is_current_view();

		if (type == MP_LIST_VIEW_AS_FOLDER_LIST && bItemFolderView)	{
			type = MP_LIST_VIEW_AS_THUMBNAIL_LIST;
		}

		if (g_pListRemoveViewHandle->bSelectallCheckBoxState)
			mp_util_remove_all_videos_question_popup((void *) NULL, type, mp_remove_view_response_done_delete_items_popup_cb, mp_remove_view_response_cancel_delete_items_popup_cb, mp_remove_view_HW_mouse_back_popup_cb, mp_remove_view_HW_key_back_popup_cb);
		else
			mp_util_remove_question_popup((-nSelectCounter), type, mp_remove_view_response_done_delete_items_popup_cb, mp_remove_view_response_cancel_delete_items_popup_cb, mp_remove_view_HW_mouse_back_popup_cb, mp_remove_view_HW_key_back_popup_cb);
		//g_pListRemoveViewHandle->nSelectCounterForEditList = 0;
	}
}

static void __mp_remove_view_trans_finished_cb(void *pUserData, Evas_Object *obj, void *event_info)
{
	if (!obj)
	{
		VideoLogError("[ERR] g_pListRemoveViewHandle is NULL.");
		return;
	}

	mp_widget_ctrl_enable_navi_handle_focus(obj);

	evas_object_smart_callback_del(obj, "transition,finished", __mp_remove_view_trans_finished_cb);
}

static Eina_Bool __mp_remove_view_back_btn_cb(void *pUserData, Elm_Object_Item *pItem)
{
	if (!g_pListRemoveViewHandle)
	{
		VideoLogInfo("[ERR] g_pListRemoveViewHandle is NULL.");
		return EINA_TRUE;
	}

	// Register transition finished callback.
	mp_widget_ctrl_disable_navi_handle_focus_except_item(g_pListRemoveViewHandle->pNaviFrameHandle, g_pListRemoveViewHandle->pNaviFrameItem);
	evas_object_smart_callback_add(g_pListRemoveViewHandle->pNaviFrameHandle, "transition,finished", __mp_remove_view_trans_finished_cb, NULL);

	mp_remove_view_pop();

	return EINA_TRUE;
}

void mp_remove_view_change_title_info(void)
{
	VideoLogInfo("nNumOfSelectdCount : %d", g_pListRemoveViewHandle->nSelectCounterForEditList);

	if (g_pListRemoveViewHandle->nSelectCounterForEditList > 0)
	{
		elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_FALSE);
	}
	else
	{
		VideoLogInfo("Unactivate delete tab.");
		elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_TRUE);
	}

	char *szCheckingTotalItem = g_strdup_printf(VIDEOS_SELECTVIEW_HEADER_PD_SELECTED, g_pListRemoveViewHandle->nSelectCounterForEditList);
	elm_object_item_domain_translatable_part_text_set(g_pListRemoveViewHandle->pNaviFrameItem, "elm.text.title", PACKAGE, szCheckingTotalItem);
	MP_FREE_STRING(szCheckingTotalItem);
}


static void __mp_remove_view_select_all_change_check_box_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!pObject)
	{
		VideoLogError("[ERR]");
		return;
	}

	if (!g_pListRemoveViewHandle->pObjVideosList)
	{
		VideoLogError("No exist pVideoGenlist");
		return;
	}

	int nCounter = 0;
	int nVideoListSize = 0;
	//bool bState = elm_check_state_get(pObject);

	if (LIST_TAB_TYPE_PERSONAL == g_pListRemoveViewHandle->eTabType)
	{
		if (g_pListRemoveViewHandle->bFolderDel)
		{
			nVideoListSize = mp_util_svc_get_video_folder_size();
		}
		else
		{
			nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		}

	}
	else
	{
		VideoLogError("Can't handle this eTabType!");
	}

	VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	if (!g_pListRemoveViewHandle->bSelectallCheckBoxState)
	{
		g_pListRemoveViewHandle->bSelectallCheckBoxState = TRUE;
		g_pListRemoveViewHandle->nSelectCounterForEditList = nVideoListSize;
	}
	else
	{
		g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
		g_pListRemoveViewHandle->nSelectCounterForEditList = 0;

		VideoLogInfo("Unactivate delete tab.");
		elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_TRUE);
	}
	mp_remove_view_change_title_info();

	for (nCounter = 0; nCounter < nVideoListSize; nCounter++)
	{
		g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nCounter] = g_pListRemoveViewHandle->bSelectallCheckBoxState;
	}

	mp_remove_view_update_all_check_box();

	//elm_genlist_realized_items_update(g_pListRemoveViewHandle->pObjVideosList);
}

void mp_remove_view_delete_select_all(void)
{
	VideoLogInfo("");

	g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
	g_pListRemoveViewHandle->nSelectCounterForEditList = 0;

	//MpVideoListRemoveViewHideSelectedInfo();
	mp_remove_view_change_title_info();
}

////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Check box callback
void mp_remove_view_change_check_box_state_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!pObject)
	{
		VideoLogInfo("[ERR]");
		return;
	}
	else
	{
		VideoLogInfo("pObject : %x", pObject);
	}

	int nVideoListSize = 0;
	int nVideoItemIndex = 0;

	if (LIST_TAB_TYPE_PERSONAL == g_pListRemoveViewHandle->eTabType)
	{
		nVideoItemIndex = (int)pUserData;

		if (g_pListRemoveViewHandle->bFolderDel)
		{
			nVideoListSize = mp_util_svc_get_video_folder_size();
		}
		else
		{
			nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		}

		VideoLogInfo("Check box - Index : %d, List Size : %d", nVideoItemIndex, nVideoListSize);
	}
	else
	{
		VideoLogError("Can't handle this eTabType!");
	}


	bool bState = elm_check_state_get(pObject);

	VideoLogInfo("BEFORE : nSelectCounterForEditList : %d", g_pListRemoveViewHandle->nSelectCounterForEditList);

	if (bState)
	{
		g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex] = TRUE;
		g_pListRemoveViewHandle->nSelectCounterForEditList++;
		if (nVideoListSize == g_pListRemoveViewHandle->nSelectCounterForEditList)
		{
			VideoLogInfo("All check box of edit list state are true.");
			g_pListRemoveViewHandle->bSelectallCheckBoxState = TRUE;
		}
	}
	else
	{
		g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex] = FALSE;
		if (nVideoListSize == g_pListRemoveViewHandle->nSelectCounterForEditList)
		{
			VideoLogInfo("All check box of edit list state are not true.");
			g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
		}

		g_pListRemoveViewHandle->nSelectCounterForEditList--;
		if (g_pListRemoveViewHandle->nSelectCounterForEditList < 1)
		{
			VideoLogInfo("Unactivate delete button.");
			elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_TRUE);
		}
	}

	elm_check_state_set(g_pListRemoveViewWidget->pSelectallbtn, g_pListRemoveViewHandle->bSelectallCheckBoxState);
	mp_remove_view_change_title_info();
	/*if (pSelectedItem)
	{
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}*/

}
//
////////////////////////////////////////////////////

////////////////////////////////////////////////
// Genlist callback
void mp_remove_view_naviframe_transition_effect_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pListRemoveViewHandle)
	{
		VideoLogError("[ERR] No exist g_pListRemoveViewHandle.");
		return;
	}

	VideoLogInfo("");

	Evas_Object *pTmpContents = NULL;
	Elm_Object_Item *pTmpItem = NULL;

	if (!g_pListRemoveViewHandle->pNaviFrameHandle)
	{
		VideoLogError("[ERR] No exist naviframe handle.");
		return;
	}

	pTmpItem = elm_naviframe_top_item_get(g_pListRemoveViewHandle->pNaviFrameHandle);
	pTmpContents = elm_object_item_content_get(pTmpItem);
	if (pTmpContents)
	{
		if (pTmpContents == g_pListRemoveViewWidget->pVideoListBox)
		{
		}
		else
		{
			VideoLogInfo("It's not pListRemoveViewTabbarLayout in top screen.");
		}
	}
	else
	{
		VideoLogError("[ERR] No exist naviframe top item.");
	}
}

void mp_remove_view_double_tap_cb(void)
{
	if (!g_pListRemoveViewHandle->pObjVideosList)
	{
		VideoLogError("pObjVideosList is not exist.");
		return;
	}

	VideoLogInfo("");

	elm_genlist_item_bring_in(elm_genlist_first_item_get(g_pListRemoveViewHandle->pObjVideosList), ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

char *mp_remove_view_get_duration_time(int nVideoItemIndex)
{
	VideoLogInfo("");

	char szTmpStr[STR_LEN_MAX] = {0, };
	char szTmpStrDurationTime[STR_LEN_MAX] = {0, };
	char szTmpStrLastPlayedPos[STR_LEN_MAX] = {0, };

	unsigned int nVideoInfoDuration = mp_util_svc_get_video_duration_time(nVideoItemIndex);
	unsigned int nVideoInfoLastTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);

	mp_util_convert_time(nVideoInfoDuration, szTmpStrDurationTime, FALSE);
	mp_util_convert_time(nVideoInfoLastTime, szTmpStrLastPlayedPos, TRUE);

	snprintf(szTmpStr, STR_LEN_MAX, "%s/%s", szTmpStrLastPlayedPos, szTmpStrDurationTime);

	return strdup(szTmpStr);
}
#ifdef VS_FEATURE_LIST_VIEW
static void __mp_remove_folder_view_select_video_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pListRemoveViewHandle->pObjVideosList)
	{
		VideoLogInfo("No exist genlist object.");
		return;
	}

	VideoLogInfo("");

	int nVideoItemIndex = 0;

	Elm_Object_Item *pItem = (Elm_Object_Item *)pEventInfo;
	Elm_Object_Item *pSelectedItem = elm_genlist_selected_item_get(pObject);

	if (pItem)
	{
		int nVideoListSize = 0;
		if (LIST_TAB_TYPE_PERSONAL == g_pListRemoveViewHandle->eTabType)
		{
			nVideoItemIndex = (int)elm_object_item_data_get(pItem);
			VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);
			if (g_pListRemoveViewHandle->bFolderDel)
			{
				nVideoListSize = mp_util_svc_get_video_folder_size();
			}
			else
			{
				nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
			}
		}
		else
		{
			VideoLogError("Can't handle this eTabType!");
		}

		if (g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex])
		{
			g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex] = FALSE;
			if (nVideoListSize == g_pListRemoveViewHandle->nSelectCounterForEditList)
			{
				VideoLogInfo("All check box of edit list state are not true.");
				g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
			}

			g_pListRemoveViewHandle->nSelectCounterForEditList--;
			if (g_pListRemoveViewHandle->nSelectCounterForEditList < 1)
			{
				VideoLogInfo("Unactivate delete button.");
				elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_TRUE);
			}
		}
		else
		{
			g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex] = TRUE;
			g_pListRemoveViewHandle->nSelectCounterForEditList++;
			if (nVideoListSize == g_pListRemoveViewHandle->nSelectCounterForEditList)
			{
				VideoLogInfo("All check box of edit list state are true.");
				g_pListRemoveViewHandle->bSelectallCheckBoxState = TRUE;
			}
		}

		elm_check_state_set(g_pListRemoveViewWidget->pSelectallbtn, g_pListRemoveViewHandle->bSelectallCheckBoxState);
		mp_remove_view_change_title_info();
		Evas_Object *check_box_layout = elm_object_item_part_content_get(pItem, "elm.icon.2");
		Evas_Object *check_box = elm_object_part_content_get(check_box_layout, "elm.swallow.content");

		if (check_box) {
			elm_check_state_set(check_box, g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex]);
		}

		//elm_genlist_item_fields_update(pSelectedItem, "elm.edit.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
	}
	else
	{
		VideoLogError("[ERR]Error item number.");
	}

	if (pSelectedItem) {
		VideoLogInfo("[INFORM] --pSelectedItem --");
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
}
#endif

static void __mp_remove_view_select_grid_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pListRemoveViewHandle->pObjVideosList)
	{
		VideoLogInfo("No exist genlist object.");
		return;
	}

	VideoLogInfo("");

	int nVideoItemIndex = 0;

	Elm_Object_Item *pItem = (Elm_Object_Item *)pEventInfo;

	if (pItem) {
		int nVideoListSize = 0;
		if (LIST_TAB_TYPE_PERSONAL == g_pListRemoveViewHandle->eTabType) {
			nVideoItemIndex = (int)elm_object_item_data_get(pItem);
			VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);
			if (g_pListRemoveViewHandle->bFolderDel)
			{
				nVideoListSize = mp_util_svc_get_video_folder_size();
			} else {
				nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
			}
		} else {
			VideoLogError("Can't handle this eTabType!");
		}

		if (g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex]) {
			g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex] = FALSE;

			if (nVideoListSize == g_pListRemoveViewHandle->nSelectCounterForEditList) {
				VideoLogInfo("All check box of edit list state are not true.");
				g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
			}

			g_pListRemoveViewHandle->nSelectCounterForEditList--;
			if (g_pListRemoveViewHandle->nSelectCounterForEditList < 1) {
				VideoLogInfo("Unactivate delete button.");
				elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_TRUE);

			}
		} else {
			g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex] = TRUE;
			g_pListRemoveViewHandle->nSelectCounterForEditList++;
			if (nVideoListSize == g_pListRemoveViewHandle->nSelectCounterForEditList) {
				VideoLogInfo("All check box of edit list state are true.");
				g_pListRemoveViewHandle->bSelectallCheckBoxState = TRUE;
			}
		}

		elm_check_state_set(g_pListRemoveViewWidget->pSelectallbtn, g_pListRemoveViewHandle->bSelectallCheckBoxState);

		mp_remove_view_change_title_info();

		elm_gengrid_item_update(pItem);
		//keep checkbox animation effect
		elm_gengrid_item_selected_set(pItem, EINA_FALSE);
	} else {
		VideoLogError("[ERR]Error item number.");
	}
}

char *mp_remove_view_get_label_of_video_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text.main.left.top"))
	{
		char *szTitle;
		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		if (szTitle)
		{
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	}
	else if (!strcmp(pPart, "elm.text.sub.left.bottom"))
	{
		return mp_util_svc_get_duration_str_time(nVideoItemIndex);
	}
	/*else if (!strcmp(pPart, "elm.text.sub.right.bottom"))
	{
		unsigned long long nFilesize = 0;

		nFilesize = mp_util_svc_get_video_item_size(nVideoItemIndex);
		char *szFileSize = mp_util_get_file_size(nFilesize);
		char *szTitleUtf8 = NULL;
		if (szFileSize)
		{
			szTitleUtf8 = elm_entry_utf8_to_markup(szFileSize);
			free(szFileSize);
			szFileSize = NULL;
		}

		return szTitleUtf8;
	}*/

	return NULL;
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW	//
static char *__mp_remove_view_get_label_of_grid_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text")) {
		char *szTitle = NULL;
		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		if (szTitle) {
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.2")) {
		return mp_util_svc_get_duration_str_time(nVideoItemIndex);
	}

	return NULL;
}
#endif

static int mp_remove_view_get_respective_video_no(int nFolderItemIndex)
{
	mp_util_svc_destory_video_item_list();
	char *szPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
	mp_util_svc_extract_video_list_from_folder(szPath, mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_ALL);
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	MP_FREE_STRING(szPath);

	return nVideoListSize;
}

static char *__mp_remove_view_get_label_of_folder_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nFolderItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text.main.left.top") || !strcmp(pPart, "elm.text")) {
		char *szTitle = NULL;
		char *title = NULL;
		char buffer[10] = {0,};
		szTitle = mp_util_svc_get_video_folder_name(nFolderItemIndex);
		int nVideoListSize = mp_remove_view_get_respective_video_no(nFolderItemIndex);
		snprintf(buffer, 10, "%d", nVideoListSize);
		title = g_strconcat(szTitle, "(", buffer, ")", NULL);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(title);

		MP_FREE_STRING(szTitle);
		MP_FREE_STRING(title);
		return szTitleUtf8;
	}
	/*else if (!strcmp(pPart, "elm.text.sub.left.bottom") || !strcmp(pPart, "elm.text.2")) {
		char *szFolderPath = NULL;
		szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
		char *pDes_path = mp_util_get_dir_by_path(szFolderPath);
		MP_FREE_STRING(szFolderPath);
		char *szFolderUtf8 = elm_entry_utf8_to_markup(pDes_path);
		MP_FREE_STRING(pDes_path);
		return szFolderUtf8;
	}*/

	return NULL;
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW
static Evas_Object *__mp_remove_folder_view_get_grid_icon_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nFolderItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.swallow.icon")) {
		char *pThumbIconUri = mp_util_get_folder_thumbnail(nFolderItemIndex, mp_sort_ctrl_get_sort_state());
		Evas_Object *pBg = NULL;

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			pBg = elm_image_add(pObject);
			MP_FREE_STRING(pThumbIconUri);
			evas_object_size_hint_aspect_set(pBg, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
			elm_image_file_set(pBg, pThumbIconUri, NULL);
		} else {
			pBg = mp_util_create_preload_image(pObject, pThumbIconUri, GRID_VIDEO_ICON_WIDTH);
		}

		MP_FREE_STRING(pThumbIconUri);

		return pBg;
	} else if (!strcmp(pPart, "elm.swallow.end")) {
		Evas_Object *pCheckBox = elm_check_add(pObject);
		elm_object_style_set(pCheckBox, "default");
		evas_object_propagate_events_set(pCheckBox, EINA_TRUE);
		elm_check_state_pointer_set(pCheckBox, (Eina_Bool *)&(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nFolderItemIndex]));

		evas_object_show(pCheckBox);

		return pCheckBox;
	}
	return NULL;
}

static Evas_Object *__mp_remove_view_get_grid_icon_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	if (!strcmp(pPart, "elm.swallow.icon")) {
		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			pBg = elm_image_add(pObject);
			MP_FREE_STRING(pThumbIconUri);
			evas_object_size_hint_aspect_set(pBg, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
			elm_image_file_set(pBg, pThumbIconUri, NULL);
		} else {
			pBg = mp_util_create_preload_image(pObject, pThumbIconUri, GRID_VIDEO_ICON_WIDTH);
		}

		MP_FREE_STRING(pThumbIconUri);

		return pBg;
	} else if (!strcmp(pPart, "elm.progressbar.icon")) {
		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime <= 0) {
			return NULL;
		}

		double nRatio = 0.0;
		if (nDurationTime > 0) {
			nRatio =((double)nLastPlayedTime) / ((double)nDurationTime);
		}

		if (nRatio < 0.05) {
			nRatio = 0.05;
		}

		Evas_Object *pProgressBar = elm_progressbar_add(pObject);
		elm_object_style_set(pProgressBar, "video/list_progress");
		elm_progressbar_horizontal_set(pProgressBar, EINA_TRUE);
		evas_object_size_hint_align_set(pProgressBar, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(pProgressBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_progressbar_value_set(pProgressBar, nRatio);
		evas_object_show(pProgressBar);

		return pProgressBar;
	} else if (!strcmp(pPart, "elm.lock.icon")) {
		char *pVideoFile = mp_util_svc_get_video_url(nVideoItemIndex);
		bool bRet = mp_util_check_video_personal_status((const char*)pVideoFile);
		MP_FREE_STRING(pVideoFile);
		if (bRet) {
			Evas_Object *pLockIcon = elm_image_add(pObject);
			elm_image_file_set(pLockIcon, edj_path, VIDEO_LIST_VIEW_ICON_PERSONAL_LOCK);
			evas_object_size_hint_align_set(pLockIcon, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pLockIcon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(pLockIcon);
			return pLockIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.dropbox.icon")) {
		bool bRet = mp_util_svc_is_cloud_storage(nVideoItemIndex);
		if (bRet) {
			Evas_Object *pDropboxIcon = elm_image_add(pObject);
			elm_image_file_set(pDropboxIcon, edj_path, VIDEO_LIST_VIEW_ICON_DROPBOX);
			evas_object_size_hint_align_set(pDropboxIcon, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pDropboxIcon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(pDropboxIcon);
			return pDropboxIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.swallow.end")) {
		Evas_Object *pCheckBox = elm_check_add(pObject);
		elm_object_style_set(pCheckBox, "grid");
		evas_object_propagate_events_set(pCheckBox, EINA_TRUE);
		elm_check_state_pointer_set(pCheckBox, (Eina_Bool *)&(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex]));

		evas_object_show(pCheckBox);

		return pCheckBox;
	}
	return NULL;
}
#endif

Evas_Object *mp_remove_view_get_icon_of_video_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	int nVideoItemIndex = (int)pUserData;
	Evas_Object *pLayout= NULL;

	if (!pPart || !pObject) {
		return NULL;
	}

	if (!strcmp(pPart, "elm.icon.1")) {
		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;
		char edj_path[1024] = {0};

		char *path = app_get_resource_path();
		snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_CUSTOM_THEME);
		free(path);
		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, edj_path, "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg = mp_util_create_preload_image(pLayout, pThumbIconUri, VIDEO_ICON_WIDTH);
		elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg);

		if (pBg)
			evas_object_show(pBg);

		MP_FREE_STRING(pThumbIconUri);

		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime > 0)
		{
			double nRatio = 0.0;

			if (nDurationTime > 0) {
				nRatio = ((double)nLastPlayedTime) /
						((double)nDurationTime);
			}

			if (nRatio < 0.05) {
				nRatio = 0.05;
			}

			Evas_Object *pProgressBar = elm_progressbar_add(pLayout);
			elm_object_style_set(pProgressBar, "elm/progressbar/horizontal/default");
			elm_progressbar_horizontal_set(pProgressBar, EINA_TRUE);
			elm_progressbar_span_size_set(pProgressBar, VIDEO_LIST_PROGRESSBAR_LENGTH);
			evas_object_size_hint_align_set(pProgressBar, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pProgressBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_value_set(pProgressBar, nRatio);
			elm_layout_content_set(pLayout, "elm.progressbar.icon", pProgressBar);

			if (pProgressBar)
				evas_object_show(pProgressBar);
		}

		if (!mp_util_create_video_sub_icon(pLayout, nVideoItemIndex))	{
			VideoLogWarning("Local File or Create sub icon is failed.", nVideoItemIndex);
		}

		return pLayout;
	}
	else if (!strcmp(pPart, "elm.icon.2"))
	{
		pLayout = elm_layout_add(pObject);

		Evas_Object *pCheckBox = elm_check_add(pLayout);

		elm_layout_theme_set(pLayout, "layout", "list/C/type.2", "default");

		elm_object_style_set(pCheckBox, "default");
		evas_object_propagate_events_set(pCheckBox, EINA_FALSE);
		evas_object_smart_callback_add(pCheckBox, "changed", mp_remove_view_change_check_box_state_cb, pUserData);

		elm_check_state_pointer_set(pCheckBox, (Eina_Bool *)&(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nVideoItemIndex]));
		elm_layout_content_set(pLayout, "elm.swallow.content", pCheckBox);

		evas_object_show(pCheckBox);

		return pLayout;
	}

	return NULL;
/*
	else if (!strcmp(pPart, "elm.dropbox.icon"))
	{
		bool bRet = mp_util_svc_is_cloud_storage(nVideoItemIndex);
		if (bRet)
		{
			Evas_Object *pDropboxIcon = elm_image_add(pObject);
			elm_image_file_set(pDropboxIcon, VIDEO_PLAYER_IMAGE_NAME_EDJ, VIDEO_LIST_VIEW_ICON_DROPBOX);
			evas_object_size_hint_align_set(pDropboxIcon, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pDropboxIcon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(pDropboxIcon);
			return pDropboxIcon;
		}
		else
		{
			return NULL;
		}
	}

	return NULL;
*/
}

/*static Evas_Object *__mp_remove_view_get_icon_of_folder_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nFolderItemIndex = (int)pUserData;

	if (!g_strcmp0(pPart, "elm.icon.1"))
	{
		VideoLogInfo("pPart: elm.icon.1 - thumbnail");

		Evas_Object *pLayout = NULL;
		Evas_Object *pBg = NULL;
		char *pThumbIconUri = mp_util_get_folder_thumbnail(nFolderItemIndex, mp_sort_ctrl_get_sort_state());

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, VIDEO_CUSTOM_THEME, "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri))
		{
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg = mp_util_create_preload_image(pLayout, pThumbIconUri, VIDEO_ICON_WIDTH);
		elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg);
		if (pBg)
			evas_object_show(pBg);

		if (!mp_util_create_folder_sub_icon(pLayout, nFolderItemIndex))	{
			VideoLogWarning("Create sub icon is failed. nFolderItemIndex: %d", nFolderItemIndex);
		}

		MP_FREE_STRING(pThumbIconUri);

		evas_object_show(pLayout);

		return pLayout;
	}
	else if (!strcmp(pPart, "elm.icon.2"))
	{
		Evas_Object *pLayout = elm_layout_add(pObject);
		Evas_Object *pCheckBox = elm_check_add(pLayout);

		elm_layout_theme_set(pLayout, "layout", "list/C/type.2", "default");
		evas_object_propagate_events_set(pCheckBox, EINA_FALSE);
		evas_object_smart_callback_add(pCheckBox, "changed", mp_remove_view_change_check_box_state_cb, pUserData);
		elm_check_state_set(pCheckBox, g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nFolderItemIndex]);
		elm_check_state_pointer_set(pCheckBox, (Eina_Bool *)&(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nFolderItemIndex]));

		evas_object_show(pCheckBox);

		elm_layout_content_set(pLayout, "elm.swallow.content", pCheckBox);

		return pLayout;
	}

	return NULL;
}*/

int mp_remove_view_get_sort_type(void)
{
	switch (mp_sort_ctrl_get_sort_state())
	{
	/*case MP_LIST_SORT_BY_RECENTLY_VIEWED:
		g_pListRemoveViewHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
		break;*/

	case MP_LIST_SORT_BY_RECENTLY_ADDED:
		g_pListRemoveViewHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_ADDED;
		break;

	case MP_LIST_SORT_BY_NAME:
		g_pListRemoveViewHandle->nSortType = MP_MEDIA_SORT_BY_NAME;
		break;

	/*case MP_LIST_SORT_BY_SIZE:
		g_pListRemoveViewHandle->nSortType = MP_MEDIA_SORT_BY_SIZE;
		break;

	case MP_LIST_SORT_BY_TYPE:
		g_pListRemoveViewHandle->nSortType = MP_MEDIA_SORT_BY_TYPE;
		break;*/
	}

	return g_pListRemoveViewHandle->nSortType;
}

void mp_remove_view_append_video_items(void *pObjVideosList)
{
	if (!g_pListRemoveViewHandle)
	{
		VideoLogInfo("invalid handle");
		return;
	}
	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	MP_DEL_ITC(g_pListRemoveViewHandle->pGenListItc);
	g_pListRemoveViewHandle->pGenListItc = elm_genlist_item_class_new();
	g_pListRemoveViewHandle->pGenListItc->item_style = "2line.top";	//"video/edit_list";
	g_pListRemoveViewHandle->pGenListItc->func.text_get = (void*)mp_remove_view_get_label_of_video_item_cb;
	g_pListRemoveViewHandle->pGenListItc->func.content_get = (void*)mp_remove_view_get_icon_of_video_item_cb;
	g_pListRemoveViewHandle->pGenListItc->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenListItc->func.del = NULL;

	for (nIndex = 0; nIndex < nVideoListSize; nIndex++)
	{
		elm_genlist_item_append(pObjVideosList, g_pListRemoveViewHandle->pGenListItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW	//
static void __mp_remove_view_append_grid_items(void *pObjVideosList, int nGenItemIndex)
{
	if (!g_pListRemoveViewHandle)
	{
		VideoLogInfo("invalid handle");
		return;
	}
	MP_DEL_GRID_ITC(g_pListRemoveViewHandle->pGenGridItc_1);
	MP_DEL_GRID_ITC(g_pListRemoveViewHandle->pGenGridItc_2);
	g_pListRemoveViewHandle->pGenGridItc_1 = elm_gengrid_item_class_new();
	g_pListRemoveViewHandle->pGenGridItc_1->item_style = "video/album_grid";
	g_pListRemoveViewHandle->pGenGridItc_1->func.text_get = (void*)__mp_remove_view_get_label_of_grid_item_cb;
	g_pListRemoveViewHandle->pGenGridItc_1->func.content_get = (void*)__mp_remove_view_get_grid_icon_cb;
	g_pListRemoveViewHandle->pGenGridItc_1->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenGridItc_1->func.del = NULL;

	int nCount = (mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle()) ? VP_GENLIST_THUMB_LANDSPACE_COUNT : VP_GENLIST_THUMB_PORTRAIT_COUNT);
	int nIndex = nGenItemIndex * nCount;
	int nMaxIndex = nIndex + nCount;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	Elm_Object_Item *gridItem = NULL;
	for (; (nIndex < nMaxIndex) && (nIndex < nVideoListSize); nIndex++) {
		gridItem = elm_gengrid_item_append(pObjVideosList, g_pListRemoveViewHandle->pGenGridItc_1, (void *)nIndex, __mp_remove_view_select_grid_item_cb, (void *)nIndex);
		elm_gengrid_item_select_mode_set(gridItem, ELM_OBJECT_SELECT_MODE_ALWAYS);
		elm_object_item_data_set(gridItem, (void *)nIndex);
	}
}

static Evas_Object *__mp_remove_view_create_gengrid(Evas_Object *parent, int nWidth, int nHeight)
{
	Evas_Object *pGengrid = NULL;
	pGengrid = elm_gengrid_add(parent);
	if (!pGengrid) {
		VideoLogError("pVideoGrid does not exist.");
		return NULL;
	}
	evas_object_size_hint_weight_set(pGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pGengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_gengrid_item_size_set(pGengrid, nWidth, nHeight);
	elm_gengrid_align_set(pGengrid, 0.0, 0.0);
	elm_gengrid_horizontal_set(pGengrid, EINA_TRUE);

	return pGengrid;
}

static void mp_remove_view_get_icon_dimensions(int *height, int *width)
{
	int winHeight = 0;
	int winWidth = 0;
	elm_win_screen_size_get(mp_util_get_main_window_handle(), NULL, NULL,
				&winWidth, &winHeight);

	bool bLandscape = mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());
	if (bLandscape) {
		*width = winHeight / VP_GENLIST_THUMB_LANDSPACE_COUNT;
	} else {
		*width = winWidth / VP_GENLIST_THUMB_PORTRAIT_COUNT;
	}
	*height = VP_ALBUM_THUMB_ICON_HEIGHT * MP_SCALE;
}

static Evas_Object *__mp_remove_view_get_icon_of_grid_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	if (!g_pListRemoveViewHandle || !pObject) {
		VideoLogError("g_pMainViewWidgetOfShareList is NULL");
		return NULL;
	}

	int nGenItemIndex = (int)pUserData;
	int nHeight = 0;
	int nWidth = 0;
	mp_remove_view_get_icon_dimensions(&nHeight, &nWidth);

	Evas_Object *pGengrid = __mp_remove_view_create_gengrid(pObject, nWidth, nHeight);
	__mp_remove_view_append_grid_items(pGengrid, nGenItemIndex);

	return pGengrid;
}

int mp_remove_view_thumbnail_to_genlist_index(int nVideoItemIndex)
{
	if (nVideoItemIndex < 0) {
		VideoLogDebug("invalid nVideoItemIndex");
		return 0;
	}
	int nGenlistIndex = 0;
	int nItemCount = (mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle()) ? VP_GENLIST_THUMB_LANDSPACE_COUNT : VP_GENLIST_THUMB_PORTRAIT_COUNT);
	nGenlistIndex = nVideoItemIndex/nItemCount;
	return nGenlistIndex;
}

static bool __mp_remove_view_append_grid_layout(void *pGenlist)
{
	if (!pGenlist || !g_pListRemoveViewHandle) {
		VideoLogError("error handle.");
		return FALSE;
	}
	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	if (nVideoListSize <= 0) {
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);
		return FALSE;
	}
	else
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);

	MP_DEL_ITC(g_pListRemoveViewHandle->pGenListItc);
	g_pListRemoveViewHandle->pGenListItc = elm_genlist_item_class_new();
	g_pListRemoveViewHandle->pGenListItc->item_style = "video/1icon.thumbnail";
	g_pListRemoveViewHandle->pGenListItc->func.text_get = NULL;
	g_pListRemoveViewHandle->pGenListItc->func.content_get = (void *)__mp_remove_view_get_icon_of_grid_cb;
	g_pListRemoveViewHandle->pGenListItc->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenListItc->func.del = NULL;
	g_pListRemoveViewHandle->pGenListItc->decorate_item_style = NULL;

	MP_DEL_ITC(g_pListRemoveViewHandle->pGenListLandscapeItc);
	g_pListRemoveViewHandle->pGenListLandscapeItc = elm_genlist_item_class_new();
	g_pListRemoveViewHandle->pGenListLandscapeItc->item_style = "video/1icon.thumbnail.landscape";
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.text_get = NULL;
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.content_get = (void *)__mp_remove_view_get_icon_of_grid_cb;
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.del = NULL;
	g_pListRemoveViewHandle->pGenListLandscapeItc->decorate_item_style = NULL;

	int nGenlistRow = 0;
	nGenlistRow = mp_remove_view_thumbnail_to_genlist_index(nVideoListSize - 1);

	if (mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle()))
	{
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist, g_pListRemoveViewHandle->pGenListLandscapeItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	} else {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist, g_pListRemoveViewHandle->pGenListItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}

	return TRUE;
}

static void __mp_remove_view_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pListRemoveViewHandle) {
		VideoLogError("__mp_remove_view_rotate_cb IS null");
		return;
	}
	bool bLandscape = mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());
	if (bLandscape) {
		VideoLogInfo("Landscape mode");
		g_pListRemoveViewHandle->bLandscape = true;
	} else {
		VideoLogInfo("Portrait mode");
		g_pListRemoveViewHandle->bLandscape = false;
	}
	if (!g_pListRemoveViewHandle->bFolderDel)
		__mp_remove_view_arrange_video_list(g_pListRemoveViewHandle->pObjVideosList, LIST_TAB_TYPE_PERSONAL, TRUE);
	else
		__mp_remove_view_arrange_folder_list(g_pListRemoveViewHandle->pObjVideosList, LIST_TAB_TYPE_PERSONAL, TRUE);
	mp_remove_view_change_title_info();
}

static void __mp_remove_folder_view_append_grid_items(void *pObjVideosList, int nGenItemIndex)
{
	if (!g_pListRemoveViewHandle) {
		VideoLogInfo("invalid handle");
		return;
	}
	MP_DEL_GRID_ITC(g_pListRemoveViewHandle->pGenGridItc_1);
	g_pListRemoveViewHandle->pGenGridItc_1 = elm_gengrid_item_class_new();
	g_pListRemoveViewHandle->pGenGridItc_1->item_style = "video/album_grid";
	g_pListRemoveViewHandle->pGenGridItc_1->func.text_get = (void*)__mp_remove_view_get_label_of_folder_cb;
	g_pListRemoveViewHandle->pGenGridItc_1->func.content_get = (void*)__mp_remove_folder_view_get_grid_icon_cb;
	g_pListRemoveViewHandle->pGenGridItc_1->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenGridItc_1->func.del = NULL;

	int nCount = (mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle()) ? VP_GENLIST_THUMB_LANDSPACE_COUNT : VP_GENLIST_THUMB_PORTRAIT_COUNT);
	int nIndex = nGenItemIndex * nCount;
	int nMaxIndex = nIndex + nCount;
	int nFolderListSize = mp_util_svc_get_video_folder_size();

	Elm_Object_Item *gridItem = NULL;
	for (; (nIndex < nMaxIndex) && (nIndex < nFolderListSize); nIndex++) {
		gridItem = elm_gengrid_item_append(pObjVideosList, g_pListRemoveViewHandle->pGenGridItc_1, (void *)nIndex, __mp_remove_view_select_grid_item_cb, (void *)nIndex);
		elm_gengrid_item_select_mode_set(gridItem, ELM_OBJECT_SELECT_MODE_ALWAYS);
		elm_object_item_data_set(gridItem, (void *)nIndex);
	}
}

static Evas_Object *__mp_remove_folder_view_get_icon_of_grid_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	if (!g_pListRemoveViewHandle || !pObject) {
		VideoLogError("g_pMainViewWidgetOfShareList is NULL");
		return NULL;
	}

	int nGenItemIndex = (int)pUserData;
	int nHeight = 0;
	int nWidth = 0;
	mp_remove_view_get_icon_dimensions(&nHeight, &nWidth);

	Evas_Object *pGengrid = __mp_remove_view_create_gengrid(pObject, nWidth, nHeight);
	__mp_remove_folder_view_append_grid_items(pGengrid, nGenItemIndex);

	return pGengrid;
}

static bool __mp_remove_view_append_folder_grid_layout(void *pGenlist)
{
	if (!pGenlist || !g_pListRemoveViewHandle) {
		VideoLogError("error handle.");
		return FALSE;
	}
	int nIndex = 0;
	int nFolderListSize = mp_util_svc_get_video_folder_size();
	if (nFolderListSize <= 0) {
		VideoLogWarning("nFolderListSize = %d", nFolderListSize);
		return FALSE;
	}
	else
		VideoLogWarning("nFolderListSize = %d", nFolderListSize);

	MP_DEL_ITC(g_pListRemoveViewHandle->pGenListItc);
	g_pListRemoveViewHandle->pGenListItc = elm_genlist_item_class_new();
	g_pListRemoveViewHandle->pGenListItc->item_style = "video/1icon.thumbnail";
	g_pListRemoveViewHandle->pGenListItc->func.text_get = NULL;
	g_pListRemoveViewHandle->pGenListItc->func.content_get = (void *)__mp_remove_folder_view_get_icon_of_grid_cb;
	g_pListRemoveViewHandle->pGenListItc->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenListItc->func.del = NULL;
	g_pListRemoveViewHandle->pGenListItc->decorate_item_style = NULL;

	MP_DEL_ITC(g_pListRemoveViewHandle->pGenListLandscapeItc);
	g_pListRemoveViewHandle->pGenListLandscapeItc = elm_genlist_item_class_new();
	g_pListRemoveViewHandle->pGenListLandscapeItc->item_style = "video/1icon.thumbnail.landscape";
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.text_get = NULL;
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.content_get = (void *)__mp_remove_folder_view_get_icon_of_grid_cb;
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenListLandscapeItc->func.del = NULL;
	g_pListRemoveViewHandle->pGenListLandscapeItc->decorate_item_style = NULL;

	int nGenlistRow = 0;
	nGenlistRow = mp_remove_view_thumbnail_to_genlist_index(nFolderListSize - 1);

	if (mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle()))
	{
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist, g_pListRemoveViewHandle->pGenListLandscapeItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	} else {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist, g_pListRemoveViewHandle->pGenListItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}

	return TRUE;
}
#endif

/*static void __mp_remove_view_append_folder_items(void *pObjVideosList)
{
	if (!g_pListRemoveViewHandle)
	{
		VideoLogInfo("invalid handle");
		return;
	}

	int nIndex = 0;
	int nFolderListSize = mp_util_svc_get_video_folder_size();
	MP_DEL_ITC(g_pListRemoveViewHandle->pGenListItc);
	g_pListRemoveViewHandle->pGenListItc = elm_genlist_item_class_new();

	g_pListRemoveViewHandle->pGenListItc->item_style = "2line.top";	//"video/edit_folder";
	g_pListRemoveViewHandle->pGenListItc->func.text_get = (void*)__mp_remove_view_get_label_of_folder_cb;
	g_pListRemoveViewHandle->pGenListItc->func.content_get = (void*)__mp_remove_view_get_icon_of_folder_cb;
	g_pListRemoveViewHandle->pGenListItc->func.state_get = NULL;
	g_pListRemoveViewHandle->pGenListItc->func.del = NULL;

	for (nIndex = 0; nIndex < nFolderListSize; nIndex++)
	{
		elm_genlist_item_append(pObjVideosList, g_pListRemoveViewHandle->pGenListItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, __mp_remove_view_select_video_item_cb, NULL);
	}
}*/

void mp_remove_view_set_value(int nTotalSize)
{
	VideoLogInfo("");

	if (g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved)
	{
		free(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved);
		g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved = NULL;
	}
	if (nTotalSize > 0) {
		g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved = (bool*)malloc(sizeof(bool) * nTotalSize);
		memset(g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved, 0, sizeof(bool) * nTotalSize);
	}

	g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
}

static bool __mp_remove_view_arrange_folder_list(void *pObjVideosList, MpVideoListTabType eTabType, bool bNeedSvc)
{
	if (!g_pListRemoveViewWidget || !g_pListRemoveViewHandle)
	{
		VideoLogError("No exist g_pRemoveListViewWidget.");
		return FALSE;
	}

	if (!pObjVideosList)
	{
		VideoLogError("No exist genlist object.");
		return FALSE;
	}
	int nFolderListSize = 0;

	mp_util_svc_destory_video_folder_list();
	mp_util_svc_extract_video_folder_list_by_type(mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_PHONE);
	nFolderListSize = mp_util_svc_get_video_folder_size();

	if (bNeedSvc)
	{
		//__mp_remove_view_update_selected_video_list();
	}
	else
	{
		mp_remove_view_set_value(nFolderListSize);
	}


	if (LIST_TAB_TYPE_PERSONAL == eTabType)
	{
		VideoLogInfo("nFolderListSize : %d", nFolderListSize);

		elm_genlist_clear(pObjVideosList);

		if (nFolderListSize > 0)
		{
			//__mp_remove_view_append_folder_items(pObjVideosList);
			__mp_remove_view_append_folder_grid_layout(pObjVideosList);
		}
		else
		{
			elm_naviframe_item_pop(g_pListRemoveViewHandle->pNaviFrameHandle);
			return FALSE;
		}
	}
	else
	{
		VideoLogError("invalid Tabbar type!");
	}

	elm_genlist_decorate_mode_set(pObjVideosList, EINA_TRUE);

	evas_object_show(pObjVideosList);
	return TRUE;
}

static bool __mp_remove_view_arrange_video_list(void *pObjVideosList, MpVideoListTabType eTabType, bool bNeedSvc)
{
	if (!g_pListRemoveViewWidget || !g_pListRemoveViewHandle)
	{
		VideoLogError("No exist g_pRemoveListViewWidget.");
		return FALSE;
	}

	if (!pObjVideosList)
	{
		VideoLogError("No exist genlist object.");
		return FALSE;
	}

	int nVideoListSize = 0;

	if (LIST_TAB_TYPE_PERSONAL == eTabType)
	{
		int nSortType = mp_remove_view_get_sort_type();
		int nViewType = mp_view_as_ctrl_get_type();
		if ((nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) && g_pListRemoveViewHandle->pFolderPath)
		{
			if (g_pListRemoveViewHandle->pFolderPath)
			{
				VideoSecureLogInfo("szFolderPath : %s", g_pListRemoveViewHandle->pFolderPath);
				if (vp_file_exists(g_pListRemoveViewHandle->pFolderPath))
				{
					//update video-list
					mp_util_svc_destory_video_item_list();
					mp_util_svc_extract_video_list_from_folder(g_pListRemoveViewHandle->pFolderPath, nSortType, MP_LIST_VIEW_PHONE);
					if (bNeedSvc)
					{
						//__mp_remove_view_update_selected_video_list();
					}
					else
					{
						nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
						mp_remove_view_set_value(nVideoListSize);
					}
				}
				else
				{
					elm_naviframe_item_pop(g_pListRemoveViewHandle->pNaviFrameHandle);
					return FALSE;
				}
			}
		}
		else
		{
			mp_util_svc_destory_video_list();
			mp_util_svc_extract_video_list_by_item_type(nSortType, MP_LIST_VIEW_PHONE);
			if (bNeedSvc)
			{
				//__mp_remove_view_update_selected_video_list();
			}
			else
			{
				nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
				mp_remove_view_set_value(nVideoListSize);
			}
		}

		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		VideoLogInfo("nVideoListSize : %d", nVideoListSize);

		if (nVideoListSize > 0)
		{
#ifdef VS_FEATURE_THUMBNAIL_VIEW	//
			if (g_pListRemoveViewHandle->bThumbnailView)
			{
				elm_genlist_clear(pObjVideosList);
				__mp_remove_view_append_grid_layout(pObjVideosList);
			}
			else
#endif
			{
				elm_genlist_clear(pObjVideosList);
				mp_remove_view_append_video_items(pObjVideosList);
			}
		}
		else
		{
			elm_naviframe_item_pop(g_pListRemoveViewHandle->pNaviFrameHandle);
			return FALSE;
		}
	}
	else
	{
		VideoLogError("invalid Tabbar type!");
	}
#ifdef VS_FEATURE_THUMBNAIL_VIEW_TODO	//
	if (!g_pListRemoveViewHandle->bThumbnailView) {
		elm_genlist_decorate_mode_set(pObjVideosList, EINA_TRUE);
	}
#endif	//
	evas_object_show(pObjVideosList);
	return TRUE;
}

Evas_Object *mp_remove_view_init_internal_layout(void *pParent)
{
	if (!pParent)
	{
		VideoLogError("[ERR]");
		return NULL;
	}

	VideoLogInfo("");

	if (g_pListRemoveViewWidget->pRemoveViewBaselayout)
	{
		evas_object_del(g_pListRemoveViewWidget->pRemoveViewBaselayout);
		g_pListRemoveViewWidget->pRemoveViewBaselayout = NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_REMOVE_LIST_EDJ);
	free(path);
	g_pListRemoveViewWidget->pRemoveViewBaselayout = elm_layout_add(pParent);
	elm_layout_file_set(g_pListRemoveViewWidget->pRemoveViewBaselayout, edj_path, REMOVE_LIST_EDJ_GROUP);
	evas_object_size_hint_weight_set(g_pListRemoveViewWidget->pRemoveViewBaselayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pListRemoveViewWidget->pRemoveViewBaselayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return g_pListRemoveViewWidget->pRemoveViewBaselayout;
}

static bool __mp_remove_view_get_selected_video_list(int nSelectedSize)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return FALSE;
	}

	int nVideoListSize = 0;

	if (LIST_TAB_TYPE_PERSONAL != g_pListRemoveViewHandle->eTabType)
	{
		return FALSE;
	}

	if (!g_pListRemoveViewHandle->nSelectCounterForEditList || nSelectedSize < 1)
	{
		return FALSE;
	}

	if (g_pListRemoveViewHandle->bFolderDel)
	{
		nVideoListSize = mp_util_svc_get_video_folder_size();
	}
	else
	{
		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	}

	__mp_remove_view_free_selected_list();

	int nIndex = 0;
	int nCounter = 0;
	char *pMediaId = NULL;

	g_pListRemoveViewHandle->pSelectedList = (char**) calloc (sizeof(char*), nSelectedSize);
	VideoLogInfo("current edit size===%d", nSelectedSize);

	for (nCounter = 0; nCounter <nVideoListSize;  nCounter++)
	{
		if (g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nCounter])
		{
			if (g_pListRemoveViewHandle->bFolderDel)
			{
				pMediaId = mp_util_svc_get_video_folder_id(nCounter);
			}
			else
			{
				pMediaId = mp_util_svc_get_video_id_by_index(nCounter);
			}
			if (pMediaId != NULL)
			{
				g_pListRemoveViewHandle->pSelectedList[nIndex] = strdup(pMediaId);
				MP_FREE_STRING(pMediaId);
				nIndex++;
			}
		}
	}
	return TRUE;
}

/*static void __mp_remove_view_update_select_all(void)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nItemListSize = 0;
	if (g_pListRemoveViewHandle->bFolderDel)
	{
		nItemListSize = mp_util_svc_get_video_folder_size();
	}
	else
	{
		nItemListSize = mp_util_svc_get_number_of_video_item_by_type();
	}

	if (nItemListSize == g_pListRemoveViewHandle->nSelectCounterForEditList)
	{
		VideoLogInfo("All check box of edit list state are true.");
		g_pListRemoveViewHandle->bSelectallCheckBoxState = TRUE;
	}
	else
	{
		g_pListRemoveViewHandle->bSelectallCheckBoxState = FALSE;
	}

	VideoLogInfo("nItemListSize=%d,nSelectCounterForEditList=%d", nItemListSize, g_pListRemoveViewHandle->nSelectCounterForEditList);
	elm_check_state_set(g_pListRemoveViewWidget->pSelectallbtn, g_pListRemoveViewHandle->bSelectallCheckBoxState);
}

static void __mp_remove_view_update_selected_video_list(void)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nVideoListSize = 0;

	if (LIST_TAB_TYPE_PERSONAL != g_pListRemoveViewHandle->eTabType)
	{
		return;
	}


	MpMediaType eMediaType = MEDIA_TYPE_FILE;

	if (g_pListRemoveViewHandle->bFolderDel)
	{
		eMediaType = MEDIA_TYPE_DIRECTORY;
		nVideoListSize = mp_util_svc_get_video_folder_size();
	}
	else
	{
		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	}

	int nIndex = 0;
	int nCounter = 0;
	bool bValidMedia = FALSE;
	int nTotal = g_pListRemoveViewHandle->nSelectCounterForEditList;
	VideoLogInfo("OldEditList ===%d", nTotal);
	g_pListRemoveViewHandle->nSelectCounterForEditList = 0;
	mp_remove_view_set_value(nVideoListSize);

	if (!g_pListRemoveViewHandle->pSelectedList)
	{
		return;
	}

	while ((nIndex < nTotal) && g_pListRemoveViewHandle->pSelectedList[nIndex])
	{

		bValidMedia = mp_util_svc_check_valid_media_id(g_pListRemoveViewHandle->pSelectedList[nIndex], eMediaType, &nCounter);
		if (bValidMedia)
		{
			g_pListRemoveViewHandle->pCheckedItemsEditListForRemoved[nCounter] = TRUE;
			g_pListRemoveViewHandle->nSelectCounterForEditList ++;
		}
		nIndex++;
	}

	 //update select all button state
	__mp_remove_view_update_select_all();

	mp_remove_view_change_title_info();

	if (g_pListRemoveViewHandle->nSelectCounterForEditList > 0)
		elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_FALSE);
	else
		elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_TRUE);
}*/

static void __mp_remove_view_db_changed_cb(void *pUserData)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	bool bNormalShow = TRUE;
	if (g_pListRemoveViewHandle->bFolderDel) {
		bNormalShow = __mp_remove_view_arrange_folder_list(g_pListRemoveViewHandle->pObjVideosList, LIST_TAB_TYPE_PERSONAL, TRUE);
		if (!bNormalShow)
		{
			VideoLogError("invalid view");
			return;
		}
	} else {
		bNormalShow = __mp_remove_view_arrange_video_list(g_pListRemoveViewHandle->pObjVideosList, LIST_TAB_TYPE_PERSONAL, TRUE);
		if (!bNormalShow)
		{
			VideoLogError("invalid view");
			return;
		}
	}

	Elm_Object_Item *pItem = NULL;
#ifdef VS_FEATURE_THUMBNAIL_VIEW_TODO	//
	if (g_pListRemoveViewHandle->bThumbnailView)
	{
		pItem = elm_gengrid_first_item_get(g_pListRemoveViewHandle->pObjVideosList);
		if (pItem)
		{
			elm_gengrid_item_bring_in(pItem, ELM_GENGRID_ITEM_SCROLLTO_IN);
		}
	}
	else
#endif	//
	{
		pItem = elm_genlist_first_item_get(g_pListRemoveViewHandle->pObjVideosList);
		if (pItem)
		{
			elm_genlist_item_bring_in(pItem, ELM_GENLIST_ITEM_SCROLLTO_IN);
		}
	}

}

static void __mp_remove_view_db_backup_cb(void *pUserData)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}
	__mp_remove_view_get_selected_video_list(g_pListRemoveViewHandle->nSelectCounterForEditList);

}

static void __mp_remove_select_all_layout_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	Evas_Object *check = elm_object_part_content_get(g_pListRemoveViewWidget->pSelAllLayout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	__mp_remove_view_select_all_change_check_box_cb(data, check, NULL);
}

void mp_remove_view_init(void *pParent, MpVideoListTabType eTabType)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");
	g_pListRemoveViewWidget->pRemoveViewBaselayout = mp_remove_view_init_internal_layout(pParent);
	if (!g_pListRemoveViewWidget->pRemoveViewBaselayout) {
		VideoLogError("pRemoveViewBaselayout is not existed.");
		return;
	}

	evas_object_event_callback_add(g_pListRemoveViewWidget->pRemoveViewBaselayout, EVAS_CALLBACK_DEL, __mp_remove_view_base_layout_del_cb, NULL);

	g_pListRemoveViewWidget->pVideoListBox = elm_box_add(g_pListRemoveViewWidget->pRemoveViewBaselayout);
	if (!g_pListRemoveViewWidget->pVideoListBox) {
		VideoLogError("pVideoListBox is not existed.");
		return;
	}

	evas_object_size_hint_weight_set(g_pListRemoveViewWidget->pVideoListBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pListRemoveViewWidget->pVideoListBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

	mp_widget_ctrl_create_select_all_layout(g_pListRemoveViewWidget->pVideoListBox, __mp_remove_view_select_all_change_check_box_cb, __mp_remove_select_all_layout_down_cb, (void *)g_pListRemoveViewWidget,
		&g_pListRemoveViewWidget->pSelectallbtn, &g_pListRemoveViewWidget->pSelAllLayout);

	if (!g_pListRemoveViewWidget->pSelAllLayout) {
		VideoLogError("pSelAllLayout is not existed.");
		return;
	}

	elm_box_pack_start(g_pListRemoveViewWidget->pVideoListBox, g_pListRemoveViewWidget->pSelAllLayout);

#ifdef VS_FEATURE_THUMBNAIL_VIEW_TODO	//
	if (g_pListRemoveViewHandle->bThumbnailView)
	{
		g_pListRemoveViewHandle->pObjVideosList = elm_gengrid_add(g_pListRemoveViewWidget->pVideoListBox);
		if (!g_pListRemoveViewHandle->pObjVideosList) {
			VideoLogError("pVideoGrid is not existed.");
			return;
		}
		evas_object_size_hint_weight_set(g_pListRemoveViewHandle->pObjVideosList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(g_pListRemoveViewHandle->pObjVideosList, EVAS_HINT_FILL, EVAS_HINT_FILL);
		int nHeight = VP_ALBUM_THUMB_ICON_HEIGHT * MP_SCALE;
		int nWidth = VIDEO_GENGRID_ICON_WIDTH * MP_SCALE;
		elm_gengrid_item_size_set(g_pListRemoveViewHandle->pObjVideosList, nWidth, nHeight);
		elm_gengrid_align_set(g_pListRemoveViewHandle->pObjVideosList, 0.0, 0.0);
		elm_gengrid_horizontal_set(g_pListRemoveViewHandle->pObjVideosList, EINA_FALSE);
		//elm_gengrid_multi_select_set(g_pListRemoveViewHandle->pObjVideosList, EINA_TRUE);
	}
	else
#endif	//
	{
		g_pListRemoveViewHandle->pObjVideosList = elm_genlist_add(g_pListRemoveViewWidget->pVideoListBox);
		if (!g_pListRemoveViewHandle->pObjVideosList) {
			VideoLogError("pObjVideosList is not existed.");
			return;
		}
		elm_scroller_bounce_set(g_pListRemoveViewHandle->pObjVideosList, EINA_FALSE, EINA_TRUE);
		evas_object_size_hint_weight_set(g_pListRemoveViewHandle->pObjVideosList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(g_pListRemoveViewHandle->pObjVideosList, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_genlist_block_count_set(g_pListRemoveViewHandle->pObjVideosList, VIDEO_GENLIST_BLOCK_COUNT);
		elm_genlist_homogeneous_set(g_pListRemoveViewHandle->pObjVideosList, EINA_TRUE);
		elm_genlist_mode_set(g_pListRemoveViewHandle->pObjVideosList, ELM_LIST_COMPRESS);
	}
	g_pListRemoveViewHandle->eTabType = eTabType;

	elm_box_pack_end(g_pListRemoveViewWidget->pVideoListBox, g_pListRemoveViewHandle->pObjVideosList);
	elm_object_part_content_set(g_pListRemoveViewWidget->pRemoveViewBaselayout, SWALLOW_LISTVIEW_CONTENT, g_pListRemoveViewWidget->pVideoListBox);

	bool bNormalShow = TRUE;

	if (g_pListRemoveViewHandle->bFolderDel) {
		bNormalShow = __mp_remove_view_arrange_folder_list(g_pListRemoveViewHandle->pObjVideosList, eTabType, FALSE);
		if (!bNormalShow)
		{
			VideoLogError("invalid view");
			return;
		}
	} else {
		bNormalShow = __mp_remove_view_arrange_video_list(g_pListRemoveViewHandle->pObjVideosList, eTabType, FALSE);
		if (!bNormalShow)
		{
			VideoLogError("invalid view");
			return;
		}
	}

	char *szTitle = g_strdup_printf(VIDEOS_SELECTVIEW_HEADER_PD_SELECTED, g_pListRemoveViewHandle->nSelectCounterForEditList);

	g_pListRemoveViewHandle->pNaviFrameItem = elm_naviframe_item_push(g_pListRemoveViewHandle->pNaviFrameHandle, szTitle, NULL, NULL, g_pListRemoveViewWidget->pRemoveViewBaselayout, NULL);
	elm_object_item_domain_text_translatable_set(g_pListRemoveViewHandle->pNaviFrameItem, VIDEOS_STRING, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(g_pListRemoveViewHandle->pNaviFrameItem, __mp_remove_view_back_btn_cb, g_pListRemoveViewHandle);

	elm_naviframe_item_title_enabled_set(g_pListRemoveViewHandle->pNaviFrameItem, EINA_TRUE, EINA_FALSE);
	evas_object_smart_callback_add(g_pListRemoveViewHandle->pNaviFrameHandle, "transition,finished", mp_remove_view_naviframe_transition_effect_cb, NULL);

	/*g_pListRemoveViewWidget->pCancelbtn = mp_widget_ctrl_create_navi_left_btn(g_pListRemoveViewHandle->pNaviFrameHandle, g_pListRemoveViewHandle->pNaviFrameItem, NULL, __mp_remove_view_run_cancel_items_cb, (void *)g_pListRemoveViewHandle);
	g_pListRemoveViewWidget->pSavebtn = mp_widget_ctrl_create_navi_right_btn(g_pListRemoveViewHandle->pNaviFrameHandle, g_pListRemoveViewHandle->pNaviFrameItem, NULL, __mp_remove_view_run_delete_items_cb, (void *)g_pListRemoveViewHandle);*/

	g_pListRemoveViewWidget->pCancelbtn = elm_button_add(g_pListRemoveViewHandle->pNaviFrameHandle);
	elm_object_style_set(g_pListRemoveViewWidget->pCancelbtn, "naviframe/title_left");
	//elm_object_text_set(g_pListRemoveViewWidget->pCancelbtn, VIDEOS_DELETEVIEW_CANCEL_IDS);
	mp_util_set_translate_str(g_pListRemoveViewWidget->pCancelbtn, "IDS_TPLATFORM_ACBUTTON_CANCEL_ABB");
	elm_object_signal_callback_add(g_pListRemoveViewWidget->pCancelbtn, "elm,action,click", "", __mp_remove_view_run_cancel_items_cb, (void *)g_pListRemoveViewHandle);
	elm_object_item_part_content_set(g_pListRemoveViewHandle->pNaviFrameItem, "title_left_btn", g_pListRemoveViewWidget->pCancelbtn);
	evas_object_show(g_pListRemoveViewWidget->pCancelbtn);

	g_pListRemoveViewWidget->pSavebtn = elm_button_add(g_pListRemoveViewHandle->pNaviFrameHandle);
	elm_object_style_set(g_pListRemoveViewWidget->pSavebtn, "naviframe/title_right");
	//elm_object_text_set(g_pListRemoveViewWidget->pSavebtn, VIDEOS_DELETEVIEW_DELETE_IDS);
	mp_util_set_translate_str(g_pListRemoveViewWidget->pSavebtn, "IDS_TPLATFORM_ACBUTTON_DELETE_ABB");
	elm_object_signal_callback_add(g_pListRemoveViewWidget->pSavebtn, "elm,action,click", "", __mp_remove_view_run_delete_items_cb, (void *)g_pListRemoveViewHandle);
	elm_object_item_part_content_set(g_pListRemoveViewHandle->pNaviFrameItem, "title_right_btn", g_pListRemoveViewWidget->pSavebtn);
	evas_object_show(g_pListRemoveViewWidget->pSavebtn);

	elm_object_disabled_set(g_pListRemoveViewWidget->pSavebtn, EINA_TRUE);

	evas_object_show(g_pListRemoveViewHandle->pObjVideosList);
	evas_object_show(g_pListRemoveViewWidget->pRemoveViewBaselayout);

	mp_util_db_set_update_fun(g_pListRemoveViewHandle->euLev, __mp_remove_view_db_changed_cb);
	mp_util_db_set_backup_fun(g_pListRemoveViewHandle->euLev, __mp_remove_view_db_backup_cb);

	MP_FREE_STRING(szTitle);
	mp_util_hide_indicator();
}

void mp_remove_view_push(void *pNaviFrame, ListRemoveViewCbFunc pUpdateViewCb, ChangeListRemoveViewCbFunc pChangeViewCb, char *pFolderPath, MpVideoListTabType eTabType)
{
	VideoLogInfo("");
	mp_remove_view_destroy();
	g_pListRemoveViewHandle = (st_VideoListRemoveViewHandle*)calloc(1, sizeof(st_VideoListRemoveViewHandle));
	g_pListRemoveViewWidget = (st_VideoListRemoveViewWidget*)calloc(1, sizeof(st_VideoListRemoveViewWidget));

	memset(g_pListRemoveViewHandle, 0, sizeof(st_VideoListRemoveViewHandle));
	memset(g_pListRemoveViewWidget, 0, sizeof(st_VideoListRemoveViewWidget));
	mp_remove_view_reset_value();

	g_pListRemoveViewHandle->pNaviFrameHandle = pNaviFrame;
	g_pListRemoveViewHandle->pChangeViewUserCbFunc = pChangeViewCb;
	g_pListRemoveViewHandle->pChangeViewUpdateFunc = pUpdateViewCb;

	if (pFolderPath) {
		g_pListRemoveViewHandle->pFolderPath = strdup(pFolderPath);
	}
	g_pListRemoveViewHandle->bUpdateList = FALSE;
	g_pListRemoveViewHandle->euLev = MP_DB_UPDATE_LEV_1;

	int nViewType = mp_view_as_ctrl_get_type();
	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST && g_pListRemoveViewHandle->pFolderPath) {
		g_pListRemoveViewHandle->euLev = MP_DB_UPDATE_LEV_2;
	}

	if ((nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) && (!g_pListRemoveViewHandle->pFolderPath) && (LIST_TAB_TYPE_PERSONAL == eTabType)) {
		g_pListRemoveViewHandle->bFolderDel = TRUE;
	} else {
		g_pListRemoveViewHandle->bFolderDel = FALSE;
	}
#ifdef VS_FEATURE_THUMBNAIL_VIEW	//
	if ((nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST || nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) && (LIST_TAB_TYPE_PERSONAL == eTabType)) {
		g_pListRemoveViewHandle->bThumbnailView = TRUE;
	} else {
		g_pListRemoveViewHandle->bThumbnailView = FALSE;
	}
#endif	//
	mp_remove_view_init(g_pListRemoveViewHandle->pNaviFrameHandle, eTabType);
	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_remove_view_rotate_cb, (void*)g_pListRemoveViewHandle);
}

void mp_remove_view_pop(void)
{
	VideoLogInfo("");
	if (!g_pListRemoveViewHandle)
	{
		VideoLogError("No existed handle of remove view.");
		return;
	}

	// KEEP BELOW CALLING SEQUENCE OF FUNCTION.
	bool bUpdateList = FALSE;
	if (g_pListRemoveViewHandle->bUpdateList)
	{
		if (g_pListRemoveViewHandle->pChangeViewUpdateFunc)
		{
			bUpdateList = TRUE;
			g_pListRemoveViewHandle->pChangeViewUpdateFunc(LIST_UPDATE_TYPE_ALL);
		}
	}

	if (!bUpdateList)
	{
		if (g_pListRemoveViewHandle->pChangeViewUserCbFunc)
		{
			g_pListRemoveViewHandle->pChangeViewUserCbFunc();
		}
	}

}

bool mp_remove_view_is_top_view(void)
{
	VideoLogInfo("");

	return FALSE;
}

void mp_remove_view_change_language(void)
{
	if (!g_pListRemoveViewHandle || !g_pListRemoveViewWidget)
	{
		VideoLogError("No existed handle of remove view.");
		return;
	}

	VideoLogInfo("");

	mp_remove_view_change_title_info();
}


static void
__mp_remove_view_base_layout_del_cb(void *pUserData, Evas *e, Evas_Object *pObject, void *pEventInfo)
{
	//evas_object_event_callback_del(g_pListRemoveViewWidget->pRemoveViewBaselayout, EVAS_CALLBACK_DEL, __mp_remove_view_base_layout_del_cb);

	__mp_remove_view_reset();
}

