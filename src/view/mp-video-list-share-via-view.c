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
#include "vp-file-util.h"
#include "mp-video-log.h"
#include "mp-external-ug.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-util-media-service.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-list-share-via-view.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-util-db-controller.h"
#include "mp-rotate-ctrl.h"
#include "videos-view-mgr.h"

#define VIDEO_SELECTED_INFO_TIMER		4.0
#define FIRST_SHARE_ITEM_OFFSET			1
#define VIDEO_SHARE_ITEM_MAX			30

typedef struct {
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	/* used for list view ,thumbnail view */
	void *pObjVideosList;

	Elm_Genlist_Item_Class *pGenListItc;
	Elm_Genlist_Item_Class *pGenListLandscapeItc;
	Elm_Gengrid_Item_Class *pGenGridItc_1;
	Elm_Gengrid_Item_Class *pGenGridItc_2;

	bool *pCheckedItemsEditList;
	/* save selected video id */
	char **pSelectedList;
	bool bSelectallCheckBoxState;
	int nSelectCounterForEditList;
	MpMediaSvcSortType nSortType;
	MpDbUpdateViewLev euLev;
	ChangeListShareViewCbFunc pChangeViewUserCbFunc;
	char *pFolderPath;

	MpVideoListTabType eTabType;
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	bool bLandscape;
	bool bThumbnailView;
#endif
} st_VideoListShareViaViewHandle;

typedef struct {
	/* Evas_Object *pSelectionInfoLayout; */
	Evas_Object *pVideoListBox;
	Evas_Object *pChareViaViewBaselayout;
	Evas_Object *pSelectallbtn;
	Evas_Object *pSelAllLayout;
	Evas_Object *pSavebtn;
	Evas_Object *pCancelbtn;

} st_VideoListShareViaViewWidget;

static st_VideoListShareViaViewHandle *g_pShareViaHandle = NULL;
static st_VideoListShareViaViewWidget *g_pShareViaWidget = NULL;

int mp_share_view_get_sort_type(void);
bool __mp_share_view_arrange_video_list(void *pObjVideosList, bool bNeedSvc);
void mp_share_view_naviframe_transition_effect_cb(void *pUserData,
	Evas_Object *pObject, void *pEventInfo);
void mp_share_view_change_title_info(void);
static void __mp_share_view_base_layout_del_cb(void *pUserData, Evas *e,
	Evas_Object *pObject, void *pEventInfo);
static void __mp_share_via_view_rotate_cb(void *data, Evas_Object *obj,
	void *event_info);

static bool __mp_share_view_get_selected_video_list(int nSelectedSize);
/* static void __mp_share_view_update_selected_video_list(void); */

static void __mp_share_view_free_selected_list(void)
{
	if (!g_pShareViaHandle || !g_pShareViaHandle->pSelectedList) {
		return;
	}
	int nIndex = 0;
	while (g_pShareViaHandle->pSelectedList[nIndex]) {
		nIndex++;
		MP_FREE_STRING(g_pShareViaHandle->pSelectedList[nIndex]);
	}
	MP_FREE_STRING(g_pShareViaHandle->pSelectedList);
}

void mp_share_view_update_all_check_box(void)
{
	VideoLogInfo("");
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	if (!g_pShareViaHandle->pObjVideosList) {
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
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	if (g_pShareViaHandle->bThumbnailView) {
		currentItem = elm_genlist_first_item_get(
			g_pShareViaHandle->pObjVideosList);
		nVideoItemIndex = 0;
		while (currentItem && (nVideoItemIndex < nVideoListSize)) {
			Evas_Object *gengrid = elm_object_item_part_content_get(
				currentItem, "elm.icon");
			currentItemGengrid = elm_gengrid_first_item_get(gengrid);
			while (currentItemGengrid && (nVideoItemIndex < nVideoListSize)) {
				Evas_Object *check_box =
					elm_object_item_part_content_get(
						currentItemGengrid,
						"elm.swallow.end");
				if (check_box) {
					elm_check_state_set(check_box,
					g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex]);
				}
				currentItemGengrid = elm_gengrid_item_next_get(currentItemGengrid);
				nVideoItemIndex++;
			}
			currentItem = elm_genlist_item_next_get(currentItem);
		}

	} else
#endif
	{
		currentItem = elm_genlist_first_item_get(g_pShareViaHandle->pObjVideosList);
		nVideoItemIndex = 0;
		while (currentItem && (nVideoItemIndex < nVideoListSize)) {
			check_box_layout = elm_object_item_part_content_get(
						currentItem, "elm.icon.2");
			check_box = elm_object_part_content_get(check_box_layout,
					"elm.swallow.content");
			if (check_box) {
				elm_check_state_set(check_box,
					g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex]);
			}
			currentItem = elm_genlist_item_next_get(currentItem);
			nVideoItemIndex++;
		}
	}
}

void mp_share_view_destroy(void)
{
	VideoLogInfo("");

	if (g_pShareViaHandle) {
		evas_object_smart_callback_del(g_pShareViaHandle->pNaviFrameHandle,
		"transition,finished", mp_share_view_naviframe_transition_effect_cb);
		__mp_share_view_free_selected_list();

		MP_DEL_OBJ(g_pShareViaHandle->pObjVideosList);
		MP_DEL_ITC(g_pShareViaHandle->pGenListItc);
		MP_DEL_ITC(g_pShareViaHandle->pGenListLandscapeItc);
		MP_DEL_GRID_ITC(g_pShareViaHandle->pGenGridItc_1);
		MP_DEL_GRID_ITC(g_pShareViaHandle->pGenGridItc_2);
		MP_FREE_STRING(g_pShareViaHandle->pCheckedItemsEditList);
		MP_FREE_STRING(g_pShareViaHandle->pFolderPath);
		evas_object_smart_callback_del(
			(Evas_Object *)mp_util_get_main_window_handle(),
			"wm,rotation,changed", __mp_share_via_view_rotate_cb);

		MP_FREE_STRING(g_pShareViaHandle);
	}

	if (g_pShareViaWidget) {

		MP_DEL_OBJ(g_pShareViaWidget->pSelectallbtn);
		MP_DEL_OBJ(g_pShareViaWidget->pSelAllLayout);
		MP_DEL_OBJ(g_pShareViaWidget->pVideoListBox);
		MP_DEL_OBJ(g_pShareViaWidget->pChareViaViewBaselayout);
		MP_DEL_OBJ(g_pShareViaWidget->pSavebtn);
		MP_DEL_OBJ(g_pShareViaWidget->pCancelbtn);

		MP_FREE_STRING(g_pShareViaWidget);
	}
}

static void __mp_share_view_reset(void)
{
	VideoLogInfo("");

	if (g_pShareViaHandle) {
		mp_util_db_set_update_fun(g_pShareViaHandle->euLev, NULL);
		mp_util_db_set_backup_fun(g_pShareViaHandle->euLev, NULL);
		evas_object_smart_callback_del(
			g_pShareViaHandle->pNaviFrameHandle,
			"transition,finished",
			mp_share_view_naviframe_transition_effect_cb);
		evas_object_smart_callback_del((Evas_Object *)mp_util_get_main_window_handle(),
			"wm,rotation,changed", __mp_share_via_view_rotate_cb);
		__mp_share_view_free_selected_list();

		g_pShareViaHandle->pObjVideosList = NULL;

		MP_DEL_ITC(g_pShareViaHandle->pGenListItc);
		MP_DEL_ITC(g_pShareViaHandle->pGenListLandscapeItc);
		MP_DEL_GRID_ITC(g_pShareViaHandle->pGenGridItc_1);
		MP_DEL_GRID_ITC(g_pShareViaHandle->pGenGridItc_2);
		MP_FREE_STRING(g_pShareViaHandle->pCheckedItemsEditList);
		MP_FREE_STRING(g_pShareViaHandle->pFolderPath);

		MP_FREE_STRING(g_pShareViaHandle);
	}

	if (g_pShareViaWidget) {
		MP_DEL_OBJ(g_pShareViaWidget->pSelectallbtn);
		g_pShareViaWidget->pVideoListBox = NULL;
		g_pShareViaWidget->pSelAllLayout = NULL;

		g_pShareViaWidget->pChareViaViewBaselayout = NULL;
		MP_DEL_OBJ(g_pShareViaWidget->pSavebtn);
		MP_DEL_OBJ(g_pShareViaWidget->pCancelbtn);

		MP_FREE_STRING(g_pShareViaWidget);
	}
}

void mp_share_view_reset_value(void)
{
	if (!g_pShareViaHandle) {
		VideoLogError("[ERR] g_pShareViaHandle is not existed.");
		return;
	}
	VideoLogInfo("");

	g_pShareViaHandle->pNaviFrameHandle = NULL;
	g_pShareViaHandle->pNaviFrameItem = NULL;
	g_pShareViaHandle->pChangeViewUserCbFunc = NULL;

	g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
	g_pShareViaHandle->nSelectCounterForEditList = 0;
	g_pShareViaHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;

	if (g_pShareViaHandle->pNaviFrameItem) {
		elm_object_item_part_content_unset(g_pShareViaHandle->pNaviFrameItem, "toolbar_more_btn");
		elm_object_item_part_content_unset(g_pShareViaHandle->pNaviFrameItem, "toolbar");
		elm_object_item_part_content_unset(g_pShareViaHandle->pNaviFrameItem, "toolbar_button2");
	}
}

void mp_share_view_launching_share_panel(void)
{
	if (!g_pShareViaHandle || !g_pShareViaHandle->pCheckedItemsEditList) {
		VideoLogError("g_pShareViaHandle is null.");
		return;
	}

	VideoLogInfo("");

	int nErr = 0;
	int nCounter = 0;
	int nTotalVideoItem = 0;
	int nTotalAppendedItem = 0;
	int nExtraDataSize = 0;

	app_control_h pService = NULL;

	char *pOperation = VIDEO_SHARE_OPERATION_SINGLE;
	char *szFileUrlForServiceSetUri = NULL;
	char **szFileUrlArrayForServiceExtraData = NULL;

	nTotalVideoItem = mp_util_svc_get_number_of_video_item_by_type();
	nExtraDataSize = nTotalVideoItem;

	if (nExtraDataSize) {
		szFileUrlArrayForServiceExtraData = (char **)calloc(
			nExtraDataSize, sizeof(char *));
	}

	/* Extract First URI. */
	for (nCounter = 0; nCounter < nTotalVideoItem; nCounter++) {
		if (g_pShareViaHandle->pCheckedItemsEditList[nCounter]) {
			if (mp_util_svc_is_cloud_storage(nCounter)) {
				continue;
			}

			char *szFileUrl = mp_util_svc_get_video_url(nCounter);

			if (szFileUrl) {
				szFileUrlForServiceSetUri = szFileUrl;
				nCounter++;
				break;
			} else {
				VideoLogError("szFileUrl is not existed.");
			}
		}
	}

	VideoLogInfo("nCounter = %d", nCounter);

	/* Extract another URI if existing selected items more. */
	if (nExtraDataSize > 0 && szFileUrlArrayForServiceExtraData) {
		for (nCounter = 0; nCounter < nTotalVideoItem; nCounter++) {
			if (g_pShareViaHandle->pCheckedItemsEditList[nCounter]) {
				if (mp_util_svc_is_cloud_storage(nCounter)) {
					continue;
				}

				char *szFileUrl = mp_util_svc_get_video_url(nCounter);

				if (szFileUrl) {
					if (nTotalAppendedItem < nExtraDataSize) {
						szFileUrlArrayForServiceExtraData[nTotalAppendedItem] = szFileUrl;
						VideoSecureLogInfo("Appending share item : %s, Total Appended item : %d", (const char **)szFileUrlArrayForServiceExtraData[nTotalAppendedItem], nTotalAppendedItem);
						nTotalAppendedItem++;
						continue;
					} else {
						VideoLogWarning("[WARNING] OVERFLOW ALOOCATED MEMORY SIZE.");
						free(szFileUrl);
					}
				} else {
					VideoLogError("szFileUrl is not existed.");
				}
			}
		}
	}

	if (nTotalAppendedItem > 1) {
		pOperation = VIDEO_SHARE_OPERATION_MULTIPLE;
	}

	if (!szFileUrlForServiceSetUri) {
		VideoLogWarning("[WARNING] Fail to get uri at first.");
		goto ERROR_EXCEPTION;
	}

	nErr = app_control_create(&pService);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("Fail to create ug service handle - [0x%x]", nErr);
		goto ERROR_EXCEPTION;
	}

	nErr = app_control_set_uri(pService, szFileUrlForServiceSetUri);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("Fail to set uri into ug service handle - [0x%x]", nErr);
		goto ERROR_EXCEPTION;
	}

	nErr = app_control_set_operation(pService, pOperation);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogInfo("app_control_add_extra_data_array().. [0x%x]", nErr);
		goto ERROR_EXCEPTION;
	}
	if (nTotalAppendedItem >= 1) {
		nErr = app_control_add_extra_data_array(pService,
			"http://tizen.org/appcontrol/data/path",
			(const char **)szFileUrlArrayForServiceExtraData,
			nTotalAppendedItem);
		if (nErr != APP_CONTROL_ERROR_NONE) {
			VideoLogWarning("[WARNING] app_control_add_extra_data_array() [0x%x]", nErr);
			goto ERROR_EXCEPTION;
		}
	}
/*
	nErr = app_control_set_window(pService, elm_win_xwindow_get(
			mp_util_get_main_window_handle()));
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogWarning("[WARNING] app_control_set_window()... [0x%x]", nErr);
		goto ERROR_EXCEPTION;
	}
*/
	nErr = app_control_send_launch_request(pService, NULL, NULL);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogWarning("[WARNING] app_control_send_launch_request().. [0x%x]", nErr);
		goto ERROR_EXCEPTION;
	}
	elm_naviframe_item_pop(g_pShareViaHandle->pNaviFrameHandle);
	app_control_destroy(pService);
	pService = NULL;

	VideoLogInfo("Launching share panel.");

ERROR_EXCEPTION:
	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}

	if (szFileUrlForServiceSetUri) {
		free(szFileUrlForServiceSetUri);
		szFileUrlForServiceSetUri = NULL;
	}

	if (szFileUrlArrayForServiceExtraData) {
		for (nCounter = 0; nCounter < nExtraDataSize; nCounter++) {
			if (szFileUrlArrayForServiceExtraData[nCounter]) {
				free(szFileUrlArrayForServiceExtraData[nCounter]);
				szFileUrlArrayForServiceExtraData[nCounter] = NULL;
			}
		}
		free(szFileUrlArrayForServiceExtraData);
		szFileUrlArrayForServiceExtraData = NULL;
	}

	return;
}

void __mp_share_view_show_cancel_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	if (!g_pShareViaHandle->pNaviFrameHandle) {
		VideoSecureLogError("g_pShareViaHandle->pNaviFrameHandle is NULL");
		return;
	}

	VideoLogInfo("");

	elm_naviframe_item_pop(g_pShareViaHandle->pNaviFrameHandle);
}

void __mp_share_view_show_share_via_panel_cb(void *data, Evas_Object *obj,
	const char *emission, const char *source)
{
	if (!g_pShareViaHandle) {
		VideoLogInfo("[ERR] g_pMainViewHandle is NULL.");
		return;
	}

	VideoLogInfo("");

	if (!g_pShareViaHandle->pObjVideosList) {
		VideoLogInfo("No exist genlist object.");
		return;
	}

	VideoLogInfo("[INFO]nSelectCounterForEditList : %d",
		g_pShareViaHandle->nSelectCounterForEditList);
	if (g_pShareViaHandle->nSelectCounterForEditList > 0) {
		mp_share_view_launching_share_panel();
		/* g_pShareViaHandle->nSelectCounterForEditList = 0; */
	}
}

static void __mp_share_view_trans_finished_cb(void *pUserData,
	Evas_Object *obj, void *event_info)
{
	if (!obj) {
		VideoLogError("[ERR] g_pShareViaHandle is NULL.");
		return;
	}

	mp_widget_ctrl_enable_navi_handle_focus(obj);

	evas_object_smart_callback_del(obj, "transition,finished",
		__mp_share_view_trans_finished_cb);
}

static Eina_Bool mp_share_view_back_btn_cb(void *pUserData,
	Elm_Object_Item *pItem)
{
	if (!g_pShareViaHandle) {
		VideoLogInfo("[ERR] g_pShareViaHandle is NULL.");
		return EINA_TRUE;
	}

	mp_widget_ctrl_disable_navi_handle_focus_except_item(
		g_pShareViaHandle->pNaviFrameHandle,
		g_pShareViaHandle->pNaviFrameItem);
	evas_object_smart_callback_add(g_pShareViaHandle->pNaviFrameHandle,
		"transition,finished", __mp_share_view_trans_finished_cb,
		NULL);

	mp_share_view_pop();

	return EINA_TRUE;
}

void mp_share_view_change_title_info(void)
{
	VideoLogInfo("nNumOfSelectdCount : %d",
		g_pShareViaHandle->nSelectCounterForEditList);

	if (g_pShareViaHandle->nSelectCounterForEditList > 0) {
		elm_object_disabled_set(g_pShareViaWidget->pSavebtn, EINA_FALSE);
	} else {
		VideoLogInfo("Unactivate delete tab.");
		elm_object_disabled_set(g_pShareViaWidget->pSavebtn, EINA_TRUE);
	}

	char *szCheckingTotalItem = g_strdup_printf(
		VIDEOS_SELECTVIEW_HEADER_PD_SELECTED,
		g_pShareViaHandle->nSelectCounterForEditList);
	elm_object_item_domain_translatable_part_text_set(
		g_pShareViaHandle->pNaviFrameItem, "elm.text.title",
		VIDEOS_STRING, szCheckingTotalItem);
	MP_FREE_STRING(szCheckingTotalItem);
}

static void __mp_share_view_select_all_change_check_box_cb(void *pUserData,
	Evas_Object *pObject, void *pEventInfo)
{
	if (!pObject) {
		VideoLogError("[ERR]");
		return;
	}

	if (!g_pShareViaHandle->pObjVideosList) {
		VideoLogError("No exist pVideoGenlist");
		return;
	}

	int nCounter = 0;
	int nVideoListSize = 0;

	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	if (!g_pShareViaHandle->bSelectallCheckBoxState) {
		if (nVideoListSize > VIDEO_SHARE_ITEM_MAX) {
			VideoLogError("nVideoListSize is bigger than VIDEO_SHARE_ITEM_MAX");
			char *pStrWarning = g_strdup_printf(
				VIDEOS_NOTIPOP_MSG_SHARE_ITEM_MAX,
				VIDEO_SHARE_ITEM_MAX);
			mp_util_ticker_toast_popup(pStrWarning, false, false);
			MP_FREE_STRING(pStrWarning);
			elm_check_state_set(g_pShareViaWidget->pSelectallbtn, g_pShareViaHandle->bSelectallCheckBoxState);
			return;
		}
		g_pShareViaHandle->bSelectallCheckBoxState = TRUE;
		g_pShareViaHandle->nSelectCounterForEditList = nVideoListSize;
	} else {
		g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
		g_pShareViaHandle->nSelectCounterForEditList = 0;

		VideoLogInfo("Unactivate delete button.");
		elm_object_disabled_set(g_pShareViaWidget->pSavebtn, EINA_TRUE);
	}

	elm_check_state_set(g_pShareViaWidget->pSelectallbtn,
		g_pShareViaHandle->bSelectallCheckBoxState);

	mp_share_view_change_title_info();

	for (nCounter = 0; nCounter < nVideoListSize; nCounter++) {
		g_pShareViaHandle->pCheckedItemsEditList[nCounter] =
			g_pShareViaHandle->bSelectallCheckBoxState;
	}

	mp_share_view_update_all_check_box();

	/* elm_genlist_realized_items_update(g_pShareViaHandle->pObjVideosList); */
}

void mp_share_view_delete_select_all(void)
{
	VideoLogInfo("");

	g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
	g_pShareViaHandle->nSelectCounterForEditList = 0;

	mp_share_view_change_title_info();
}

/*
Check box callback
*/
void mp_share_view_change_check_box_state_cb(void *pUserData,
	Evas_Object *pObject, void *pEventInfo)
{
	if (!pObject) {
		VideoLogError("[ERR]");
		return;
	} else {
		VideoLogError("pObject : %x", pObject);
	}

	int nVideoItemIndex = (int)pUserData;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	bool bState = elm_check_state_get(pObject);

	if (bState) {
		if (g_pShareViaHandle->nSelectCounterForEditList >
			(VIDEO_SHARE_ITEM_MAX - 1)) {
			char *pStrWarning = g_strdup_printf(
				VIDEOS_NOTIPOP_MSG_SHARE_ITEM_MAX,
				VIDEO_SHARE_ITEM_MAX);
			mp_util_ticker_toast_popup(pStrWarning, false, false);
			MP_FREE_STRING(pStrWarning);

			if (pObject) {
				elm_check_state_set(pObject, FALSE);
			}
			return;
		}
		g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex] = TRUE;
		g_pShareViaHandle->nSelectCounterForEditList++;
		if (nVideoListSize ==
			g_pShareViaHandle->nSelectCounterForEditList) {
			VideoLogInfo("All check box of edit list state are true.");
			g_pShareViaHandle->bSelectallCheckBoxState = TRUE;
		}
	} else {
		g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex] = FALSE;
		if (nVideoListSize == g_pShareViaHandle->nSelectCounterForEditList) {
			VideoLogInfo("All check box of edit list state are not true.");
			g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
		}

		g_pShareViaHandle->nSelectCounterForEditList--;
		if (g_pShareViaHandle->nSelectCounterForEditList < 1) {
			VideoLogInfo("Unactivate delete button.");
			elm_object_disabled_set(g_pShareViaWidget->pSavebtn, EINA_TRUE);
		}
	}

	elm_check_state_set(g_pShareViaWidget->pSelectallbtn,
		g_pShareViaHandle->bSelectallCheckBoxState);
	mp_share_view_change_title_info();
/*
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
*/
}

/*
Genlist callback
*/
void mp_share_view_naviframe_transition_effect_cb(void *pUserData,
	Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pShareViaHandle) {
		VideoLogError("[ERR] No exist g_pShareViaHandle.");
		return;
	}

	VideoLogInfo("");

	Evas_Object *pTmpContents = NULL;
	Elm_Object_Item *pTmpItem = NULL;

	if (!g_pShareViaHandle->pNaviFrameHandle) {
		VideoLogInfo("[ERR] No exist naviframe handle.");
		return;
	}

	pTmpItem = elm_naviframe_top_item_get(g_pShareViaHandle->pNaviFrameHandle);
	pTmpContents = elm_object_item_content_get(pTmpItem);
	if (pTmpContents) {
		if (pTmpContents == g_pShareViaWidget->pVideoListBox) {
		} else {
			VideoLogInfo("It's not pListShareViaViewTabbarLayout in top screen.");
		}
	} else {
		VideoLogError("[ERR] No exist naviframe top item.");
	}
}

void mp_share_view_double_tap_cb(void)
{
	if (!g_pShareViaHandle->pObjVideosList) {
		VideoLogError("pObjVideosList is not exist.");
		return;
	}

	VideoLogInfo("");

	elm_genlist_item_bring_in(elm_genlist_first_item_get(
		g_pShareViaHandle->pObjVideosList),
		ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

char *mp_share_view_get_duration_time(int nVideoItemIndex)
{
	VideoLogInfo("");

	char szTmpStr[STR_LEN_MAX] = {0,};
	char szTmpStrDurationTime[STR_LEN_MAX] = {0,};
	char szTmpStrLastPlayedPos[STR_LEN_MAX] = {0,};

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

static void __mp_share_view_select_grid_item_cb(void *pUserData,
	Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pShareViaHandle->pObjVideosList) {
		VideoLogError("No exist genlist object.");
		return;
	}

	VideoLogInfo("");

	int nVideoItemIndex = 0;

	Elm_Object_Item *pItem = (Elm_Object_Item *)pEventInfo;
	Elm_Object_Item *pSelectedItem = elm_gengrid_selected_item_get(pObject);

	if (pItem) {
		nVideoItemIndex = (int)elm_object_item_data_get(pItem);
		VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

		int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

		VideoLogInfo("BEFORE : nSelectCounterForEditList : %d", g_pShareViaHandle->nSelectCounterForEditList);

		if (g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex]) {
			g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex] = FALSE;
			if (nVideoListSize == g_pShareViaHandle->nSelectCounterForEditList) {
				VideoLogInfo("All check box of edit list state are not true.");
				g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
			}

			g_pShareViaHandle->nSelectCounterForEditList--;
			if (g_pShareViaHandle->nSelectCounterForEditList < 1) {
				VideoLogInfo("Unactivate delete button.");
				elm_object_disabled_set(g_pShareViaWidget->pSavebtn, EINA_TRUE);
			}
		} else {
			VideoLogInfo("selected count = %d", g_pShareViaHandle->nSelectCounterForEditList);
			if (g_pShareViaHandle->nSelectCounterForEditList > (VIDEO_SHARE_ITEM_MAX - 1)) {
				char *pStrWarning = g_strdup_printf(VIDEOS_NOTIPOP_MSG_SHARE_ITEM_MAX, VIDEO_SHARE_ITEM_MAX);
				mp_util_ticker_toast_popup(pStrWarning, false, false);
				MP_FREE_STRING(pStrWarning);
				if (pSelectedItem) {
					elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
					elm_gengrid_item_update(pSelectedItem);
				}
				return;
			}
			g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex] = TRUE;
			g_pShareViaHandle->nSelectCounterForEditList++;
			if (nVideoListSize == g_pShareViaHandle->nSelectCounterForEditList) {
				VideoLogInfo("All check box of edit list state are true.");
				g_pShareViaHandle->bSelectallCheckBoxState = TRUE;
			}
		}

		VideoLogInfo("AFTER : nSelectCounterForEditList : %d", g_pShareViaHandle->nSelectCounterForEditList);
		elm_check_state_set(g_pShareViaWidget->pSelectallbtn, g_pShareViaHandle->bSelectallCheckBoxState);

		mp_share_view_change_title_info();
		elm_gengrid_item_update(pItem);
	} else {
		VideoLogInfo("[ERR]Error item number.");
	}
}

void mp_share_view_select_video_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pShareViaHandle->pObjVideosList) {
		VideoLogError("No exist genlist object.");
		return;
	}

	VideoLogInfo("");

	int nVideoItemIndex = 0;

	Elm_Object_Item *pItem = (Elm_Object_Item *)pEventInfo;
	Elm_Object_Item *pSelectedItem = elm_genlist_selected_item_get(pObject);

	if (pItem) {
		nVideoItemIndex = (int)elm_object_item_data_get(pItem);
		VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

		int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

		VideoLogInfo("BEFORE : nSelectCounterForEditList : %d", g_pShareViaHandle->nSelectCounterForEditList);

		if (g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex]) {
			g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex] = FALSE;
			if (nVideoListSize == g_pShareViaHandle->nSelectCounterForEditList) {
				VideoLogInfo("All check box of edit list state are not true.");
				g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
			}

			g_pShareViaHandle->nSelectCounterForEditList--;
			if (g_pShareViaHandle->nSelectCounterForEditList < 1) {
				VideoLogInfo("Unactivate delete button.");
				elm_object_disabled_set(g_pShareViaWidget->pSavebtn, EINA_TRUE);
			}
		} else {
			VideoLogInfo("Unactivate delete button. = %d", g_pShareViaHandle->nSelectCounterForEditList);
			if (g_pShareViaHandle->nSelectCounterForEditList > (VIDEO_SHARE_ITEM_MAX - 1)) {
				char *pStrWarning = g_strdup_printf(VIDEOS_NOTIPOP_MSG_SHARE_ITEM_MAX, VIDEO_SHARE_ITEM_MAX);
				mp_util_ticker_toast_popup(pStrWarning, false, false);
				MP_FREE_STRING(pStrWarning);
				if (pSelectedItem) {
					elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
				}
				return;
			}
			g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex] = TRUE;
			g_pShareViaHandle->nSelectCounterForEditList++;
			if (nVideoListSize == g_pShareViaHandle->nSelectCounterForEditList) {
				VideoLogInfo("All check box of edit list state are true.");
				g_pShareViaHandle->bSelectallCheckBoxState = TRUE;
			}
		}

		VideoLogInfo("AFTER : nSelectCounterForEditList : %d", g_pShareViaHandle->nSelectCounterForEditList);
		elm_check_state_set(g_pShareViaWidget->pSelectallbtn, g_pShareViaHandle->bSelectallCheckBoxState);
		mp_share_view_change_title_info();
		Evas_Object *check_box_layout = elm_object_item_part_content_get(pItem, "elm.icon.2");
		Evas_Object *check_box = elm_object_part_content_get(check_box_layout, "elm.swallow.content");
		if (check_box) {
			elm_check_state_set(check_box, g_pShareViaHandle->pCheckedItemsEditList[nVideoItemIndex]);
		}

		/* elm_genlist_item_fields_update(pSelectedItem, "elm.edit.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT); */
	} else {
		VideoLogInfo("[ERR]Error item number.");
	}

	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW
static char *__mp_share_view_get_label_of_grid_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
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

char *mp_share_view_get_label_of_video_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text.main.left.top")) {
		char *szTitle = NULL;
		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		if (szTitle) {
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.sub.left.bottom")) {
		return mp_util_svc_get_duration_str_time(nVideoItemIndex);
	}
/*
	else if (!strcmp(pPart, "elm.text.sub.right.bottom")) {
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
	}
*/

	return NULL;
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW
static Evas_Object *__mp_share_view_get_grid_icon_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
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
		int nLastPlayedTime =
			mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime =
			mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime <= 0) {
			return NULL;
		}

		double nRatio = 0.0;
		if (nDurationTime > 0) {
			nRatio = ((double)nLastPlayedTime) / ((double)nDurationTime);
		}

		if (nRatio < 0.05) {
			nRatio = 0.05;
		}
		Evas_Object *pProgressBar = elm_progressbar_add(pObject);
		elm_object_style_set(pProgressBar, "video/list_progress");
		elm_progressbar_horizontal_set(pProgressBar, EINA_TRUE);
		evas_object_size_hint_align_set(pProgressBar,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(pProgressBar,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_progressbar_value_set(pProgressBar, nRatio);
		evas_object_show(pProgressBar);

		return pProgressBar;
	} else if (!strcmp(pPart, "elm.lock.icon")) {
		char *pVideoFile = mp_util_svc_get_video_url(nVideoItemIndex);
		bool bRet = mp_util_check_video_personal_status(
				(const char *)pVideoFile);
		MP_FREE_STRING(pVideoFile);
		if (bRet) {
			Evas_Object *pLockIcon = elm_image_add(pObject);
			elm_image_file_set(pLockIcon,
				edj_path,
				VIDEO_LIST_VIEW_ICON_PERSONAL_LOCK);
			evas_object_size_hint_align_set(pLockIcon,
				EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pLockIcon,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(pLockIcon);
			return pLockIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.dropbox.icon")) {
		bool bRet = mp_util_svc_is_cloud_storage(nVideoItemIndex);
		if (bRet) {
			Evas_Object *pDropboxIcon = elm_image_add(pObject);
			elm_image_file_set(pDropboxIcon,
				edj_path,
				VIDEO_LIST_VIEW_ICON_DROPBOX);
			evas_object_size_hint_align_set(pDropboxIcon,
				EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pDropboxIcon,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(pDropboxIcon);
			return pDropboxIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.swallow.end")) {
		Evas_Object *pCheckBox = elm_check_add(pObject);
		elm_object_style_set(pCheckBox, "default");
		elm_check_state_pointer_set(pCheckBox,
			(Eina_Bool *)&(g_pShareViaHandle->pCheckedItemsEditList
			[nVideoItemIndex]));

		evas_object_show(pCheckBox);

		return pCheckBox;
	}
	return NULL;
}
#endif

Evas_Object *mp_share_view_get_icon_of_video_item_cb(const void *pUserData,
	Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;
	Evas_Object *pLayout = NULL;

	if (!strcmp(pPart, "elm.icon.1")) {
		pLayout = elm_layout_add(pObject);
		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(
					nVideoItemIndex);
		Evas_Object *pBg = NULL;

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg = mp_util_create_preload_image(pLayout, pThumbIconUri,
			VIDEO_ICON_WIDTH);
		if (pBg)
			evas_object_show(pBg);
		char edj_path[1024] = {0};

		char *path = app_get_resource_path();
		snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VIDEO_CUSTOM_THEME);
		free(path);
		elm_layout_file_set(pLayout, edj_path,
			"listview.thumbnail.layout");
		elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg);

		MP_FREE_STRING(pThumbIconUri);

		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(
					nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(
					nVideoItemIndex);

		if (nLastPlayedTime > 0) {
			double nRatio = 0.0;

			if (nDurationTime > 0) {
				nRatio = ((double)nLastPlayedTime) /
						((double)nDurationTime);
			}

			if (nRatio < 0.05) {
				nRatio = 0.05;
			}
			Evas_Object *pProgressBar = elm_progressbar_add(
							pLayout);
			elm_object_style_set(pProgressBar,
				"elm/progressbar/horizontal/default");
			elm_progressbar_horizontal_set(pProgressBar,
				EINA_TRUE);
			elm_progressbar_span_size_set(pProgressBar,
				VIDEO_LIST_PROGRESSBAR_LENGTH);
			evas_object_size_hint_align_set(pProgressBar,
				EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pProgressBar,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_value_set(pProgressBar, nRatio);
			evas_object_show(pProgressBar);

			elm_layout_content_set(pLayout, "elm.progress.icon",
				pProgressBar);
		}

		if (!mp_util_create_video_sub_icon(pLayout, nVideoItemIndex)) {
			VideoLogWarning("Create sub icon is failed.",
				nVideoItemIndex);
		}

		return pLayout;
	} else if (!strcmp(pPart, "elm.icon.2")) {
		pLayout = elm_layout_add(pObject);
		Evas_Object *pCheckBox = elm_check_add(pLayout);

		elm_layout_theme_set(pLayout, "layout", "list/C/type.2",
			"default");
		evas_object_propagate_events_set(pCheckBox, EINA_FALSE);
		evas_object_smart_callback_add(pCheckBox, "changed",
			mp_share_view_change_check_box_state_cb, pUserData);
		elm_check_state_pointer_set(pCheckBox,
			(Eina_Bool *)&(g_pShareViaHandle->pCheckedItemsEditList
			[nVideoItemIndex]));

		evas_object_show(pCheckBox);

		elm_layout_content_set(pLayout, "elm.swallow.content",
			pCheckBox);

		return pLayout;
	}

	MP_DEL_OBJ(pLayout);
	return NULL;
}

int mp_share_view_get_sort_type(void)
{
	switch (mp_sort_ctrl_get_sort_state()) {
/*
	case MP_LIST_SORT_BY_RECENTLY_VIEWED:
		g_pShareViaHandle->nSortType =
			MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
		break;
*/

	case MP_LIST_SORT_BY_RECENTLY_ADDED:
		g_pShareViaHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_ADDED;
		break;

	case MP_LIST_SORT_BY_NAME:
		g_pShareViaHandle->nSortType = MP_MEDIA_SORT_BY_NAME;
		break;

/*
	case MP_LIST_SORT_BY_SIZE:
		g_pShareViaHandle->nSortType = MP_MEDIA_SORT_BY_SIZE;
		break;

	case MP_LIST_SORT_BY_TYPE:
		g_pShareViaHandle->nSortType = MP_MEDIA_SORT_BY_TYPE;
		break;
*/
	}

	return g_pShareViaHandle->nSortType;
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW
static void __mp_share_view_append_grid_items(void *pObjVideosList,
	int nGenItemIndex)
{
	if (!g_pShareViaHandle) {
		VideoLogInfo("invalid handle");
		return;
	}
	MP_DEL_GRID_ITC(g_pShareViaHandle->pGenGridItc_1);
	MP_DEL_GRID_ITC(g_pShareViaHandle->pGenGridItc_2);
	g_pShareViaHandle->pGenGridItc_1 = elm_gengrid_item_class_new();
	g_pShareViaHandle->pGenGridItc_1->item_style = "video/album_grid";
	g_pShareViaHandle->pGenGridItc_1->func.text_get =
		(void *)__mp_share_view_get_label_of_grid_item_cb;
	g_pShareViaHandle->pGenGridItc_1->func.content_get =
		(void *)__mp_share_view_get_grid_icon_cb;
	g_pShareViaHandle->pGenGridItc_1->func.state_get = NULL;
	g_pShareViaHandle->pGenGridItc_1->func.del = NULL;

	int nCount = (mp_rotate_ctrl_check_landspace_by_win(
			(Evas_Object *)mp_util_get_main_window_handle()) ?
			VP_GENLIST_THUMB_LANDSPACE_COUNT :
			VP_GENLIST_THUMB_PORTRAIT_COUNT);
	int nIndex = nGenItemIndex * nCount;
	int nMaxIndex = nIndex + nCount;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	Elm_Object_Item *gridItem = NULL;
	for (; (nIndex < nMaxIndex) && (nIndex < nVideoListSize); nIndex++) {
		gridItem = elm_gengrid_item_append(pObjVideosList, g_pShareViaHandle->pGenGridItc_1, (void *)nIndex, __mp_share_view_select_grid_item_cb, (void *)nIndex);
		elm_gengrid_item_select_mode_set(gridItem, ELM_OBJECT_SELECT_MODE_ALWAYS);
		elm_object_item_data_set(gridItem, (void *)nIndex);
	}
}

static Evas_Object *__mp_share_view_create_gengrid(Evas_Object *parent,
	int nWidth, int nHeight)
{
	Evas_Object *pGengrid;
	pGengrid = elm_gengrid_add(parent);
	if (!pGengrid) {
		VideoLogError("pVideoGrid is not existed.");
			return NULL;
	}
	evas_object_size_hint_weight_set(pGengrid, EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pGengrid, EVAS_HINT_FILL,
		EVAS_HINT_FILL);
	elm_gengrid_item_size_set(pGengrid, nWidth, nHeight);
	elm_gengrid_align_set(pGengrid, 0.0, 0.0);
	elm_gengrid_horizontal_set(pGengrid, EINA_TRUE);

	return pGengrid;
}

static void mp_share_view_get_icon_dimensions(int *height, int *width)
{
	int winHeight = 0;
	int winWidth = 0;
	elm_win_screen_size_get(mp_util_get_main_window_handle(), NULL, NULL,
				&winWidth, &winHeight);

	bool bLandscape = mp_rotate_ctrl_check_landspace_by_win(
		(Evas_Object *)mp_util_get_main_window_handle());
	if (bLandscape) {
		*width = winHeight / VP_GENLIST_THUMB_LANDSPACE_COUNT;
	} else {
		*width = winWidth / VP_GENLIST_THUMB_PORTRAIT_COUNT;
	}
	*height = VP_ALBUM_THUMB_ICON_HEIGHT * MP_SCALE;
}

static Evas_Object *__mp_share_view_get_icon_of_grid_cb(const void *pUserData,
	Evas_Object *pObject, const char *pPart)
{
	if (!g_pShareViaHandle || !pObject) {
		VideoLogError("g_pMainViewWidgetOfShareList is NULL");
		return NULL;
	}

	int nGenItemIndex = (int)pUserData;
	int nHeight = 0;
	int nWidth = 0;
	mp_share_view_get_icon_dimensions(&nHeight, &nWidth);

	Evas_Object *pGengrid = __mp_share_view_create_gengrid(pObject,
		nWidth, nHeight);
	__mp_share_view_append_grid_items(pGengrid, nGenItemIndex);

	return pGengrid;
}

int mp_share_view_thumbnail_to_genlist_index(int nVideoItemIndex)
{
	if (nVideoItemIndex < 0) {
		VideoLogDebug("invalid nVideoItemIndex");
		return 0;
	}
	int nGenlistIndex = 0;
	int nItemCount = (mp_rotate_ctrl_check_landspace_by_win(
		(Evas_Object *)mp_util_get_main_window_handle()) ?
		VP_GENLIST_THUMB_LANDSPACE_COUNT :
		VP_GENLIST_THUMB_PORTRAIT_COUNT);
	nGenlistIndex = nVideoItemIndex/nItemCount;
	return nGenlistIndex;
}

static bool __mp_share_view_append_grid_layout(void *pGenlist)
{
	if (!pGenlist || !g_pShareViaHandle) {
		VideoLogError("error handle.");
		return FALSE;
	}
	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	if (nVideoListSize <= 0) {
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);
		return FALSE;
	} else {
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);
	}

	MP_DEL_ITC(g_pShareViaHandle->pGenListItc);
	g_pShareViaHandle->pGenListItc = elm_genlist_item_class_new();
	g_pShareViaHandle->pGenListItc->item_style = "video/1icon.thumbnail";
	g_pShareViaHandle->pGenListItc->func.text_get = NULL;
	g_pShareViaHandle->pGenListItc->func.content_get =
		(void *)__mp_share_view_get_icon_of_grid_cb;
	g_pShareViaHandle->pGenListItc->func.state_get = NULL;
	g_pShareViaHandle->pGenListItc->func.del = NULL;
	g_pShareViaHandle->pGenListItc->decorate_item_style = NULL;

	MP_DEL_ITC(g_pShareViaHandle->pGenListLandscapeItc);
	g_pShareViaHandle->pGenListLandscapeItc = elm_genlist_item_class_new();
	g_pShareViaHandle->pGenListLandscapeItc->item_style =
		"video/1icon.thumbnail.landscape";
	g_pShareViaHandle->pGenListLandscapeItc->func.text_get = NULL;
	g_pShareViaHandle->pGenListLandscapeItc->func.content_get =
		(void *)__mp_share_view_get_icon_of_grid_cb;
	g_pShareViaHandle->pGenListLandscapeItc->func.state_get = NULL;
	g_pShareViaHandle->pGenListLandscapeItc->func.del = NULL;
	g_pShareViaHandle->pGenListLandscapeItc->decorate_item_style = NULL;

	int nGenlistRow = 0;
	nGenlistRow = mp_share_view_thumbnail_to_genlist_index(
			nVideoListSize - 1);

	if (mp_rotate_ctrl_check_landspace_by_win(
		(Evas_Object *)mp_util_get_main_window_handle())) {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist,
				g_pShareViaHandle->pGenListLandscapeItc,
				(void *)nIndex,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				NULL,
				NULL);
		}
	} else {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist,
				g_pShareViaHandle->pGenListItc,
				(void *)nIndex,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				NULL,
				NULL);
		}
	}

	return TRUE;
}

static void __mp_share_via_view_rotate_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	if (!g_pShareViaHandle) {
		VideoLogError("__mp_share_via_view_rotate_cb IS null");
		return;
	}
	bool bLandscape = mp_rotate_ctrl_check_landspace_by_win(
		(Evas_Object *)mp_util_get_main_window_handle());
	if (bLandscape) {
		VideoLogInfo("Landscape mode");
		g_pShareViaHandle->bLandscape = true;
	} else {
		VideoLogInfo("Portrait mode");
		g_pShareViaHandle->bLandscape = false;
	}
	__mp_share_view_arrange_video_list(g_pShareViaHandle->pObjVideosList,
		TRUE);
	mp_share_view_change_title_info();
}
#endif

static void __mp_share_view_append_video_items(void *pObjVideosList)
{
	VideoLogInfo("");
	if (!g_pShareViaHandle) {
		VideoLogError("No exist g_pShareViaHandle.");
		return;
	}

	int nIndex = 0;
	int nVideoListSize =
		mp_util_svc_get_number_of_video_item_by_type();
	MP_DEL_ITC(g_pShareViaHandle->pGenListItc);
	g_pShareViaHandle->pGenListItc = elm_genlist_item_class_new();
/*
	g_pShareViaHandle->pGenListItc->item_style = "video/edit_list";
*/
	g_pShareViaHandle->pGenListItc->item_style = "2line.top";
	g_pShareViaHandle->pGenListItc->func.text_get =
		(void *)mp_share_view_get_label_of_video_item_cb;
	g_pShareViaHandle->pGenListItc->func.content_get =
		(void *)mp_share_view_get_icon_of_video_item_cb;
	g_pShareViaHandle->pGenListItc->func.state_get = NULL;
	g_pShareViaHandle->pGenListItc->func.del = NULL;
/*
	g_pShareViaHandle->pGenListItc->decorate_item_style = "mode/slide4";
	g_pShareViaHandle->pGenListItc->decorate_all_item_style =
		"edit_default";
*/

	for (nIndex = 0; nIndex < nVideoListSize; nIndex++) {
		elm_genlist_item_append(pObjVideosList,
			g_pShareViaHandle->pGenListItc, (void *)nIndex, NULL,
			ELM_GENLIST_ITEM_NONE,
			mp_share_view_select_video_item_cb, NULL);
	}
}

void mp_share_view_set_value(int nTotalSize)
{
	VideoLogInfo("nTotalSize : [%d]", nTotalSize);

	if (g_pShareViaHandle->pCheckedItemsEditList) {
		free(g_pShareViaHandle->pCheckedItemsEditList);
		g_pShareViaHandle->pCheckedItemsEditList = NULL;
	}

	if (nTotalSize > 0) {
		g_pShareViaHandle->pCheckedItemsEditList =
			(bool *)malloc(sizeof(bool)*nTotalSize);
		memset(g_pShareViaHandle->pCheckedItemsEditList, 0,
			sizeof(bool)*nTotalSize);
	}

	g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
}

bool __mp_share_view_arrange_video_list(void *pObjVideosList, bool bNeedSvc)
{
	if (!g_pShareViaWidget || !g_pShareViaHandle) {
		VideoLogError("No exist g_pShareViaListViewWidget.");
		return FALSE;
	}

	if (!pObjVideosList) {
		VideoLogError("No exist genlist object.");
		return FALSE;
	}

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	int nSortType = mp_share_view_get_sort_type();
	int nViewType = mp_view_as_ctrl_get_type();

	if ((nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) &&
		(g_pShareViaHandle->pFolderPath)) {
		VideoSecureLogInfo("szFolderPath : %s",
			g_pShareViaHandle->pFolderPath);
		if (g_pShareViaHandle->pFolderPath) {
			if (vp_file_exists(g_pShareViaHandle->pFolderPath)) {
				/* update video-list */
				VideoLogInfo("update delete item");
				mp_util_svc_destory_video_item_list();
				mp_util_svc_extract_video_list_from_folder(
					g_pShareViaHandle->pFolderPath,
					nSortType,
					MP_LIST_VIEW_PHONE);
#ifdef ENABLE_DRM_FEATURE
				mp_util_svc_filter_drm_video_item_list();
#endif
				if (bNeedSvc) {
/*
				__mp_share_view_update_selected_video_list();
*/
				} else {
					mp_share_view_set_value(nVideoListSize);
				}
			} else {
				VideoLogInfo("popup folder item");
				elm_naviframe_item_pop(
					g_pShareViaHandle->pNaviFrameHandle);
				return FALSE;
			}
		}
	} else {
		mp_util_svc_destory_video_item_list();
		mp_util_svc_extract_video_list_by_item_type(nSortType,
			MP_LIST_VIEW_PHONE);
#ifdef ENABLE_DRM_FEATURE
		mp_util_svc_filter_drm_video_item_list();
#endif
		if (bNeedSvc) {
			/* __mp_share_view_update_selected_video_list(); */
		} else {
			mp_share_view_set_value(nVideoListSize);
		}
	}

	VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	if (nVideoListSize > 0) {
#ifdef VS_FEATURE_THUMBNAIL_VIEW
		if (g_pShareViaHandle->bThumbnailView) {
			elm_genlist_clear(pObjVideosList);
			__mp_share_view_append_grid_layout(pObjVideosList);
		} else
#endif
		{
			elm_genlist_clear(pObjVideosList);
			__mp_share_view_append_video_items(pObjVideosList);
		}
	} else {
		elm_naviframe_item_pop(g_pShareViaHandle->pNaviFrameHandle);
		return FALSE;
	}
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	if (!g_pShareViaHandle->bThumbnailView) {
		elm_genlist_decorate_mode_set(pObjVideosList, EINA_TRUE);
	}
#endif
	evas_object_show(pObjVideosList);
	return TRUE;
}

Evas_Object *mp_share_view_create_internal_layout(void *pParent)
{
	if (!pParent) {
		VideoLogInfo("[ERR]");
		return NULL;
	}

	VideoLogInfo("");

	if (g_pShareViaWidget->pChareViaViewBaselayout) {
		evas_object_del(g_pShareViaWidget->pChareViaViewBaselayout);
		g_pShareViaWidget->pChareViaViewBaselayout = NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", VIDEO_PLAYER_REMOVE_LIST_EDJ);
	free(path);
	g_pShareViaWidget->pChareViaViewBaselayout = elm_layout_add(pParent);
	elm_layout_file_set(g_pShareViaWidget->pChareViaViewBaselayout,
		edj_path,
		REMOVE_LIST_EDJ_GROUP);
	evas_object_size_hint_weight_set(
		g_pShareViaWidget->pChareViaViewBaselayout,
		EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(
		g_pShareViaWidget->pChareViaViewBaselayout,
		EVAS_HINT_FILL,
		EVAS_HINT_FILL);

	return g_pShareViaWidget->pChareViaViewBaselayout;
}

static bool __mp_share_view_get_selected_video_list(int nSelectedSize)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return FALSE;
	}

	int nVideoListSize = 0;

	if (LIST_TAB_TYPE_PERSONAL != g_pShareViaHandle->eTabType) {
		return FALSE;
	}

	if (!g_pShareViaHandle->nSelectCounterForEditList ||
		nSelectedSize < 1) {
		return FALSE;
	}

	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	__mp_share_view_free_selected_list();

	int nIndex = 0;
	int nCounter = 0;
	char *pMediaId = NULL;

	g_pShareViaHandle->pSelectedList = (char **)calloc(sizeof(char*),
						nSelectedSize);
	VideoLogInfo("current edit size===%d", nSelectedSize);

	for (nCounter = 0; nCounter < nVideoListSize; nCounter++) {
		if (g_pShareViaHandle->pCheckedItemsEditList[nCounter]) {
			pMediaId = mp_util_svc_get_video_id_by_index(nCounter);

			if (pMediaId) {
				g_pShareViaHandle->pSelectedList[nIndex] =
					strdup(pMediaId);
				MP_FREE_STRING(pMediaId);
				nIndex++;
			}
		}
	}
	return TRUE;
}

/*
static void __mp_share_view_update_select_all(void)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nVideoListSize = 0;
	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	if (nVideoListSize == g_pShareViaHandle->nSelectCounterForEditList) {
		VideoLogInfo("All check box of edit list state are true.");
		g_pShareViaHandle->bSelectallCheckBoxState = TRUE;
	}
	else
	{
		g_pShareViaHandle->bSelectallCheckBoxState = FALSE;
	}

	VideoLogInfo("nVideoListSize=%d,nSelectCounterForEditList=%d",
		nVideoListSize, g_pShareViaHandle->nSelectCounterForEditList);
	elm_check_state_set(g_pShareViaWidget->pSelectallbtn,
		g_pShareViaHandle->bSelectallCheckBoxState);
}

static void __mp_share_view_update_selected_video_list(void)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nVideoListSize = 0;

	if (LIST_TAB_TYPE_PERSONAL != g_pShareViaHandle->eTabType) {
		return;
	}

	MpMediaType eMediaType = MEDIA_TYPE_FILE;

	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	mp_share_view_set_value(nVideoListSize);

	int nIndex = 0;
	int nCounter = 0;
	bool bValidMedia = FALSE;
	int nTotal = g_pShareViaHandle->nSelectCounterForEditList;
	VideoLogInfo("OldEditList = %d", nTotal);
	g_pShareViaHandle->nSelectCounterForEditList = 0;

	if (!g_pShareViaHandle->pSelectedList) {
		return;
	}

	while ((nIndex < nTotal) && g_pShareViaHandle->pSelectedList[nIndex]) {

		bValidMedia = mp_util_svc_check_valid_media_id(
			g_pShareViaHandle->pSelectedList[nIndex], eMediaType,
			&nCounter);
		if (bValidMedia) {
			g_pShareViaHandle->pCheckedItemsEditList[nCounter] =
				TRUE;
			g_pShareViaHandle->nSelectCounterForEditList++;
		}
		nIndex++;
	}

	__mp_share_view_update_select_all();

	mp_share_view_change_title_info();

	if (g_pShareViaHandle->nSelectCounterForEditList > 0)
		elm_object_disabled_set(g_pShareViaWidget->pSavebtn,
			EINA_FALSE);
	else
		elm_object_disabled_set(g_pShareViaWidget->pSavebtn,
			EINA_TRUE);

}
*/


static void __mp_share_view_db_changed_cb(void *pUserData)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}
	bool bNormalShow = TRUE;
	bNormalShow = __mp_share_view_arrange_video_list(
			g_pShareViaHandle->pObjVideosList, TRUE);
	if (!bNormalShow) {
		VideoLogError("invalid view");
		return;
	}

	Elm_Object_Item *pItem = NULL;
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	if (g_pShareViaHandle->bThumbnailView) {
		pItem = elm_gengrid_first_item_get(
			g_pShareViaHandle->pObjVideosList);
		if (pItem) {
			elm_gengrid_item_bring_in(pItem,
				ELM_GENGRID_ITEM_SCROLLTO_IN);
		}
	} else
#endif
	{
		pItem = elm_genlist_first_item_get(
			g_pShareViaHandle->pObjVideosList);
		if (pItem) {
			elm_genlist_item_bring_in(pItem,
				ELM_GENLIST_ITEM_SCROLLTO_IN);
		}
	}
}

static void __mp_share_view_db_backup_cb(void *pUserData)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}
	__mp_share_view_get_selected_video_list(
		g_pShareViaHandle->nSelectCounterForEditList);
}

static void __mp_share_select_all_layout_down_cb(void *data, Evas *evas,
	Evas_Object *obj, void *event_info)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	Evas_Object *check = elm_object_part_content_get(
				g_pShareViaWidget->pSelAllLayout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);

	if (nVideoListSize <= VIDEO_SHARE_ITEM_MAX) {
		elm_check_state_set(check, !state);
	}

	__mp_share_view_select_all_change_check_box_cb(data, check, NULL);
}

void mp_share_view_init(void *pParent)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");
	g_pShareViaWidget->pChareViaViewBaselayout =
		mp_share_view_create_internal_layout(pParent);
	if (!g_pShareViaWidget->pChareViaViewBaselayout) {
		VideoLogError("pChareViaViewBaselayout is not existed.");
		return;
	}
	evas_object_event_callback_add(
		g_pShareViaWidget->pChareViaViewBaselayout,
		EVAS_CALLBACK_DEL,
		__mp_share_view_base_layout_del_cb,
		NULL);

	g_pShareViaWidget->pVideoListBox = elm_box_add(
		g_pShareViaWidget->pChareViaViewBaselayout);
	if (!g_pShareViaWidget->pVideoListBox) {
		VideoLogError("pVideoListBox is not existed.");
		return;
	}

	evas_object_size_hint_weight_set(g_pShareViaWidget->pVideoListBox,
		EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pShareViaWidget->pVideoListBox,
		EVAS_HINT_FILL, EVAS_HINT_FILL);

	mp_widget_ctrl_create_select_all_layout(
		g_pShareViaWidget->pVideoListBox,
		__mp_share_view_select_all_change_check_box_cb,
		__mp_share_select_all_layout_down_cb,
		(void *)g_pShareViaWidget,
		&g_pShareViaWidget->pSelectallbtn,
		&g_pShareViaWidget->pSelAllLayout);

	if (!g_pShareViaWidget->pSelAllLayout) {
		VideoLogError("pSelAllLayout is not existed.");
		return;
	}

	elm_box_pack_start(g_pShareViaWidget->pVideoListBox,
		g_pShareViaWidget->pSelAllLayout);


#ifdef VS_FEATURE_THUMBNAIL_VIEW
	if (g_pShareViaHandle->bThumbnailView) {
		g_pShareViaHandle->pObjVideosList = elm_genlist_add(
			g_pShareViaWidget->pVideoListBox);
		if (!g_pShareViaHandle->pObjVideosList) {
			VideoLogError("pObjVideosList does not exist.");
			return;
		}
		elm_scroller_bounce_set(g_pShareViaHandle->pObjVideosList,
			EINA_FALSE, EINA_TRUE);
		evas_object_size_hint_weight_set(
			g_pShareViaHandle->pObjVideosList, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(
			g_pShareViaHandle->pObjVideosList, EVAS_HINT_FILL,
			EVAS_HINT_FILL);
		elm_genlist_block_count_set(g_pShareViaHandle->pObjVideosList,
			VIDEO_GENLIST_BLOCK_COUNT);
		elm_genlist_homogeneous_set(g_pShareViaHandle->pObjVideosList,
			EINA_TRUE);
		elm_genlist_mode_set(g_pShareViaHandle->pObjVideosList,
			ELM_LIST_COMPRESS);
	} else
#endif
	{
		g_pShareViaHandle->pObjVideosList = elm_genlist_add(
			g_pShareViaWidget->pVideoListBox);
		if (!g_pShareViaHandle->pObjVideosList) {
			VideoLogError("pObjVideosList is not existed.");
			return;
		}
		evas_object_size_hint_weight_set(
			g_pShareViaHandle->pObjVideosList,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(
			g_pShareViaHandle->pObjVideosList,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_genlist_block_count_set(g_pShareViaHandle->pObjVideosList,
			VIDEO_GENLIST_BLOCK_COUNT);
		elm_genlist_homogeneous_set(g_pShareViaHandle->pObjVideosList,
			EINA_TRUE);
		elm_genlist_mode_set(g_pShareViaHandle->pObjVideosList,
			ELM_LIST_COMPRESS);
	}

	elm_box_pack_end(g_pShareViaWidget->pVideoListBox,
		g_pShareViaHandle->pObjVideosList);
	elm_object_part_content_set(g_pShareViaWidget->pChareViaViewBaselayout,
		SWALLOW_LISTVIEW_CONTENT, g_pShareViaWidget->pVideoListBox);

	bool bNormalShow = TRUE;
	bNormalShow = __mp_share_view_arrange_video_list(
		g_pShareViaHandle->pObjVideosList, FALSE);
	if (!bNormalShow) {
		VideoLogError("invalid view");
		return;
	}

	char *szTitle = g_strdup_printf(VIDEOS_SELECTVIEW_HEADER_PD_SELECTED,
		g_pShareViaHandle->nSelectCounterForEditList);

	g_pShareViaHandle->pNaviFrameItem = elm_naviframe_item_push(
		g_pShareViaHandle->pNaviFrameHandle, szTitle, NULL, NULL,
		g_pShareViaWidget->pChareViaViewBaselayout, NULL);
	elm_object_item_domain_text_translatable_set(
		g_pShareViaHandle->pNaviFrameItem, VIDEOS_STRING, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(g_pShareViaHandle->pNaviFrameItem,
		mp_share_view_back_btn_cb, g_pShareViaHandle);

	elm_naviframe_item_title_enabled_set(g_pShareViaHandle->pNaviFrameItem,
		EINA_TRUE, EINA_FALSE);
	evas_object_smart_callback_add(g_pShareViaHandle->pNaviFrameHandle,
		"transition,finished",
		mp_share_view_naviframe_transition_effect_cb, NULL);

/*
	g_pShareViaWidget->pCancelbtn = mp_widget_ctrl_create_navi_left_btn(
		g_pShareViaHandle->pNaviFrameHandle,
		g_pShareViaHandle->pNaviFrameItem,
		NULL,
		__mp_share_view_show_cancel_cb,
		(void *)g_pShareViaHandle);
	g_pShareViaWidget->pSavebtn = mp_widget_ctrl_create_navi_right_btn(
		g_pShareViaHandle->pNaviFrameHandle,
		g_pShareViaHandle->pNaviFrameItem,
		NULL, __mp_share_view_show_share_via_panel_cb,
		(void *)g_pShareViaHandle);
*/

	g_pShareViaWidget->pCancelbtn = elm_button_add(
		g_pShareViaHandle->pNaviFrameHandle);
	elm_object_style_set(g_pShareViaWidget->pCancelbtn, "naviframe/title_left");
	//elm_object_text_set(g_pShareViaWidget->pCancelbtn,
	//	VIDEOS_SHAREVIEW_CANCEL_IDS);
	mp_util_set_translate_str(g_pShareViaWidget->pCancelbtn,
		"IDS_TPLATFORM_ACBUTTON_CANCEL_ABB");
	elm_object_signal_callback_add(g_pShareViaWidget->pCancelbtn,
		"elm,action,click", "", __mp_share_view_show_cancel_cb,
		(void *)g_pShareViaHandle);
	elm_object_item_part_content_set(g_pShareViaHandle->pNaviFrameItem,
		"title_left_btn", g_pShareViaWidget->pCancelbtn);
	evas_object_show(g_pShareViaWidget->pCancelbtn);

	/* Title Done layout */
	g_pShareViaWidget->pSavebtn = elm_button_add(
					g_pShareViaHandle->pNaviFrameHandle);
	elm_object_style_set(g_pShareViaWidget->pSavebtn, "naviframe/title_right");
	//elm_object_text_set(g_pShareViaWidget->pSavebtn,
	//	VIDEOS_SHAREVIEW_DONE_IDS);
	mp_util_set_translate_str(g_pShareViaWidget->pSavebtn,
		"IDS_TPLATFORM_ACBUTTON_DONE_ABB");
	elm_object_signal_callback_add(g_pShareViaWidget->pSavebtn,
		"elm,action,click", "",
		__mp_share_view_show_share_via_panel_cb,
		(void *)g_pShareViaHandle);
	elm_object_item_part_content_set(g_pShareViaHandle->pNaviFrameItem,
		"title_right_btn", g_pShareViaWidget->pSavebtn);
	evas_object_show(g_pShareViaWidget->pSavebtn);
	elm_object_disabled_set(g_pShareViaWidget->pSavebtn, EINA_TRUE);

	evas_object_show(g_pShareViaHandle->pObjVideosList);
	evas_object_show(g_pShareViaWidget->pChareViaViewBaselayout);

	mp_util_db_set_update_fun(g_pShareViaHandle->euLev,
		__mp_share_view_db_changed_cb);
	mp_util_db_set_backup_fun(g_pShareViaHandle->euLev,
		__mp_share_view_db_backup_cb);

	MP_FREE_STRING(szTitle);
	mp_util_hide_indicator();
}

void mp_share_view_push(void *pNaviFrame,
		ChangeListShareViewCbFunc pChangeViewCb, char *pFolderPath,
		MpVideoListTabType eTabType)
{
	VideoLogInfo("");
	if (g_pShareViaHandle || g_pShareViaWidget) {
		mp_share_view_destroy();
	}

	g_pShareViaHandle = (st_VideoListShareViaViewHandle *)calloc(1,
				sizeof(st_VideoListShareViaViewHandle));
	g_pShareViaWidget = (st_VideoListShareViaViewWidget *)calloc(1,
				sizeof(st_VideoListShareViaViewWidget));

	memset(g_pShareViaHandle, 0, sizeof(st_VideoListShareViaViewHandle));
	memset(g_pShareViaWidget, 0, sizeof(st_VideoListShareViaViewWidget));
	mp_share_view_reset_value();

	g_pShareViaHandle->pNaviFrameHandle = pNaviFrame;
	g_pShareViaHandle->pChangeViewUserCbFunc = pChangeViewCb;
	if (pFolderPath) {
		g_pShareViaHandle->pFolderPath = strdup(pFolderPath);
	}

	g_pShareViaHandle->eTabType = eTabType;
	g_pShareViaHandle->euLev = MP_DB_UPDATE_LEV_1;

	int nViewType = mp_view_as_ctrl_get_type();

	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST &&
		g_pShareViaHandle->pFolderPath) {
		g_pShareViaHandle->euLev = MP_DB_UPDATE_LEV_2;
	}
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	if ((nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST ||
		nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) &&
		(LIST_TAB_TYPE_PERSONAL == eTabType)) {
		g_pShareViaHandle->bThumbnailView = TRUE;
	} else {
		g_pShareViaHandle->bThumbnailView = FALSE;
	}
#endif

	mp_share_view_init(g_pShareViaHandle->pNaviFrameHandle);
	evas_object_smart_callback_add(
		(Evas_Object *)mp_util_get_main_window_handle(),
		"wm,rotation,changed", __mp_share_via_view_rotate_cb,
		(void *)g_pShareViaHandle);
}

void mp_share_view_pop(void)
{
	VideoLogInfo("");

	/*
	elm_naviframe_item_pop(g_pShareViaHandle->pNaviFrameHandle);
	*/
	if (!g_pShareViaHandle) {
		VideoLogError("No existed handle of share view.");
		return;
	}
	if (g_pShareViaHandle->pChangeViewUserCbFunc) {
		g_pShareViaHandle->pChangeViewUserCbFunc();
	}

	mp_util_hide_indicator();
}

bool mp_share_view_is_top_view(void)
{
	VideoLogInfo("");

	return FALSE;
}

void mp_share_view_change_language(void)
{
	if (!g_pShareViaHandle || !g_pShareViaWidget) {
		VideoLogError("No existed handle of share view.");
		return;
	}

	VideoLogInfo("");

	mp_share_view_change_title_info();
}


static void
__mp_share_view_base_layout_del_cb(void *pUserData, Evas *e,
	Evas_Object *pObject, void *pEventInfo)
{
	/*
	evas_object_event_callback_del(
		g_pShareViaWidget->pChareViaViewBaselayout,
		EVAS_CALLBACK_DEL, __mp_share_view_base_layout_del_cb);
	*/

	__mp_share_view_reset();
}


