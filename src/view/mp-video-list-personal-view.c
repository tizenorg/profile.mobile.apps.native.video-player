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
#include <app.h>
#include <app_preference.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-external-ug.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-util-media-service.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-list-personal-ctrl.h"
#include "mp-video-list-personal-view.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-util-db-controller.h"
#include "mp-util-config.h"
#include "vp-file-util.h"


#define PREF_MP_UTIL_PERSONAL_NO_ASK_AGAIN "preference/org.tizen.videos/personal_no_ask_again"
#define VIDEO_SELECTED_INFO_TIMER		4.0


typedef struct {
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	void *pVideosGenlist;

	bool *pCheckedItemsEditListForRemoved;
	char **pSelectedList;	/*save selected video id*/
	bool bSelectallCheckBoxState;
	int nSelectCounterForEditList;
	MpMediaSvcSortType nSortType;
	ChangePersonalViewCbFunc pChangeViewUserCbFunc;
	char *pFolderPath;
	int nViewStyle;		/*0:add to personal. //1,remove from personal*/
	bool bFolderMove;
	bool bCheckBoxState;
	MpDbUpdateViewLev euLev;

} st_VideoListPersonalViewHandle;

typedef struct {
	/*Evas_Object *pSelectionInfoLayout;*/
	Evas_Object *pVideoListBox;
	Evas_Object *pPersonalViewBaselayout;
	Evas_Object *pSelectallbtn;
	Evas_Object *pSelAllLayout;
	Evas_Object *pSavebtn;
	Evas_Object *pCancelbtn;

	Ecore_Timer *pSelectedInfoLayoutTimer;
} st_VideoListPersonalViewWidget;


st_VideoListPersonalViewHandle *g_pListPersonalViewHandle = NULL;
st_VideoListPersonalViewWidget *g_pListPersonalViewWidget = NULL;


Elm_Genlist_Item_Class VideoPersonal_Itc;
Elm_Genlist_Item_Class FolderPersonal_Itc;
Elm_Genlist_Item_Class FolderPersonalNoItem_Itc;




/* Pre define function*/
int mp_personal_view_get_sort_type(void);
void mp_personal_view_arrange_video_list(int nItemSortType,
		void *pVideosGenlist,
		bool bNeedSvc);
void mp_personal_view_naviframe_transition_effect_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo);
void mp_personal_view_change_title_info(void);
static void __mp_personal_view_base_layout_del_cb(void *pUserData,
		Evas *e,
		Evas_Object *pObject,
		void *pEventInfo);
static void __mp_personal_view_ignore_personal_vconf();

static bool __mp_personal_view_get_selected_video_list(int nSelectedSize);
static void __mp_personal_view_update_selected_video_list(void);

static void __mp_personal_view_free_selected_list(void)
{
	if (!g_pListPersonalViewHandle
			|| !g_pListPersonalViewHandle->pSelectedList) {
		return;
	}
	int nIndex = 0;
	while (g_pListPersonalViewHandle->pSelectedList[nIndex]) {
		nIndex++;
		MP_FREE_STRING(g_pListPersonalViewHandle->pSelectedList[nIndex]);
	}
	MP_FREE_STRING(g_pListPersonalViewHandle->pSelectedList);
}

void mp_personal_view_update_all_check_box(void)
{
	VideoLogInfo("");
	int nPersonalSize = 0;
	if (g_pListPersonalViewHandle->bFolderMove) {
		nPersonalSize = mp_util_svc_get_video_folder_size();
	} else {
		nPersonalSize = mp_util_svc_get_number_of_video_item_by_type();
	}
	if (!g_pListPersonalViewHandle->pVideosGenlist) {
		VideoLogInfo("genlist is NULL");
		return;
	}
	if (nPersonalSize < 1) {
		VideoLogInfo("nVideoListSize < 1:%d", nPersonalSize);
		return;
	}
	Elm_Object_Item *currentItem = NULL;
	currentItem =
		elm_genlist_first_item_get(g_pListPersonalViewHandle->
					   pVideosGenlist);
	int nVideoItemIndex = 0;
	Evas_Object *check_box = NULL;
	while (currentItem && (nVideoItemIndex < nPersonalSize)) {
		check_box =
			elm_object_item_part_content_get(currentItem,
					"elm.edit.icon.1");
		if (check_box) {
			elm_check_state_set(check_box,
					    g_pListPersonalViewHandle->
					    pCheckedItemsEditListForRemoved
					    [nVideoItemIndex]);
		}
		currentItem = elm_genlist_item_next_get(currentItem);
		nVideoItemIndex++;
	}
}

static void __mp_personal_view_reset(void)
{
	VideoLogInfo("");
	__mp_personal_view_ignore_personal_vconf();

	if (g_pListPersonalViewHandle) {
		mp_util_db_set_update_fun(g_pListPersonalViewHandle->euLev, NULL);
		mp_util_db_set_backup_fun(g_pListPersonalViewHandle->euLev, NULL);
		evas_object_smart_callback_del(g_pListPersonalViewHandle->
					       pNaviFrameHandle,
					       "transition,finished",
					       mp_personal_view_naviframe_transition_effect_cb);
		__mp_personal_view_free_selected_list();

		g_pListPersonalViewHandle->pVideosGenlist = NULL;

		MP_FREE_STRING(g_pListPersonalViewHandle->
			       pCheckedItemsEditListForRemoved);
		MP_FREE_STRING(g_pListPersonalViewHandle->pFolderPath);

		free(g_pListPersonalViewHandle);
		g_pListPersonalViewHandle = NULL;
	}

	if (g_pListPersonalViewWidget) {

		MP_DEL_OBJ(g_pListPersonalViewWidget->pSelectallbtn);
		MP_DEL_OBJ(g_pListPersonalViewWidget->pSavebtn);
		MP_DEL_OBJ(g_pListPersonalViewWidget->pCancelbtn);

		g_pListPersonalViewWidget->pSelAllLayout = NULL;
		g_pListPersonalViewWidget->pVideoListBox = NULL;

		g_pListPersonalViewWidget->pPersonalViewBaselayout = NULL;

		MP_DEL_TIMER(g_pListPersonalViewWidget->pSelectedInfoLayoutTimer);

		free(g_pListPersonalViewWidget);
		g_pListPersonalViewWidget = NULL;
	}

}

void mp_personal_view_destroy(void)
{
	VideoLogInfo("");

	__mp_personal_view_ignore_personal_vconf();

	if (g_pListPersonalViewHandle) {
		evas_object_smart_callback_del(g_pListPersonalViewHandle->
					       pNaviFrameHandle,
					       "transition,finished",
					       mp_personal_view_naviframe_transition_effect_cb);
		__mp_personal_view_free_selected_list();

		MP_DEL_OBJ(g_pListPersonalViewHandle->pVideosGenlist);

		MP_FREE_STRING(g_pListPersonalViewHandle->
			       pCheckedItemsEditListForRemoved);
		MP_FREE_STRING(g_pListPersonalViewHandle->pFolderPath);

		free(g_pListPersonalViewHandle);
		g_pListPersonalViewHandle = NULL;
	}

	if (g_pListPersonalViewWidget) {

		MP_DEL_OBJ(g_pListPersonalViewWidget->pSelectallbtn);
		MP_DEL_OBJ(g_pListPersonalViewWidget->pSelAllLayout);
		MP_DEL_OBJ(g_pListPersonalViewWidget->pSavebtn);
		MP_DEL_OBJ(g_pListPersonalViewWidget->pCancelbtn);
		MP_DEL_OBJ(g_pListPersonalViewWidget->pVideoListBox);
		MP_DEL_OBJ(g_pListPersonalViewWidget->pPersonalViewBaselayout);
		MP_DEL_TIMER(g_pListPersonalViewWidget->pSelectedInfoLayoutTimer);

		free(g_pListPersonalViewWidget);
		g_pListPersonalViewWidget = NULL;
	}

}

void mp_personal_view_reset_value(void)
{
	VideoLogInfo("");

	if (!g_pListPersonalViewHandle) {
		VideoLogError("[ERR] g_pListPersonalViewHandle is not existed.");
		return;
	}

	g_pListPersonalViewHandle->pNaviFrameHandle = NULL;
	g_pListPersonalViewHandle->pNaviFrameItem = NULL;
	g_pListPersonalViewHandle->pChangeViewUserCbFunc = NULL;

	g_pListPersonalViewHandle->bSelectallCheckBoxState = FALSE;
	g_pListPersonalViewHandle->nSelectCounterForEditList = 0;
	g_pListPersonalViewHandle->nSortType =
		MP_MEDIA_SORT_BY_RECENTLY_VIEWED;

	if (g_pListPersonalViewHandle->pNaviFrameItem) {
		elm_object_item_part_content_unset(g_pListPersonalViewHandle->
						   pNaviFrameItem,
						   "toolbar_more_btn");
		elm_object_item_part_content_unset(g_pListPersonalViewHandle->
						   pNaviFrameItem, "toolbar");
		elm_object_item_part_content_unset(g_pListPersonalViewHandle->
						   pNaviFrameItem,
						   "toolbar_button2");
	}
}


/*//////////////////////////////////////////////////*/
/* Remove items. (Below functions are about the item deletion to call by button of toolbar.)*/
void mp_personal_view_done_delete_complete(void)
{
	if (!g_pListPersonalViewHandle) {
		VideoLogError("[ERR] g_pListPersonalViewHandle is NULL.");
		return;
	}

	VideoLogInfo("");

	if (g_pListPersonalViewHandle->pNaviFrameHandle) {
		elm_naviframe_item_pop(g_pListPersonalViewHandle->
				       pNaviFrameHandle);
	}

}

static void __mp_personal_view_run_cancel_items_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pListPersonalViewHandle->pNaviFrameHandle) {
		VideoLogError
		("g_pListPersonalViewHandle->pNaviFrameHandle is NULL");
		return;
	}

	VideoLogInfo("");

	elm_naviframe_item_pop(g_pListPersonalViewHandle->pNaviFrameHandle);
}

bool mp_personal_view_set_vconf_asked_state(bool bAsked)
{
	int nErr = 0;

	nErr =	preference_set_boolean(PREF_MP_UTIL_PERSONAL_NO_ASK_AGAIN, bAsked);
	if (nErr != 0) {
		VideoLogError
		("SET PREF_MP_UTIL_PERSONAL_NO_ASK_AGAIN is fail [0x%x]",
		 nErr);
		return FALSE;
	}

	return TRUE;
}

bool mp_personal_view_get_vconf_asked_state(bool *bAsked)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = 	preference_get_boolean(PREF_MP_UTIL_PERSONAL_NO_ASK_AGAIN, &nVal);
	if (nErr != 0) {
		VideoLogError
		("GET PREF_MP_UTIL_PERSONAL_NO_ASK_AGAIN is fail [0x%x]",
		 nErr);
		return FALSE;
	}

	*bAsked = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

static void __mp_personal_view_ask_remove_back_cb(void *data,
		Evas_Object *obj,
		void *event_info)
{
	mp_util_delete_popup_handle();
}

static void __mp_personal_view_ask_remove_ok_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pListPersonalViewHandle) {
		VideoLogError("[ERR] g_pListPersonalViewHandle is NULL.");
		return;
	}

	VideoLogInfo("");
	mp_util_delete_popup_handle();

	mp_personal_view_set_vconf_asked_state(g_pListPersonalViewHandle->
					       bCheckBoxState);

	int nPersonalListSize = 0;
	if (g_pListPersonalViewHandle->bFolderMove) {
		nPersonalListSize = mp_util_svc_get_video_folder_size();
	} else {
		nPersonalListSize =
			mp_util_svc_get_number_of_video_item_by_type();
	}

	mp_personal_ctrl_show(g_pListPersonalViewHandle->
			      pCheckedItemsEditListForRemoved,
			      nPersonalListSize,
			      g_pListPersonalViewHandle->nViewStyle,
			      g_pListPersonalViewHandle->bFolderMove,
			      mp_personal_view_done_delete_complete, NULL);
}

static void __mp_personal_view_check_box_changed_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!pObject) {
		VideoLogError("[ERR] pObject is NULL.");
		return;
	}
	if (!g_pListPersonalViewHandle) {
		VideoLogError("[ERR] g_pMainViewHandle is NULL.");
		return;
	}

	g_pListPersonalViewHandle->bCheckBoxState =
		elm_check_state_get(pObject);
}

static void __mp_personal_view_run_delete_items_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pListPersonalViewHandle) {
		VideoLogError("[ERR] g_pMainViewHandle is NULL.");
		return;
	}

	VideoLogInfo("");

	if (!g_pListPersonalViewHandle->pVideosGenlist) {
		VideoLogError("No exist genlist object.");
		return;
	}

	if (g_pListPersonalViewHandle->nSelectCounterForEditList > 0) {
		int nPersonalListSize = 0;
		if (g_pListPersonalViewHandle->bFolderMove) {
			nPersonalListSize = mp_util_svc_get_video_folder_size();
		} else {
			nPersonalListSize =
				mp_util_svc_get_number_of_video_item_by_type();
		}
		bool bAskedState = FALSE;
		mp_personal_view_get_vconf_asked_state(&bAskedState);
		VideoLogError("bAskedState %d", bAskedState);

		if (g_pListPersonalViewHandle->nViewStyle == 1 && (!bAskedState)) {
			char *szCheckLable = NULL;
			if (nPersonalListSize > 1) {
				szCheckLable =
					g_strdup_printf(MP_PERSONAL_ITEMS_MOVE_TO,
							VIDEO_DOWNLOAD_FILE_DIR);
			} else {
				szCheckLable =
					g_strdup_printf(MP_PERSONAL_ITEM_MOVE_TO,
							VIDEO_DOWNLOAD_FILE_DIR);
			}
			mp_util_text_and_check_popup
			(__mp_personal_view_ask_remove_back_cb,
			 __mp_personal_view_ask_remove_back_cb,
			 __mp_personal_view_ask_remove_ok_cb,
			 __mp_personal_view_check_box_changed_cb, szCheckLable,
			 NULL);
			MP_FREE_STRING(szCheckLable);
		} else {
			mp_personal_ctrl_show(g_pListPersonalViewHandle->
					      pCheckedItemsEditListForRemoved,
					      nPersonalListSize,
					      g_pListPersonalViewHandle->nViewStyle,
					      g_pListPersonalViewHandle->bFolderMove,
					      mp_personal_view_done_delete_complete,
					      NULL);
		}
	}
}

/**/
/*//////////////////////////////////////////////*/

/*//////////////////////////////////////////////*/
/* Back button*/

static void __mp_personal_view_trans_finished_cb(void *pUserData,
		Evas_Object *obj,
		void *event_info)
{
	if (!obj) {
		VideoLogError("[ERR] g_pListPersonalViewHandle is NULL.");
		return;
	}

	mp_widget_ctrl_enable_navi_handle_focus(obj);

	evas_object_smart_callback_del(obj, "transition,finished",
				       __mp_personal_view_trans_finished_cb);
}

static Eina_Bool mp_personal_view_back_btn_cb(void *pUserData,
		Elm_Object_Item *pItem)
{
	if (!g_pListPersonalViewHandle) {
		VideoLogInfo("[ERR] g_pListPersonalViewHandle is NULL.");
		return EINA_TRUE;
	}
	/* Register transition finished callback.*/
	mp_widget_ctrl_disable_navi_handle_focus_except_item
	(g_pListPersonalViewHandle->pNaviFrameHandle,
	 g_pListPersonalViewHandle->pNaviFrameItem);
	evas_object_smart_callback_add(g_pListPersonalViewHandle->
				       pNaviFrameHandle,
				       "transition,finished",
				       __mp_personal_view_trans_finished_cb,
				       NULL);

	mp_personal_view_pop();

	return EINA_TRUE;
}

void mp_personal_view_change_title_info(void)
{
	VideoLogInfo("nNumOfSelectdCount : %d",
		     g_pListPersonalViewHandle->nSelectCounterForEditList);

	if (g_pListPersonalViewHandle->nSelectCounterForEditList > 0) {
		elm_object_disabled_set(g_pListPersonalViewWidget->pSavebtn,
					EINA_FALSE);
	} else {
		VideoLogInfo("Unactivate delete tab.");
		elm_object_disabled_set(g_pListPersonalViewWidget->pSavebtn,
					EINA_TRUE);
	}

	char *szCheckingTotalItem =
		g_strdup_printf(VIDEOS_SELECTVIEW_HEADER_PD_SELECTED,
				g_pListPersonalViewHandle->
				nSelectCounterForEditList);
	elm_object_item_domain_translatable_part_text_set
	(g_pListPersonalViewHandle->pNaviFrameItem, "elm.text.title",
	 PACKAGE, szCheckingTotalItem);
	MP_FREE_STRING(szCheckingTotalItem);
}


static void __mp_personal_view_select_all_change_check_box_cb(void
		*pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!pObject) {
		VideoLogError("[ERR]");
		return;
	}

	if (!g_pListPersonalViewHandle->pVideosGenlist) {
		VideoLogError("No exist pVideoGenlist");
		return;
	}

	int nCounter = 0;
	int nPersonalListSize = 0;
	/*bool bState = elm_check_state_get(pObject);*/
	if (g_pListPersonalViewHandle->bFolderMove) {
		nPersonalListSize = mp_util_svc_get_video_folder_size();
	} else {
		nPersonalListSize =
			mp_util_svc_get_number_of_video_item_by_type();
	}
	VideoLogInfo("nPersonalListSize : %d", nPersonalListSize);

	if (!g_pListPersonalViewHandle->bSelectallCheckBoxState) {
		g_pListPersonalViewHandle->bSelectallCheckBoxState = TRUE;
		g_pListPersonalViewHandle->nSelectCounterForEditList =
			nPersonalListSize;
	} else {
		g_pListPersonalViewHandle->bSelectallCheckBoxState = FALSE;
		g_pListPersonalViewHandle->nSelectCounterForEditList = 0;

		VideoLogInfo("Unactivate delete tab.");
		elm_object_disabled_set(g_pListPersonalViewWidget->pSavebtn,
					EINA_TRUE);
	}
	mp_personal_view_change_title_info();

	for (nCounter = 0; nCounter < nPersonalListSize; nCounter++) {
		g_pListPersonalViewHandle->
		pCheckedItemsEditListForRemoved[nCounter] =
			g_pListPersonalViewHandle->bSelectallCheckBoxState;
	}

	mp_personal_view_update_all_check_box();

	/*elm_genlist_realized_items_update(g_pListPersonalViewHandle->pVideosGenlist);*/
}

void mp_personal_view_delete_select_all(void)
{
	VideoLogInfo("");

	g_pListPersonalViewHandle->bSelectallCheckBoxState = FALSE;
	g_pListPersonalViewHandle->nSelectCounterForEditList = 0;

	/*MpVideoPersonalViewHideSelectedInfo();*/
	mp_personal_view_change_title_info();
}

/*//////////////////////////////////////////////////*/

/*//////////////////////////////////////////////////*/
/* Check box callback*/
void mp_personal_view_change_check_box_state_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!pObject) {
		VideoLogInfo("[ERR]");
		return;
	} else {
		VideoLogInfo("pObject : %x", pObject);
	}

	int nVideoItemIndex = (int) pUserData;
	int nPersonalListSize = 0;
	if (g_pListPersonalViewHandle->bFolderMove) {
		nPersonalListSize = mp_util_svc_get_video_folder_size();
	} else {
		nPersonalListSize =
			mp_util_svc_get_number_of_video_item_by_type();
	}
	bool bState = elm_check_state_get(pObject);

	VideoLogInfo("Check box - Index : %d, List Size : %d",
		     nVideoItemIndex, nPersonalListSize);

	VideoLogInfo("BEFORE : nSelectCounterForEditList : %d",
		     g_pListPersonalViewHandle->nSelectCounterForEditList);

	if (bState) {
		g_pListPersonalViewHandle->
		pCheckedItemsEditListForRemoved[nVideoItemIndex] = TRUE;
		g_pListPersonalViewHandle->nSelectCounterForEditList++;
		if (nPersonalListSize ==
				g_pListPersonalViewHandle->nSelectCounterForEditList) {
			VideoLogInfo("All check box of edit list state are true.");
			g_pListPersonalViewHandle->bSelectallCheckBoxState = TRUE;
		}
	} else {
		g_pListPersonalViewHandle->
		pCheckedItemsEditListForRemoved[nVideoItemIndex] = FALSE;
		if (nPersonalListSize ==
				g_pListPersonalViewHandle->nSelectCounterForEditList) {
			VideoLogInfo
			("All check box of edit list state are not true.");
			g_pListPersonalViewHandle->bSelectallCheckBoxState = FALSE;
		}

		g_pListPersonalViewHandle->nSelectCounterForEditList--;
		if (g_pListPersonalViewHandle->nSelectCounterForEditList < 1) {
			VideoLogInfo("Unactivate delete button.");
			elm_object_disabled_set(g_pListPersonalViewWidget->pSavebtn,
						EINA_TRUE);
		}
	}
	elm_check_state_set(g_pListPersonalViewWidget->pSelectallbtn,
			    g_pListPersonalViewHandle->
			    bSelectallCheckBoxState);
	mp_personal_view_change_title_info();
	/*if (pSelectedItem)
	   {
	   elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	   } */

}

/**/
/*//////////////////////////////////////////////////*/

/*//////////////////////////////////////////////*/
/* Genlist callback*/
void mp_personal_view_naviframe_transition_effect_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (!g_pListPersonalViewHandle) {
		VideoLogError("[ERR] No exist g_pListPersonalViewHandle.");
		return;
	}

	VideoLogInfo("");

	Evas_Object *pTmpContents = NULL;
	Elm_Object_Item *pTmpItem = NULL;

	if (!g_pListPersonalViewHandle->pNaviFrameHandle) {
		VideoLogError("[ERR] No exist naviframe handle.");
		return;
	}

	pTmpItem =
		elm_naviframe_top_item_get(g_pListPersonalViewHandle->
					   pNaviFrameHandle);
	pTmpContents = elm_object_item_content_get(pTmpItem);
	if (pTmpContents) {
		if (pTmpContents == g_pListPersonalViewWidget->pVideoListBox) {
		} else {
			VideoLogInfo
			("It's not pPersonalViewTabbarLayout in top screen.");
		}
	} else {
		VideoLogError("[ERR] No exist naviframe top item.");
	}
}

void mp_personal_view_double_tap_cb(void)
{
	if (!g_pListPersonalViewHandle->pVideosGenlist) {
		VideoLogError("pVideosGenlist is not exist.");
		return;
	}

	VideoLogInfo("");

	elm_genlist_item_bring_in(elm_genlist_first_item_get
				  (g_pListPersonalViewHandle->pVideosGenlist),
				  ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

char *mp_personal_view_get_duration_time(int nVideoItemIndex)
{
	VideoLogInfo("");

	char szTmpStr[STR_LEN_MAX] = { 0, };
	char szTmpStrDurationTime[STR_LEN_MAX] = { 0, };
	char szTmpStrLastPlayedPos[STR_LEN_MAX] = { 0, };

	unsigned int nVideoInfoDuration =
		mp_util_svc_get_video_duration_time(nVideoItemIndex);
	unsigned int nVideoInfoLastTime =
		mp_util_svc_get_video_last_played_pos(nVideoItemIndex);

	mp_util_convert_time(nVideoInfoDuration, szTmpStrDurationTime, FALSE);
	mp_util_convert_time(nVideoInfoLastTime, szTmpStrLastPlayedPos, TRUE);

	snprintf(szTmpStr, STR_LEN_MAX, "%s/%s", szTmpStrLastPlayedPos,
		 szTmpStrDurationTime);

	return strdup(szTmpStr);
}

void mp_personal_view_select_video_item_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pListPersonalViewHandle->pVideosGenlist) {
		VideoLogInfo("No exist genlist object.");
		return;
	}

	VideoLogInfo("");

	int nVideoItemIndex = 0;

	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;
	Elm_Object_Item *pSelectedItem =
		elm_genlist_selected_item_get(pObject);

	if (pItem) {
		nVideoItemIndex = (int) elm_object_item_data_get(pItem);
		VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

		int nPersonalListSize = 0;
		if (g_pListPersonalViewHandle->bFolderMove) {
			nPersonalListSize = mp_util_svc_get_video_folder_size();
		} else {
			nPersonalListSize =
				mp_util_svc_get_number_of_video_item_by_type();
		}

		if (g_pListPersonalViewHandle->
				pCheckedItemsEditListForRemoved[nVideoItemIndex]) {
			g_pListPersonalViewHandle->
			pCheckedItemsEditListForRemoved[nVideoItemIndex] = FALSE;
			if (nPersonalListSize ==
					g_pListPersonalViewHandle->nSelectCounterForEditList) {
				VideoLogInfo
				("All check box of edit list state are not true.");
				g_pListPersonalViewHandle->bSelectallCheckBoxState =
					FALSE;
			}

			g_pListPersonalViewHandle->nSelectCounterForEditList--;
			if (g_pListPersonalViewHandle->nSelectCounterForEditList < 1) {
				VideoLogInfo("Unactivate delete button.");
				elm_object_disabled_set(g_pListPersonalViewWidget->
							pSavebtn, EINA_TRUE);
			}
		} else {
			g_pListPersonalViewHandle->
			pCheckedItemsEditListForRemoved[nVideoItemIndex] = TRUE;
			g_pListPersonalViewHandle->nSelectCounterForEditList++;
			if (nPersonalListSize ==
					g_pListPersonalViewHandle->nSelectCounterForEditList) {
				VideoLogInfo
				("All check box of edit list state are true.");
				g_pListPersonalViewHandle->bSelectallCheckBoxState = TRUE;
			}
		}
		elm_check_state_set(g_pListPersonalViewWidget->pSelectallbtn,
				    g_pListPersonalViewHandle->
				    bSelectallCheckBoxState);
		mp_personal_view_change_title_info();
		Evas_Object *check_box =
			elm_object_item_part_content_get(pItem, "elm.edit.icon.1");
		if (check_box) {
			elm_check_state_set(check_box,
					    g_pListPersonalViewHandle->
					    pCheckedItemsEditListForRemoved
					    [nVideoItemIndex]);
		}
		/*elm_genlist_item_fields_update(pSelectedItem, "elm.edit.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);*/
	} else {
		VideoLogError("[ERR]Error item number.");
	}

	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
}

char *mp_personal_view_get_label_of_video_item_cb(const void *pUserData,
		Evas_Object *pObject,
		const char *pPart)
{
	int nVideoItemIndex = (int) pUserData;

	if (!strcmp(pPart, "elm.text.1")) {
		char *szTitle;
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
	/*else if (!strcmp(pPart, "elm.text.3"))
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
	   } */

	return NULL;
}

char *mp_personal_view_get_label_of_folder_cb(const void *pUserData,
		Evas_Object *pObject,
		const char *pPart)
{
	int nFolderItemIndex = (int) pUserData;

	if (!strcmp(pPart, "elm.text.1")) {
		char *szTitle = NULL;
		szTitle = mp_util_svc_get_video_folder_name(nFolderItemIndex);

		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		MP_FREE_STRING(szTitle);
		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.2")) {
		char *szFolderPath = NULL;
		szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
		char *pDes_path = mp_util_get_dir_by_path(szFolderPath);
		MP_FREE_STRING(szFolderPath);
		char *szFolderUtf8 = elm_entry_utf8_to_markup(pDes_path);
		MP_FREE_STRING(pDes_path);
		return szFolderUtf8;
	} else {
	}

	return NULL;
}


Evas_Object *mp_personal_view_get_icon_of_video_item_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char *pPart)
{

	int nVideoItemIndex = (int) pUserData;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);

	if (!strcmp(pPart, "elm.thumbnail.icon")) {
		char *pThumbIconUri =
			mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg =
			mp_util_create_preload_image(pObject, pThumbIconUri,
						     VIDEO_ICON_WIDTH);
		if (pBg)
			evas_object_show(pBg);

		MP_FREE_STRING(pThumbIconUri);

		return pBg;
	} else if (!strcmp(pPart, "elm.progressbar.icon")) {
		int nLastPlayedTime =
			mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime =
			mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime <= 0) {
			return NULL;
		}
		
		double nRatio = 0.0;

		if (nDurationTime > 0) {
			nRatio = ((double)nLastPlayedTime) /
					((double)nDurationTime);
		}

		if (nRatio < 0.05) {
			nRatio = 0.05;
		}

		Evas_Object *pProgressBar = elm_progressbar_add(pObject);
		elm_object_style_set(pProgressBar, "video/list_progress");
		elm_progressbar_horizontal_set(pProgressBar, EINA_TRUE);
		evas_object_size_hint_align_set(pProgressBar, EVAS_HINT_FILL,
						EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(pProgressBar, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
		elm_progressbar_value_set(pProgressBar, nRatio);
		evas_object_show(pProgressBar);

		return pProgressBar;
	} else if (!strcmp(pPart, "elm.lock.icon")) {
		char *pVideoFile = mp_util_svc_get_video_url(nVideoItemIndex);
		bool bRet =
			mp_util_check_video_personal_status((const char *)
					pVideoFile);
		MP_FREE_STRING(pVideoFile);
		if (bRet) {
			Evas_Object *pLockIcon = elm_image_add(pObject);
			elm_image_file_set(pLockIcon, edj_path,
					   VIDEO_LIST_VIEW_ICON_PERSONAL_LOCK);
			evas_object_size_hint_align_set(pLockIcon, EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pLockIcon, EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_show(pLockIcon);
			return pLockIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.edit.icon.1")) {
		Evas_Object *pCheckBox = elm_check_add(pObject);
		evas_object_propagate_events_set(pCheckBox, EINA_FALSE);
		evas_object_smart_callback_add(pCheckBox, "changed",
					       mp_personal_view_change_check_box_state_cb,
					       pUserData);
		/*evas_object_smart_callback_add(pCheckBox, "changed", NULL, pUserData);*/
		elm_check_state_set(pCheckBox,
				    g_pListPersonalViewHandle->
				    pCheckedItemsEditListForRemoved
				    [nVideoItemIndex]);
		elm_check_state_pointer_set(pCheckBox,
					    (Eina_Bool *) &
					    (g_pListPersonalViewHandle->
					     pCheckedItemsEditListForRemoved
					     [nVideoItemIndex]));

		evas_object_show(pCheckBox);

		return pCheckBox;
	}
	return NULL;
}

Evas_Object *mp_personal_view_get_icon_of_folder_cb(const void *pUserData,
		Evas_Object *pObject,
		const char *pPart)
{
	int nFolderItemIndex = (int) pUserData;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);

	if (!strcmp(pPart, "elm.thumbnail.icon")) {
		Evas_Object *pBg = elm_bg_add(pObject);
		if (pBg) {
			elm_bg_load_size_set(pBg, VIDEO_ICON_WIDTH,
					     VIDEO_ICON_HEIGHT);
			evas_object_size_hint_max_set(pBg, VIDEO_ICON_WIDTH,
						      VIDEO_ICON_HEIGHT);
			evas_object_size_hint_weight_set(pBg, EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(pBg, EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			char *pFolderIconUri =
				mp_util_get_folder_icon_path(nFolderItemIndex);
			elm_bg_file_set(pBg, (const char *) pFolderIconUri, NULL);
			MP_FREE_STRING(pFolderIconUri);
		}
		return pBg;
	} else if (!strcmp(pPart, "elm.lock.icon")) {
		char *pFolderoFile =
			mp_util_svc_get_video_folder_url(nFolderItemIndex);
		bool bRet =
			mp_util_check_video_personal_status((const char *)
					pFolderoFile);
		MP_FREE_STRING(pFolderoFile);
		if (bRet) {
			Evas_Object *pLockIcon = elm_image_add(pObject);
			elm_image_file_set(pLockIcon, edj_path,
					   VIDEO_LIST_VIEW_ICON_PERSONAL_LOCK);
			evas_object_size_hint_align_set(pLockIcon, EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pLockIcon, EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_show(pLockIcon);
			return pLockIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.edit.icon.1")) {
		Evas_Object *pCheckBox = elm_check_add(pObject);
		evas_object_propagate_events_set(pCheckBox, EINA_FALSE);
		evas_object_smart_callback_add(pCheckBox, "changed",
					       mp_personal_view_change_check_box_state_cb,
					       pUserData);
		/*evas_object_smart_callback_add(pCheckBox, "changed", NULL, pUserData);*/
		elm_check_state_set(pCheckBox,
				    g_pListPersonalViewHandle->
				    pCheckedItemsEditListForRemoved
				    [nFolderItemIndex]);
		elm_check_state_pointer_set(pCheckBox,
					    (Eina_Bool *) &
					    (g_pListPersonalViewHandle->
					     pCheckedItemsEditListForRemoved
					     [nFolderItemIndex]));

		evas_object_show(pCheckBox);

		return pCheckBox;
	}
	return NULL;
}

/**/
/*//////////////////////////////////////////////*/

int mp_personal_view_get_sort_type(void)
{
	switch (mp_sort_ctrl_get_sort_state()) {
	/*case MP_LIST_SORT_BY_RECENTLY_VIEWED:
	   g_pListPersonalViewHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
	   break; */

	case MP_LIST_SORT_BY_RECENTLY_ADDED:
		g_pListPersonalViewHandle->nSortType =
			MP_MEDIA_SORT_BY_RECENTLY_ADDED;
		break;

	case MP_LIST_SORT_BY_NAME:
		g_pListPersonalViewHandle->nSortType = MP_MEDIA_SORT_BY_NAME;
		break;

		/*case MP_LIST_SORT_BY_SIZE:
		   g_pListPersonalViewHandle->nSortType = MP_MEDIA_SORT_BY_SIZE;
		   break;

		   case MP_LIST_SORT_BY_TYPE:
		   g_pListPersonalViewHandle->nSortType = MP_MEDIA_SORT_BY_TYPE;
		   break; */
	}

	return g_pListPersonalViewHandle->nSortType;
}

void mp_personal_view_append_video_items(void *pVideosGenlist)
{
	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	VideoPersonal_Itc.item_style = "video/edit_list";
	VideoPersonal_Itc.func.text_get =
		(void *) mp_personal_view_get_label_of_video_item_cb;
	VideoPersonal_Itc.func.content_get =
		(void *) mp_personal_view_get_icon_of_video_item_cb;
	VideoPersonal_Itc.func.state_get = NULL;
	VideoPersonal_Itc.func.del = NULL;
	/*VideoPersonal_Itc.decorate_item_style = "mode/slide4";*/
	/*VideoPersonal_Itc.decorate_all_item_style = "edit_default";*/

	for (nIndex = 0; nIndex < nVideoListSize; nIndex++) {
		elm_genlist_item_append(pVideosGenlist, &VideoPersonal_Itc,
					(void *) nIndex, NULL,
					ELM_GENLIST_ITEM_NONE,
					mp_personal_view_select_video_item_cb,
					NULL);
	}
}

void mp_personal_view_append_folder_items(void *pVideosGenlist)
{
	int nIndex = 0;
	int nFolderListSize = mp_util_svc_get_video_folder_size();

	FolderPersonal_Itc.item_style = "video/edit_folder";
	FolderPersonal_Itc.func.text_get =
		(void *) mp_personal_view_get_label_of_folder_cb;
	FolderPersonal_Itc.func.content_get =
		(void *) mp_personal_view_get_icon_of_folder_cb;
	FolderPersonal_Itc.func.state_get = NULL;
	FolderPersonal_Itc.func.del = NULL;

	for (nIndex = 0; nIndex < nFolderListSize; nIndex++) {
		elm_genlist_item_append(pVideosGenlist, &FolderPersonal_Itc,
					(void *) nIndex, NULL,
					ELM_GENLIST_ITEM_NONE,
					mp_personal_view_select_video_item_cb,
					NULL);
	}
}


void mp_personal_view_init_set_value(int nTotalSize)
{
	VideoLogInfo("");

	if (g_pListPersonalViewHandle->pCheckedItemsEditListForRemoved) {
		free(g_pListPersonalViewHandle->pCheckedItemsEditListForRemoved);
		g_pListPersonalViewHandle->pCheckedItemsEditListForRemoved = NULL;
	}
	if (nTotalSize > 0) {
		g_pListPersonalViewHandle->pCheckedItemsEditListForRemoved =
			(bool *) malloc(sizeof(bool)*nTotalSize);
		memset(g_pListPersonalViewHandle->pCheckedItemsEditListForRemoved,
		       0, sizeof(bool)*nTotalSize);
	}

	g_pListPersonalViewHandle->bSelectallCheckBoxState = FALSE;
}

static void __mp_personal_view_get_folder_data(void)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}
	/*      int nViewType = mp_view_as_ctrl_get_type();*/
	int nSortType = mp_personal_view_get_sort_type();

	mp_util_svc_destory_video_folder_list();

	if (g_pListPersonalViewHandle->nViewStyle == 0) {
		mp_util_svc_extract_personal_video_folder_list(nSortType, 0);
	} else {
		mp_util_svc_extract_personal_video_folder_list(nSortType, 1);
	}
}


static void __mp_personal_view_get_data(void)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nSortType = mp_personal_view_get_sort_type();
	int nViewType = mp_view_as_ctrl_get_type();
	if ((nViewType == MP_LIST_VIEW_AS_FOLDER_LIST)
			&& (g_pListPersonalViewHandle->pFolderPath)) {
		VideoSecureLogInfo("szFolderPath : %s",
				   g_pListPersonalViewHandle->pFolderPath);
		if (g_pListPersonalViewHandle->pFolderPath) {
			if (vp_file_exists(g_pListPersonalViewHandle->pFolderPath)) {
				/*update video-list*/

				mp_util_svc_destory_video_item_list();
				if (g_pListPersonalViewHandle->nViewStyle == 0) {
					mp_util_svc_extract_video_list_from_folder_by_personal
					(g_pListPersonalViewHandle->pFolderPath,
					 nSortType, MP_LIST_VIEW_ALL, 0);
				} else {
					mp_util_svc_extract_video_list_from_folder_by_personal
					(g_pListPersonalViewHandle->pFolderPath,
					 nSortType, MP_LIST_VIEW_ALL, 1);
				}
			} else {
				VideoLogError("invalid path");
			}
		}
	} else {
		mp_util_svc_destory_video_list();
		if (g_pListPersonalViewHandle->nViewStyle == 0) {
			mp_util_svc_extract_video_list_by_personal(nSortType,
					mp_view_ctrl_get_list_type
					(), 0);
		} else {
			mp_util_svc_extract_video_list_by_personal(nSortType,
					mp_view_ctrl_get_list_type
					(), 1);
		}
	}

}

Evas_Object *mp_personal_view_get_icon_of_no_folder_item_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char
		*pPart)
{
	VideoLogInfo("");

	if (!strcmp(pPart, "elm.icon")) {
		int width = 720 * elm_config_scale_get();
		int height = 1100 * elm_config_scale_get();
		Evas_Object *pNocontentsLayout = elm_layout_add(pObject);
		elm_layout_theme_set(pNocontentsLayout, "layout", "nocontents",
				     "video");
		evas_object_size_hint_min_set(pNocontentsLayout, width, height);
		elm_object_part_text_set(pNocontentsLayout, "elm.text",
					 "No Folders");
		evas_object_show(pNocontentsLayout);

		return pNocontentsLayout;
	}

	return NULL;
}

void mp_personal_view_append_no_personal_folder_items(void
		*pVideosGenlist)
{
	VideoLogInfo("");

	FolderPersonalNoItem_Itc.item_style = "video/1icon";
	FolderPersonalNoItem_Itc.func.text_get = NULL;
	FolderPersonalNoItem_Itc.func.content_get =
		(void *) mp_personal_view_get_icon_of_no_folder_item_cb;
	FolderPersonalNoItem_Itc.func.state_get = NULL;
	FolderPersonalNoItem_Itc.func.del = NULL;
	FolderPersonalNoItem_Itc.decorate_item_style = NULL;

	elm_genlist_item_append(pVideosGenlist, &FolderPersonalNoItem_Itc,
				NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL,
				NULL);
}

void mp_personal_view_arrange_folder_list(int nItemSortType,
		void *pVideosGenlist,
		bool bNeedSvc)
{
	if (!g_pListPersonalViewWidget || !g_pListPersonalViewHandle) {
		VideoLogError("No exist g_pRemoveListViewWidget.");
		return;
	}

	if (!pVideosGenlist) {
		VideoLogError("No exist genlist object.");
		return;
	}

	int nFolderListSize = 0;

	__mp_personal_view_get_folder_data();

	nFolderListSize = mp_util_svc_get_video_folder_size();
	VideoLogInfo("nFolderListSize : %d", nFolderListSize);
	if (bNeedSvc) {
		__mp_personal_view_update_selected_video_list();
	} else {
		mp_personal_view_init_set_value(nFolderListSize);
	}

	elm_genlist_clear(pVideosGenlist);

	if (nFolderListSize > 0) {
		mp_personal_view_append_folder_items(pVideosGenlist);
	} else {
		mp_personal_view_append_no_personal_folder_items(pVideosGenlist);
	}

	elm_genlist_decorate_mode_set(pVideosGenlist, EINA_TRUE);

	evas_object_show(pVideosGenlist);
}


void mp_personal_view_arrange_video_list(int nItemSortType,
		void *pVideosGenlist,
		bool bNeedSvc)
{
	if (!g_pListPersonalViewWidget || !g_pListPersonalViewHandle) {
		VideoLogError("No exist g_pRemoveListViewWidget.");
		return;
	}

	if (!pVideosGenlist) {
		VideoLogError("No exist genlist object.");
		return;
	}

	int nVideoListSize = 0;

	__mp_personal_view_get_data();

	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("nVideoListSize : %d", nVideoListSize);
	if (bNeedSvc) {
		__mp_personal_view_update_selected_video_list();
	} else {
		mp_personal_view_init_set_value(nVideoListSize);
	}

	elm_genlist_clear(pVideosGenlist);

	if (nVideoListSize > 0) {
		mp_personal_view_append_video_items(pVideosGenlist);
	}

	elm_genlist_decorate_mode_set(pVideosGenlist, EINA_TRUE);

	evas_object_show(pVideosGenlist);
}

Evas_Object *mp_personal_view_init_internal_layout(void *pParent)
{
	if (!pParent) {
		VideoLogError("[ERR]");
		return NULL;
	}

	VideoLogInfo("");

	if (g_pListPersonalViewWidget->pPersonalViewBaselayout) {
		evas_object_del(g_pListPersonalViewWidget->
				pPersonalViewBaselayout);
		g_pListPersonalViewWidget->pPersonalViewBaselayout = NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_PLAYER_REMOVE_LIST_EDJ);

	g_pListPersonalViewWidget->pPersonalViewBaselayout =
		elm_layout_add(pParent);
	elm_layout_file_set(g_pListPersonalViewWidget->
			    pPersonalViewBaselayout,
			    edj_path,
			    REMOVE_LIST_EDJ_GROUP);
	evas_object_size_hint_weight_set(g_pListPersonalViewWidget->
					 pPersonalViewBaselayout,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pListPersonalViewWidget->
					pPersonalViewBaselayout,
					EVAS_HINT_FILL, EVAS_HINT_FILL);

	return g_pListPersonalViewWidget->pPersonalViewBaselayout;
}

static bool __mp_personal_view_get_selected_video_list(int nSelectedSize)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return FALSE;
	}

	int nVideoListSize = 0;

	if (!g_pListPersonalViewHandle->nSelectCounterForEditList
			|| nSelectedSize < 1) {
		return FALSE;
	}

	if (g_pListPersonalViewHandle->bFolderMove) {
		nVideoListSize = mp_util_svc_get_video_folder_size();
	} else {
		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	}

	__mp_personal_view_free_selected_list();

	int nIndex = 0;
	int nCounter = 0;
	char *pMediaId = NULL;

	g_pListPersonalViewHandle->pSelectedList =
		(char **) calloc(sizeof(char *), nSelectedSize);
	VideoLogInfo("current edit size===%d", nSelectedSize);

	for (nCounter = 0; nCounter < nVideoListSize; nCounter++) {
		if (g_pListPersonalViewHandle->
				pCheckedItemsEditListForRemoved[nCounter]) {
			if (g_pListPersonalViewHandle->bFolderMove) {
				pMediaId = mp_util_svc_get_video_folder_id(nCounter);
			} else {
				pMediaId = mp_util_svc_get_video_id_by_index(nCounter);
			}
			if (pMediaId != NULL) {
				g_pListPersonalViewHandle->pSelectedList[nIndex] =
					strdup(pMediaId);
				MP_FREE_STRING(pMediaId);
				nIndex++;
			}
		}
	}
	return TRUE;
}

static void __mp_personal_view_update_selected_video_list(void)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nVideoListSize = 0;

	MpMediaType eMediaType = MEDIA_TYPE_FILE;

	if (g_pListPersonalViewHandle->bFolderMove) {
		eMediaType = MEDIA_TYPE_DIRECTORY;
		nVideoListSize = mp_util_svc_get_video_folder_size();
	} else {
		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	}

	int nIndex = 0;
	int nCounter = 0;
	bool bValidMedia = FALSE;
	int nTotal = g_pListPersonalViewHandle->nSelectCounterForEditList;
	VideoLogInfo("OldEditList ===%d", nTotal);
	g_pListPersonalViewHandle->nSelectCounterForEditList = 0;
	mp_personal_view_init_set_value(nVideoListSize);

	if (!g_pListPersonalViewHandle->pSelectedList) {
		return;
	}

	while ((nIndex < nTotal)
			&& g_pListPersonalViewHandle->pSelectedList[nIndex]) {

		bValidMedia =
			mp_util_svc_check_valid_media_id(g_pListPersonalViewHandle->
					pSelectedList[nIndex],
					eMediaType, &nCounter);
		if (bValidMedia) {
			g_pListPersonalViewHandle->
			pCheckedItemsEditListForRemoved[nCounter] = TRUE;
			g_pListPersonalViewHandle->nSelectCounterForEditList++;
		}
		nIndex++;
	}

	mp_personal_view_change_title_info();

	if (g_pListPersonalViewHandle->nSelectCounterForEditList > 0)
		elm_object_disabled_set(g_pListPersonalViewWidget->pSavebtn,
					EINA_FALSE);
	else
		elm_object_disabled_set(g_pListPersonalViewWidget->pSavebtn,
					EINA_TRUE);

}

static void __mp_personal_view_db_backup_cb(void *pUserData)
{
	if (!g_pListPersonalViewHandle) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}
	__mp_personal_view_get_selected_video_list(g_pListPersonalViewHandle->
			nSelectCounterForEditList);

}

static void __mp_personal_view_db_changed_cb(void *pUserData)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}
	int nSortType = mp_personal_view_get_sort_type();
	int nViewType = mp_view_as_ctrl_get_type();
	if ((nViewType == MP_LIST_VIEW_AS_FOLDER_LIST)
			&& g_pListPersonalViewHandle->pFolderPath) {
		if (g_pListPersonalViewHandle->pFolderPath) {
			VideoSecureLogInfo("szFolderPath : %s",
					   g_pListPersonalViewHandle->pFolderPath);
			if (vp_file_exists(g_pListPersonalViewHandle->pFolderPath)) {
				/*update video-list*/

				mp_util_svc_destory_video_item_list();
				if (g_pListPersonalViewHandle->nViewStyle == 0) {
					mp_util_svc_extract_video_list_from_folder_by_personal
					(g_pListPersonalViewHandle->pFolderPath,
					 nSortType, MP_LIST_VIEW_ALL, 0);
				} else {
					mp_util_svc_extract_video_list_from_folder_by_personal
					(g_pListPersonalViewHandle->pFolderPath,
					 nSortType, MP_LIST_VIEW_ALL, 1);
				}
				mp_personal_view_arrange_video_list(nSortType,
								    g_pListPersonalViewHandle->
								    pVideosGenlist, TRUE);
			} else {
				elm_naviframe_item_pop(g_pListPersonalViewHandle->
						       pNaviFrameHandle);
			}
		}
	} else if (g_pListPersonalViewHandle->bFolderMove) {
		mp_util_svc_destory_video_folder_list();

		if (g_pListPersonalViewHandle->nViewStyle == 0) {
			mp_util_svc_extract_personal_video_folder_list(nSortType, 0);
		} else {
			mp_util_svc_extract_personal_video_folder_list(nSortType, 1);
		}

		int nFolderListSize = mp_util_svc_get_video_folder_size();

		if (nFolderListSize > 0) {
			mp_personal_view_arrange_folder_list(nSortType,
							     g_pListPersonalViewHandle->
							     pVideosGenlist, TRUE);
		} else {
			elm_naviframe_item_pop(g_pListPersonalViewHandle->
					       pNaviFrameHandle);
		}
	} else {
		mp_util_svc_destory_video_list();
		if (g_pListPersonalViewHandle->nViewStyle == 0) {
			mp_util_svc_extract_video_list_by_personal(nSortType,
					mp_view_ctrl_get_list_type
					(), 0);
		} else {
			mp_util_svc_extract_video_list_by_personal(nSortType,
					mp_view_ctrl_get_list_type
					(), 1);
		}

		int nVideoListSize =
			mp_util_svc_get_number_of_video_item_by_type();
		VideoLogInfo("nVideoListSize : %d", nVideoListSize);
		if (nVideoListSize > 0) {
			mp_personal_view_arrange_video_list(nSortType,
							    g_pListPersonalViewHandle->
							    pVideosGenlist, TRUE);
		} else {
			elm_naviframe_item_pop(g_pListPersonalViewHandle->
					       pNaviFrameHandle);
		}
	}
	Elm_Object_Item *pItem =
		elm_genlist_first_item_get(g_pListPersonalViewHandle->
					   pVideosGenlist);
	if (pItem) {
		elm_genlist_item_bring_in(pItem, ELM_GENLIST_ITEM_SCROLLTO_IN);
	}

}

static void __mp_personal_view_personal_vconf_cb(keynode_t *pKeyNode,
		void *pUserData)
{
	if (g_pListPersonalViewHandle) {
		elm_naviframe_item_pop(g_pListPersonalViewHandle->
				       pNaviFrameHandle);
	}
}

static void __mp_personal_view_register_personal_vconf()
{
#ifndef	ENABLE_PERSONAL
	return;
#endif
	if (vconf_notify_key_changed
			(MP_UTIL_VCONF_KEY_PERSONAL_TAG,
			 __mp_personal_view_personal_vconf_cb, NULL) < 0) {
		VideoLogInfo
		("Fail to register MP_UTIL_VCONF_KEY_PERSONAL_TAG key callback");
	}
}

static void __mp_personal_view_ignore_personal_vconf()
{
#ifndef	ENABLE_PERSONAL
	return;
#endif
	if (vconf_ignore_key_changed
			(MP_UTIL_VCONF_KEY_PERSONAL_TAG,
			 __mp_personal_view_personal_vconf_cb) < 0) {
		VideoLogInfo("Fail to vconf_ignore_key_changed callback");
	}
}

static void __mp_personal_select_all_layout_down_cb(void *data,
		Evas *evas,
		Evas_Object *obj,
		void *event_info)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	Evas_Object *check =
		elm_object_part_content_get(g_pListPersonalViewWidget->
					    pSelAllLayout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	__mp_personal_view_select_all_change_check_box_cb(data, check, NULL);
}

void mp_personal_view_init(void *pParent)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");
	g_pListPersonalViewWidget->pPersonalViewBaselayout =
		mp_personal_view_init_internal_layout(pParent);
	if (!g_pListPersonalViewWidget->pPersonalViewBaselayout) {
		VideoLogError("pPersonalViewBaselayout is not existed.");
		return;
	}
	evas_object_event_callback_add(g_pListPersonalViewWidget->
				       pPersonalViewBaselayout,
				       EVAS_CALLBACK_DEL,
				       __mp_personal_view_base_layout_del_cb,
				       NULL);

	g_pListPersonalViewWidget->pVideoListBox =
		elm_box_add(g_pListPersonalViewWidget->pPersonalViewBaselayout);
	if (!g_pListPersonalViewWidget->pVideoListBox) {
		VideoLogError("pVideoListBox is not existed.");
		return;
	}

	evas_object_size_hint_weight_set(g_pListPersonalViewWidget->
					 pVideoListBox, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pListPersonalViewWidget->
					pVideoListBox, EVAS_HINT_FILL,
					EVAS_HINT_FILL);

	mp_widget_ctrl_create_select_all_layout(g_pListPersonalViewWidget->
						pVideoListBox,
						__mp_personal_view_select_all_change_check_box_cb,
						__mp_personal_select_all_layout_down_cb,
						(void *)
						g_pListPersonalViewWidget,
						&g_pListPersonalViewWidget->
						pSelectallbtn,
						&g_pListPersonalViewWidget->
						pSelAllLayout);

	if (!g_pListPersonalViewWidget->pSelAllLayout) {
		VideoLogError("pSelAllLayout is not existed.");
		return;
	}

	elm_box_pack_start(g_pListPersonalViewWidget->pVideoListBox,
			   g_pListPersonalViewWidget->pSelAllLayout);

	g_pListPersonalViewHandle->pVideosGenlist =
		elm_genlist_add(g_pListPersonalViewWidget->pVideoListBox);
	if (!g_pListPersonalViewHandle->pVideosGenlist) {
		VideoLogError("pVideosGenlist is not existed.");
		return;
	}

	elm_genlist_homogeneous_set(g_pListPersonalViewHandle->pVideosGenlist,
				    EINA_TRUE);
	evas_object_size_hint_weight_set(g_pListPersonalViewHandle->
					 pVideosGenlist, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pListPersonalViewHandle->
					pVideosGenlist, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	elm_genlist_block_count_set(g_pListPersonalViewHandle->pVideosGenlist,
				    VIDEO_GENLIST_BLOCK_COUNT);

	elm_genlist_mode_set(g_pListPersonalViewHandle->pVideosGenlist,
			     ELM_LIST_COMPRESS);
	elm_box_pack_end(g_pListPersonalViewWidget->pVideoListBox,
			 g_pListPersonalViewHandle->pVideosGenlist);
	elm_object_part_content_set(g_pListPersonalViewWidget->
				    pPersonalViewBaselayout,
				    SWALLOW_LISTVIEW_CONTENT,
				    g_pListPersonalViewWidget->pVideoListBox);

	/*MpVideoPersonalViewCreateSelectionInfo(pParent);*/

	/*register db callback*/
	/*mp_util_svc_set_update_db_cb_func(__mp_personal_view_db_changed_cb);*/

	if (g_pListPersonalViewHandle->bFolderMove) {
		VideoLogInfo("ArrangePersonalFolderList");
		mp_personal_view_arrange_folder_list
		(mp_personal_view_get_sort_type(),
		 g_pListPersonalViewHandle->pVideosGenlist, FALSE);
	} else {
		mp_personal_view_arrange_video_list(mp_personal_view_get_sort_type
						    (),
						    g_pListPersonalViewHandle->
						    pVideosGenlist, FALSE);
	}

	char *szTitle =
		g_strdup_printf(VIDEOS_SELECTVIEW_HEADER_PD_SELECTED,
				g_pListPersonalViewHandle->
				nSelectCounterForEditList);

	g_pListPersonalViewHandle->pNaviFrameItem =
		elm_naviframe_item_push(g_pListPersonalViewHandle->
					pNaviFrameHandle, szTitle, NULL, NULL,
					g_pListPersonalViewWidget->
					pPersonalViewBaselayout, NULL);
	elm_object_item_domain_text_translatable_set
	(g_pListPersonalViewHandle->pNaviFrameItem, VIDEOS_STRING,
	 EINA_TRUE);
	elm_naviframe_item_pop_cb_set(g_pListPersonalViewHandle->
				      pNaviFrameItem,
				      mp_personal_view_back_btn_cb,
				      g_pListPersonalViewHandle);

	elm_naviframe_item_title_enabled_set(g_pListPersonalViewHandle->
					     pNaviFrameItem, EINA_TRUE,
					     EINA_FALSE);
	evas_object_smart_callback_add(g_pListPersonalViewHandle->
				       pNaviFrameHandle,
				       "transition,finished",
				       mp_personal_view_naviframe_transition_effect_cb,
				       NULL);

	g_pListPersonalViewWidget->pCancelbtn =
		mp_widget_ctrl_create_navi_left_btn(g_pListPersonalViewHandle->
				pNaviFrameHandle,
				g_pListPersonalViewHandle->
				pNaviFrameItem, NULL,
				__mp_personal_view_run_cancel_items_cb,
				(void *)
				g_pListPersonalViewHandle);
	g_pListPersonalViewWidget->pSavebtn =
		mp_widget_ctrl_create_navi_right_btn(g_pListPersonalViewHandle->
				pNaviFrameHandle,
				g_pListPersonalViewHandle->
				pNaviFrameItem, NULL,
				__mp_personal_view_run_delete_items_cb,
				(void *)
				g_pListPersonalViewHandle);
	elm_object_disabled_set(g_pListPersonalViewWidget->pSavebtn,
				EINA_TRUE);


	evas_object_show(g_pListPersonalViewHandle->pVideosGenlist);
	evas_object_show(g_pListPersonalViewWidget->pPersonalViewBaselayout);
	__mp_personal_view_register_personal_vconf();
	mp_util_db_set_update_fun(g_pListPersonalViewHandle->euLev,
				  __mp_personal_view_db_changed_cb);
	mp_util_db_set_backup_fun(g_pListPersonalViewHandle->euLev,
				  __mp_personal_view_db_backup_cb);

	MP_FREE_STRING(szTitle);
	mp_util_hide_indicator();

}

void mp_personal_view_push(void *pNaviFrame,
			   ChangePersonalViewCbFunc pChangeViewCb,
			   char *pFolderPath, int nViewStyle)
{
	VideoLogInfo("");
	mp_personal_view_destroy();
	g_pListPersonalViewHandle =
		(st_VideoListPersonalViewHandle *) calloc(1,
				sizeof
				(st_VideoListPersonalViewHandle));
	g_pListPersonalViewWidget =
		(st_VideoListPersonalViewWidget *) calloc(1,
				sizeof
				(st_VideoListPersonalViewWidget));

	memset(g_pListPersonalViewHandle, 0,
	       sizeof(st_VideoListPersonalViewHandle));
	memset(g_pListPersonalViewWidget, 0,
	       sizeof(st_VideoListPersonalViewWidget));
	mp_personal_view_reset_value();

	g_pListPersonalViewHandle->pNaviFrameHandle = pNaviFrame;
	g_pListPersonalViewHandle->pChangeViewUserCbFunc = pChangeViewCb;
	if (pFolderPath) {
		g_pListPersonalViewHandle->pFolderPath = strdup(pFolderPath);
	}
	g_pListPersonalViewHandle->nViewStyle = nViewStyle;
	g_pListPersonalViewHandle->euLev = MP_DB_UPDATE_LEV_1;

	int nViewType = mp_view_as_ctrl_get_type();
	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST
			&& g_pListPersonalViewHandle->pFolderPath) {
		g_pListPersonalViewHandle->euLev = MP_DB_UPDATE_LEV_2;
	}

	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST
			&& !g_pListPersonalViewHandle->pFolderPath) {
		g_pListPersonalViewHandle->bFolderMove = TRUE;
	} else {
		g_pListPersonalViewHandle->bFolderMove = FALSE;
	}

	mp_personal_view_init(g_pListPersonalViewHandle->pNaviFrameHandle);
}

void mp_personal_view_update(void)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nSortType = mp_personal_view_get_sort_type();
	int nViewType = mp_view_as_ctrl_get_type();
	if ((nViewType == MP_LIST_VIEW_AS_FOLDER_LIST)
			&& g_pListPersonalViewHandle->pFolderPath) {
		if (g_pListPersonalViewHandle->pFolderPath) {
			VideoSecureLogInfo("szFolderPath : %s",
					   g_pListPersonalViewHandle->pFolderPath);
			if (vp_file_exists(g_pListPersonalViewHandle->pFolderPath)) {
				/*update video-list*/
				mp_util_svc_destory_video_item_list();
				if (g_pListPersonalViewHandle->nViewStyle == 0) {
					mp_util_svc_extract_video_list_from_folder_by_personal
					(g_pListPersonalViewHandle->pFolderPath,
					 nSortType, MP_LIST_VIEW_ALL, 0);
				} else {
					mp_util_svc_extract_video_list_from_folder_by_personal
					(g_pListPersonalViewHandle->pFolderPath,
					 nSortType, MP_LIST_VIEW_ALL, 1);
				}

				elm_genlist_clear(g_pListPersonalViewHandle->
						  pVideosGenlist);

				mp_personal_view_append_video_items
				(g_pListPersonalViewHandle->pVideosGenlist);
			} else {
				elm_naviframe_item_pop(g_pListPersonalViewHandle->
						       pNaviFrameHandle);
			}
		}
	} else if (g_pListPersonalViewHandle->bFolderMove) {
		mp_util_svc_destory_video_folder_list();

		if (g_pListPersonalViewHandle->nViewStyle == 0) {
			mp_util_svc_extract_personal_video_folder_list(nSortType, 0);
		} else {
			mp_util_svc_extract_personal_video_folder_list(nSortType, 1);
		}

		elm_genlist_clear(g_pListPersonalViewHandle->pVideosGenlist);

		int nFolderListSize = mp_util_svc_get_video_folder_size();

		if (nFolderListSize > 0) {
			mp_personal_view_append_folder_items
			(g_pListPersonalViewHandle->pVideosGenlist);
		} else {
			elm_naviframe_item_pop(g_pListPersonalViewHandle->
					       pNaviFrameHandle);
		}
	} else {
		mp_util_svc_destory_video_list();
		if (g_pListPersonalViewHandle->nViewStyle == 0) {
			mp_util_svc_extract_video_list_by_personal(nSortType,
					mp_view_ctrl_get_list_type
					(), 0);
		} else {
			mp_util_svc_extract_video_list_by_personal(nSortType,
					mp_view_ctrl_get_list_type
					(), 1);
		}

		int nVideoListSize =
			mp_util_svc_get_number_of_video_item_by_type();

		elm_genlist_clear(g_pListPersonalViewHandle->pVideosGenlist);

		if (nVideoListSize > 0) {
			mp_personal_view_append_video_items
			(g_pListPersonalViewHandle->pVideosGenlist);
		} else {
			elm_naviframe_item_pop(g_pListPersonalViewHandle->
					       pNaviFrameHandle);
		}
	}

}


void mp_personal_view_pop(void)
{
	VideoLogInfo("");
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	mp_util_db_reset_backup_fun_up_self(g_pListPersonalViewHandle->euLev);

	if (g_pListPersonalViewHandle->pChangeViewUserCbFunc) {
		g_pListPersonalViewHandle->
		pChangeViewUserCbFunc(LIST_UPDATE_TYPE_ALL);
	}
}

bool mp_personal_view_is_top_view(void)
{
	if (!g_pListPersonalViewHandle || !g_pListPersonalViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return FALSE;
	}

	return TRUE;
}


static void
__mp_personal_view_base_layout_del_cb(void *pUserData, Evas *e,
				      Evas_Object *pObject,
				      void *pEventInfo)
{
	/*evas_object_event_callback_del(g_pListPersonalViewWidget->pPersonalViewBaselayout, EVAS_CALLBACK_DEL, __mp_personal_view_base_layout_del_cb);*/

	__mp_personal_view_reset();
}
